#include "net_group.h"
#include "mgr.h"
namespace router
{
void net_group::add_net (const net_group_id& gid, const net_id& nid )
{
    get_mgr().get_net(nid).add_netgroup( gid );
    _nets.emplace_back( nid );
}
size_t net_group::get_edge_size()
{
    size_t ret = 0;
    for (int i = 0; i < get_size(); i++)
    {
        const tree& t = get_topologyMgr().get_tree_ref(_nets[i]);
        ret += t.get_edge_size();
    }
    return ret;
}
void net_group::print() const
{
    for (int i = 0;i < (int)_nets.size(); i++)
    {
        std::cout << _nets[i] << " ";
    }std::cout << std::endl;
}
}
