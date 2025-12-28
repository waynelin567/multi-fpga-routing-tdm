#include "net.h"
#include "mgr.h"
#include "db.h"

namespace router
{

void net::set_source( const fpga_id fid ) 
{
    _source = fid;
}

void net::add_target( const fpga_id fid ) 
{
    _targets.emplace_back( fid );
}

void net::print() const
{
    std::cout << "source: " << _source << std::endl;
    std::cout << "target:";
    for (int i = 0; i < (int)_targets.size(); i++)
    {
        std::cout << ' ' << _targets[i];
    }
    std::cout << std::endl;

    std::cout << "netgroups ";
    for (int i = 0; i < (int)_netGroups.size(); i++)
    {
        std::cout << _netGroups[i] << " ";
    }
    std::cout << std::endl;
}

void net::get_degrees(double& sum, double& max) const
{
    max = sum = (double)get_graphMgr().get_degree(_source);

    for (int i = 0; i < (int)_targets.size(); i++)
    {
        sum += (double)get_graphMgr().get_degree(_targets[i]);
        max = std::max(max, (double)get_graphMgr().get_degree(_targets[i]));
    }
}

}
