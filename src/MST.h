#ifndef __MST_H__
#define __MST_H__

#include "def.h"
#include "db.h"
#include "mgr.h"
#include "BFS.h"

#pragma GCC system_header
#include "tbb/global_control.h"
#include "tbb/parallel_for.h"

namespace router
{

class MST
{
    typedef std::tuple<int, int, int> edge;
    typedef std::priority_queue<edge, std::vector<edge>, std::greater<edge> > KruskalQueue;
    typedef std::pair<int, int> node;
    typedef std::priority_queue<node, std::vector<node>, std::greater<node> > PrimQueue;
public:
    MST(BFSdb& bfsdb);
    ~MST() {}

    void run                ();
    void generateMST        (const int& nid);
    void Kruskal            (const int& nid, const int& netSize, std::vector<std::vector<int> >& mstAdjList, int** adjMaxtrix);
    void Prim               (const int& nid, const int& netSize, std::vector<std::vector<int> >& mstAdjList, int** adjMaxtrix);
    void topologicalSortBFS (const int& nid, const int* indexMap, const std::vector<std::vector<int> >& mstAdjList);
    void topologicalSortUCS (const int& nid, const int* indexMap, const std::vector<std::vector<int> >& mstAdjList, int** adjMaxtrix);
    int  getMSTCost         (const int& nid) {return _mst_cost[nid];}
    auto getSortedFPGAs     (const int& nid) -> std::vector<fpga_id>& {return _sorted_mst[nid];}

private:
    BFSdb&                             _bfsdb;
    std::vector<std::vector<fpga_id> > _sorted_mst;
    std::vector<int>                   _mst_cost;

    void addToQueue         (const std::vector<bool>& connected, KruskalQueue& Q, const int& latestPoint, const int& netSize, int** adjMaxtrix);
};

}
#endif /* __MST_H__ */
