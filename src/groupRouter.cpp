#include "groupRouter.h"
#include "db.h"
#include "mgr.h"
namespace router
{

AstarRouter::AstarRouter(net_id nid, std::vector<Cost>& c1,
                         std::vector<fpga_id>& fpgas, 
                         BFSdb& bfsdb):
    _nid(nid), 
    _c1(c1), 
    _fpgas(fpgas),
    _bfsdb(bfsdb)
{
    _isVisited.resize(get_graphMgr().get_fpga_size(), false);
    _min_node_cost.resize(get_graphMgr().get_fpga_size(),
                          std::numeric_limits<Cost>::max());
    _parent.resize(get_graphMgr().get_fpga_size(), -1);
}
void AstarRouter::route()
{
    for (int i = 0; i < (int)_fpgas.size()-1; i++)
    {
        std::fill(_min_node_cost.begin(), _min_node_cost.end(), 
                  std::numeric_limits<Cost>::max());
        if (!_isVisited[_fpgas[i]])
        {
            doAstar(_fpgas[i], _fpgas[i+1]);
        }
        else if (!_isVisited[_fpgas[i+1]])
        {
            doAstar(_fpgas[i+1], _fpgas[i]);
        }
    }
    get_topologyMgr().set_tree(_nid, _tree);
}
void AstarRouter::doAstar(fpga_id src, fpga_id dest)
{
    graphMgr& _gMgr = get_graphMgr();
    initSrc(src);
    while (!_Q.empty())
    {
        Cost node_cost = _Q.begin()->first;
        fpga_id u = _Q.begin()->second;
        _Q.erase(_Q.begin());
        if (u == dest) 
        {
            break;
        }
        else if (_isVisited[u])
        {
            dest = u;
            break;
        }
        
        const auto& neighbors = get_graphMgr().get_adjlist(u);
        for (const auto& v : neighbors)
        {
            Cost edge_cost = getEdgeCost(_gMgr.get_edge_id(u, v));
            Cost cost_thru_u = node_cost + edge_cost;
            if (cost_thru_u < _min_node_cost[v])
            {
                auto hit = _Q.find(std::make_pair(_min_node_cost[v], v));
                if (hit!= _Q.end()) _Q.erase(hit);
                
                _min_node_cost[v] = cost_thru_u;
                _parent[v] = u;
                _Q.emplace(_min_node_cost[v], v);
            }
        }
    }
    tracePath(src, dest);
}

void AstarRouter::tracePath(fpga_id src, fpga_id dest)
{
    graphMgr& _gMgr = get_graphMgr();
    fpga_id now_fid = -1;
    for (now_fid = dest; now_fid != src; now_fid = _parent[now_fid])
    {
        _tree.add_edge(_gMgr.get_edge_id(now_fid, _parent[now_fid]));
        _isVisited[now_fid] = true;
    }
    _isVisited[src] = true;
}

double AstarRouter::getEdgeCost(fpga_edge_id eid)
{
    return _c1[eid];
}

void AstarRouter::initSrc(fpga_id src)
{
    _Q.clear();
    _min_node_cost[src] = 0;
    _Q.emplace(_min_node_cost[src], src);
}
/******************** groupRouter ********************/
groupRouter::groupRouter():
    _MSTdb(_BFSdb)
{
    _BFSdb.calcBFSdb();
    _MSTdb.run();
    _c1.resize(get_graphMgr().get_fpga_edge_size(), 1);
}

void groupRouter::sortNetGroups()
{
    _groupOrder.resize(get_mgr().get_net_group_size());
    for (int gid = 0; gid < (int)_groupOrder.size(); gid++)
    {
        _groupOrder[gid].first = 0;
        _groupOrder[gid].second = gid;
        const auto& nets = get_mgr().get_net_group(gid).get_nets();
        for (const auto& nid : nets)
        {
            _groupOrder[gid].first += _MSTdb.getMSTCost(nid);
        }
    }
    std::sort(_groupOrder.begin(), _groupOrder.end(), std::less<std::pair<int, net_group_id> >());
    /*std::sort(_groupOrder.begin(), _groupOrder.end(), 
            [](const std::pair<int, net_group_id>& _1, 
               const std::pair<int, net_group_id>& _2)
            {
                return _1.first < _2.first;
            });*/
}

void groupRouter::getIntraGroupNetOrder(net_group_id gid, 
          std::vector<int>& netCounter)
{
    mgr& m = get_mgr();
    const auto& nets = m.get_net_group(gid).get_nets(); 
    
    std::vector<std::pair<int, net_id> > intraNetOrder;
    intraNetOrder.reserve(nets.size());
    
    //for (int i = 0; i < (int)nets.size(); i ++)
    for (int i = (int)nets.size()-1; i>=0; i --)
    {
        netCounter[nets[i]] ++;
        if (netCounter[nets[i]] == 
            (int)m.get_net(nets[i]).get_netgroup_size())
        {
            intraNetOrder.emplace_back(_MSTdb.getMSTCost(nets[i]), 
                                       nets[i]);
        }
    } 
    if (!intraNetOrder.empty())
    {
        //std::sort(intraNetOrder.begin(), intraNetOrder.end(),
        //        std::less<std::pair<int, net_id> >());
        std::sort(intraNetOrder.begin(), intraNetOrder.end(), 
            [](const std::pair<int, net_id>& _1, 
               const std::pair<int, net_id>& _2)
            {
                return _1.first < _2.first;
            });
        for (int i = 0; i < (int)intraNetOrder.size(); i++)
        {
            _netOrder.push_back(intraNetOrder[i].second);
        }
    }
}
void groupRouter::getNetOrder()
{
    mgr& m = get_mgr();
    _netOrder.reserve(m.get_net_size());
    std::vector<int> netCounter(m.get_net_size(), 0);
    for (int i = 0; i < (int)_groupOrder.size(); i++)
    {
        getIntraGroupNetOrder(_groupOrder[i].second, netCounter);        
    }
    _netOrder.push_back(-1);
}
void groupRouter::getParallelNetOrder(std::vector<std::vector<net_id> >& nids_t)
{
    size_t reserveSize = _netOrder.size()/nids_t.size();
    for (int i = 0; i < (int)nids_t.size(); i++) 
        nids_t[i].reserve(reserveSize);
    
    int counter = 0;
    for (int i = 0; i < (int)_netOrder.size(); i++)
    {
        if (_netOrder[i]!=-1)
        {
            nids_t[counter].push_back(_netOrder[i]);
            counter ++;
        }
        else
        {
            while (counter < (int)nids_t.size())
            {
                nids_t[counter].push_back(-1);
                counter++; 
            }
        }
        if (counter == (int)nids_t.size()) 
            counter = 0;
    }
}
void groupRouter::route(int nParallel)
{
    topologyMgr& _tMgr = get_topologyMgr();
    sortNetGroups();
    getNetOrder();
    std::vector<std::vector<net_id> > nids_t(nParallel);
    getParallelNetOrder(nids_t);
    // stores the net that the corresponding thread is processing
    std::vector<net_id> net_t(nParallel, -1); 
    //all threads should be of the same size
    size_t iter = nids_t[0].size(); 
    #pragma omp parallel num_threads(nParallel)
    for (size_t i = 0; i < iter; ++i) 
    {
        int tid = omp_get_thread_num(); 
        const net_id& nid = nids_t[tid][i];
        if (nid != -1) {
            AstarRouter astar(nid, _c1, _MSTdb.getSortedFPGAs(nid), 
                              _BFSdb);
            astar.route();
            net_t[tid] = nid;
        }
        #pragma omp barrier
        {
        #pragma omp single
        {
        for (int j = 0; j < (int)net_t.size(); j++) {
            net_id& n = net_t[j];
            if (n == -1) continue;
            const treeEdge& edges = _tMgr.get_tree_ref(n).get_edges();
            for (const auto& eid : edges) {
                _c1[eid]++;
            }
            n = -1;
        }
        }
        }
    }
    for (net_id nid = 0; nid < get_mgr().get_net_size(); ++nid) 
    {
        get_topologyMgr().add_tree_to_edge(nid);
    }
}
}
