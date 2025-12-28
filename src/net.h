#ifndef _NET_H_
#define _NET_H_
#include "def.h"
#include "tree.h"
#include "db.h"
namespace router
{

class net
{
public:
    explicit net(): _source( -1 ) {}

    fpga_id get_source        () const                         {return _source;}
    int     get_target_size   () const                         {return ( int )_targets.size();}
    int     get_netgroup_size () const                         {return ( int )_netGroups.size();}
    fpga_id get_target        ( int index ) const              {return _targets[index];}
    auto    get_netgroups     () -> std::vector<net_group_id>& {return _netGroups;}
    void    add_netgroup      ( const net_group_id gid )       {_netGroups.emplace_back(gid);}
    void    set_source        ( const fpga_id fid );
    void    add_target        ( const fpga_id fid );
    void    reserve           ( const size_t i )               { _targets.reserve(i); }
    void    print             () const;
    void    get_degrees       ( double& sum, double& max ) const;

private:
    fpga_id                   _source;
    std::vector<fpga_id>      _targets;
    std::vector<net_group_id> _netGroups;
};

}
#endif
