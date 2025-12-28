#include "ripupRouter.h"
#include "db.h"
#include "mgr.h"
#include "steiner.h"
namespace router
{

RipUpRouter::RipUpRouter()
{
    _edgeWeight.resize(get_graphMgr().get_fpga_edge_size(), 0.001);
    _ripUpCount = get_OptionMgr().RipUpCount;
}
void RipUpRouter::run()
{
    for (int i = 0; i < _ripUpCount; i++)
    {
        pickNets();
        ripup();
        route();
        _candidates.clear();
    }

}
void RipUpRouter::calcNGImportance(std::vector<std::pair<double, net_group_id> >& NGImportance)
{
    mgr& m = get_mgr();
    topologyMgr& _tMgr = get_topologyMgr();
    std::vector<int> netImportance (m.get_net_size(), 0);
    for (int nid = 0; nid < (int)netImportance.size(); nid ++)
    {
        const auto& edges = _tMgr.get_tree_edges(nid);
        for (const auto& eid : edges)
        {
            netImportance[nid] += _tMgr.get_passing_tree_size(eid);
        }
    }
    double max = 0;
    net_group_id maxID = -1;
    for (int gid = 0; gid < (int)NGImportance.size(); gid ++)
    {
        const auto& nets = m.get_net_group(gid).get_nets();
        NGImportance[gid].first = 0;
        NGImportance[gid].second = gid;
        for (int i = 0; i < (int)nets.size(); i++)
        {
            NGImportance[gid].first += netImportance[nets[i]];
        }
        if (NGImportance[gid].first > max)
        {
            max = NGImportance[gid].first;
            maxID = gid;
        }
    }
    NGImportance[0].first = max;
    NGImportance[0].second = maxID;
//    std::sort(NGImportance.begin(), NGImportance.end(), []
//      (const std::pair<double, net_group_id>& _1, 
//       const std::pair<double, net_group_id>& _2) {return _1 > _2;});
}
void RipUpRouter::pickNets()
{
    mgr& m = get_mgr();
    std::vector<std::pair<double, net_group_id> > NGImportance (m.get_net_group_size());
    std::vector<bool> isPicked(m.get_net_size(), false);
    calcNGImportance(NGImportance);
    for (int i = 0; i < 1; i++)
    {
        const auto& nets = m.get_net_group(NGImportance[i].second).get_nets();
        for (int j = 0; j < (int)nets.size(); j++)
        {
            if (!isPicked[nets[j]])
            {
                isPicked[nets[j]] = true;
                _candidates.push_back(nets[j]);
            }
        }
    }
    std::sort(_candidates.begin(), _candidates.end(),
            [&](const net_id& _1, const net_id& _2)
    {
        return m.get_net(_1).get_target_size() < m.get_net(_2).get_target_size();
    });
}
void RipUpRouter::ripup()
{
    topologyMgr& _tMgr = get_topologyMgr();
    for (int i = 0; i < (int)_candidates.size(); i++)
    {
       const auto& treeEdges = _tMgr.get_tree_edges(_candidates[i]);
       for (const auto& e: treeEdges)
       {
           get_topologyMgr().delete_passing_tree(e, _candidates[i]);
       }
    }
}
void RipUpRouter::setNetColors(std::vector<std::vector<int> >& colors)
{
    std::vector<bool> check(get_graphMgr().get_fpga_size(), false);
    for (int i = 0; i < (int)_candidates.size(); ++i) {
        const net& curNet = get_mgr().get_net(_candidates[i]);
        auto c = &colors[i][0];
        boost::put(c, curNet.get_source(), paal::Terminals::TERMINAL);
        check[curNet.get_source()] = true;
        for (int i = 0; i < curNet.get_target_size(); ++i) {
            boost::put(c, curNet.get_target(i), paal::Terminals::TERMINAL);
            check[curNet.get_target(i)] = true;
        }

        for (int fid = 0; fid < get_graphMgr().get_fpga_size(); ++fid) {
            if (!check[fid]) boost::put(c, fid, paal::Terminals::NONTERMINAL);
            else check[fid] = false;
        }
    }
}
void RipUpRouter::route()
{
    get_boostGraphMgr().SetWeight(_edgeWeight);
    auto g     = get_boostGraphMgr().GetGraph();
    auto index = get(boost::vertex_index, g);
    typedef boost::graph_traits<decltype(g)>::edge_descriptor Edge;
    std::vector<std::set<Edge> > steinerEdges(_candidates.size());
    std::vector<std::vector<int> > colors(_candidates.size());
    for (int i = 0; i < (int)colors.size(); ++i) colors[i].resize(num_vertices(g));
    setNetColors(colors);

    for (int i = 0; i < (int)_candidates.size(); ++i) {
        const net_id& nid = _candidates[i];
        paal::steiner_tree_greedy(g, std::inserter(steinerEdges[i], steinerEdges[i].begin()),
            boost::vertex_color_map(boost::make_iterator_property_map(colors[i].begin(), index)));
        tree newTree;
        for (const auto& e : steinerEdges[i]) {
            auto v1 = source(e, g);
            auto v2 = target(e, g);
            fpga_edge_id eid = get_graphMgr().get_edge_id((fpga_id)index[v1], (fpga_id)index[v2]);
            newTree.add_edge(eid);
            const auto& ee = boost::edge((fpga_id)index[v1], (fpga_id)index[v2], g); 
            boost::put(boost::edge_weight_t(), g, ee.first, (_edgeWeight[eid]+=1) );
        }
        get_topologyMgr().set_tree(nid, newTree);
        get_topologyMgr().add_tree_to_edge(nid);

    }
}

}
