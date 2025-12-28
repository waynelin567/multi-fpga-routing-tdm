#ifndef _DB_H_
#define _DB_H_
#include "def.h"
#include "tree.h"
#pragma GCC system_header
#include <tbb/parallel_for.h>
#include <tbb/global_control.h>
namespace router
{

class topologyMgr
{
public:
    void      init                 (const int& e_num, const int& n_num);
    void      add_passing_tree     (const int& eid, const int& nid);
    void      delete_passing_tree  (const int& eid, const int& nid);
    void      copy_passing_trees   (const int& eid, std::unordered_map<net_id, UI>& ret) const;
    void      add_TDM              (const int& eid, const int& nid, const UI val);
    int       get_passing_tree_size(const int& eid) const           {return _pssTreeSizes[eid];}
    vTrees&   get_passing_trees    (const int& eid)                 {return _passingTrees[eid];} 
    TdmType   get_TDM_value        (const int& eid, const int& nid) {return _passingTrees[eid][nid].second;}

    int       get_tree_size        (const int& nid) const {return (int)_topologies[nid].size();}
    tree&     get_tree_ref         (const int& nid)       {return _topologies[nid];}
    treeEdge& get_tree_edges       (const int& nid)       {return _topologies[nid].get_edges();}
    TdmType   CalcTDM              (const int& nid);
    void      print_tree           (const int& nid) const {_topologies[nid].print();}
    void      set_tree             (const int& nid, const tree& t) {_topologies[nid] = t;}
    void      add_tree_to_edge     (const int& nid);

private:
    // passing trees on edges, TDM values are stored here
    std::vector<vTrees>     _passingTrees;
    std::vector<int>        _pssTreeSizes;

    // corresponding topologies
    std::vector<tree>       _topologies;

public:
    static topologyMgr& get_instance()
    {
        static topologyMgr m;
        return m;
    }

};
inline topologyMgr& get_topologyMgr() {return topologyMgr::get_instance();}

class graphMgr
{
public:
    graphMgr() {}
    ~graphMgr();

    void         init                (int f_num, int e_num);
    void         add_edge            (const fpga_id&, const fpga_id&, const fpga_edge_id&);
    int          get_fpga_size       () const                                     {return _fpga_num;}
    int          get_fpga_edge_size  () const                                     {return _fpga_edge_num;}
    size_t       get_degree          (const fpga_id& fid)                         {return _adjlist[fid].size();}
    fpga_edge_id get_edge_id         (const fpga_id& fid_1, const fpga_id& fid_2) {return _vertex_edge_map[fid_1][fid_2];}
    fpga_id      get_fpga_edge_point (const fpga_edge_id& eid, const short& i)    {return _edge_info[eid][i];}

    void         print_edge          (const fpga_edge_id& eid);
    auto         get_adjlist         (const fpga_id fid) -> std::vector<fpga_id>& {return _adjlist[fid];}

private:
    std::vector<fpga_id>*         _adjlist;         // adjacency list
    fpga_edge_id**                _vertex_edge_map; // adjacency matrix
    fpga_id**                     _edge_info;       // edge connection info

    int                           _fpga_num;
    int                           _fpga_edge_num;

public:
    static graphMgr& get_instance()
    {
        static graphMgr m;
        return m;
    }
};
inline graphMgr& get_graphMgr() {return graphMgr::get_instance();}

}
#endif
