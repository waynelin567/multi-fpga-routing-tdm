#ifndef _TREE_H_
#define _TREE_H_
#include "def.h"
namespace router
{

class tree
{
public:
    tree() {}
    ~tree() {}

    int       get_edge_size () const                    {return ( int )_fpga_edges.size();}
    treeEdge& get_edges     ()                          {return _fpga_edges;}
    void      add_edge      ( const fpga_edge_id& eid ) {_fpga_edges.emplace_back(eid);}
    void      print         () const;
    int       size          () const;

private:
    treeEdge _fpga_edges;
};

}
#endif /* _TREE_H_ */
