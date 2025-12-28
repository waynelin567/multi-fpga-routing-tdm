#ifndef _GROUP_ROUTER_H_
#define _GROUP_ROUTER_H_
#include <iostream>
#include <set>
#include "def.h"
#include "tree.h"
#include "MST.h"
#include "BFS.h"
namespace router
{

class AstarRouter
{
    typedef double Cost;
    typedef std::pair<Cost, fpga_id> Node;
    struct Comp
    {
        template<typename T>
        bool operator()(const T& l, const T& r) const
        {
            if (l.first == r.first) 
            {
                if (l.second != r.second) return true;
                else return false;
            }
            else return l.first < r.first;
        }

    };
public:
    AstarRouter(net_id nid, std::vector<Cost>& c1,
                std::vector<fpga_id>& fpgas, BFSdb& bfsdb);
    void route();
private:
    void doAstar(fpga_id src, fpga_id dest);
    void initSrc(fpga_id src);
    Cost getEdgeCost(fpga_edge_id eid);
    void tracePath(fpga_id src, fpga_id dest);
private:
    net_id                      _nid;
    std::vector<Cost>&          _c1;
    std::vector<fpga_id>&       _fpgas;
    BFSdb&                      _bfsdb;

    std::vector<bool>           _isVisited;
    std::vector<Cost>           _min_node_cost;
    std::vector<fpga_id>        _parent;
    std::multiset<Node, Comp>   _Q;
    tree                        _tree;
};

class groupRouter
{
public:
    groupRouter();
    void route(int nParallel);
private:
    void sortNetGroups();
    void getNetOrder();
    void getParallelNetOrder(std::vector<std::vector<net_id> >& nids_t);
    void getIntraGroupNetOrder(net_group_id gid, 
            std::vector<int>& netCounter);
private:
    BFSdb _BFSdb;
    MST   _MSTdb;
    std::vector<std::pair<int, net_group_id> > _groupOrder;
    std::vector<net_id> _netOrder;
    std::vector<double> _c1;

};

}
#endif
