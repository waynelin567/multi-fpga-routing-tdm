#include "db.h"
namespace router
{

void topologyMgr::init(const int& e_num, const int& n_num)
{
    _pssTreeSizes.resize(e_num, 0);
    _passingTrees.resize(e_num);
    tbb::parallel_for(0, e_num, 1,
        [&](const int& i)
        {
            _passingTrees[i].reserve(n_num);
            for (int j = 0; j < n_num; ++j)
            {
                _passingTrees[i].emplace_back(false, TdmType(0));
            }
        }
    );
    _topologies.resize(n_num);
}

void topologyMgr::add_passing_tree(const int& eid, const int& nid)
{
    assert(!_passingTrees[eid][nid].first);
    _passingTrees[eid][nid].first = true;
    _pssTreeSizes[eid]++;
}

void topologyMgr::delete_passing_tree(const int& eid, const int& nid)
{
    assert(_passingTrees[eid][nid].first);
    _passingTrees[eid][nid].first = false;
    _pssTreeSizes[eid]--;
    assert(_pssTreeSizes[eid] >= 0);
}

void topologyMgr::copy_passing_trees(const int& eid, std::unordered_map<net_id, UI>& ret) const
{
    ret.reserve(_pssTreeSizes[eid]);
    const auto& trees = _passingTrees[eid];
    for (int nid = 0; nid < (int)trees.size(); nid++) 
    {
        if (trees[nid].first == true)
        {
            ret.emplace(nid, 0);
        }
    }
}

void topologyMgr::add_TDM(const int& eid, const int& nid, const UI val)
{
    _passingTrees[eid][nid].second = val;
}

TdmType topologyMgr::CalcTDM(const int& nid)
{
    TdmType sum = 0;
    const auto& edges = _topologies[nid].get_edges();
    for (const auto& eid : edges)
    {
        assert(_passingTrees[eid][nid].first);
        sum += _passingTrees[eid][nid].second;
    }
    return sum;
}

void topologyMgr::add_tree_to_edge(const int& nid)
{
    std::vector<bool> vec(get_graphMgr().get_fpga_edge_size(), false);
    const auto& edges = _topologies[nid].get_edges();
    for (const auto& eid : edges) {
        add_passing_tree(eid, nid);
        assert(vec[eid] == false);
        vec[eid] = true;
    }
}

graphMgr::~graphMgr()
{
    delete [] _adjlist;

    for (int fid = 0; fid < _fpga_num; ++fid) {
        delete [] _vertex_edge_map[fid];
    }
    delete [] _vertex_edge_map;

    for (int eid = 0; eid < _fpga_edge_num; ++eid) {
        delete [] _edge_info[eid];
    }
    delete _edge_info;
}

void graphMgr::init(int f_num, int e_num)
{
    _adjlist = new std::vector<fpga_id>[f_num];

    _vertex_edge_map = new fpga_id*[f_num];
    for (int fid = 0; fid < f_num; ++fid) {
        _vertex_edge_map[fid] = new fpga_id[f_num];
        for (int i = 0; i < f_num; ++i) {
            _vertex_edge_map[fid][i] = -1;
        }
    }

    _edge_info = new fpga_id*[e_num];
    for (int eid = 0; eid < e_num; ++eid) {
        _edge_info[eid] = new fpga_id[2];
    }

    _fpga_num = f_num;
    _fpga_edge_num = e_num;
}

void graphMgr::add_edge(const fpga_id& fid_1, const fpga_id& fid_2, const fpga_edge_id& eid)
{
    _adjlist[fid_1].push_back(fid_2);
    _adjlist[fid_2].push_back(fid_1);
    _vertex_edge_map[fid_1][fid_2] = eid;
    _vertex_edge_map[fid_2][fid_1] = eid;
    _edge_info[eid][0] = fid_1;
    _edge_info[eid][1] = fid_2;
}

void graphMgr::print_edge(const fpga_edge_id& eid)
{
    std::cout << eid << " connect " << _edge_info[eid][0] << " -- " << _edge_info[eid][1] << std::endl;
}

}
