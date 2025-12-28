#ifndef _NET_GROUP_H_
#define _NET_GROUP_H_
#include <iostream>
#include <vector>
#include "net.h"
#include "def.h"
namespace router
{

class net_group
{
public:
    explicit net_group() {}

    void         add_net       ( const net_group_id& gid, const net_id& nid );
    void         reserve       ( const size_t i ) { _nets.reserve(i); }
    void         print         () const;
    int          get_size      () const                   {return ( int )_nets.size();}
    net_id       get_net       (const int& index) const   {return _nets[index];}
    size_t       get_edge_size ();
    auto         get_nets      () -> std::vector<net_id>& {return _nets;}
private:
    std::vector<net_id> _nets;
};

}
#endif
