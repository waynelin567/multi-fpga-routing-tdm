#include "parser.h"
#include "mgr.h"
#include "net.h"
#include "db.h"
#include <sstream>
namespace router
{
static std::string buffer;
static std::vector<int> container;
bool parser::read( const std::string& fname )
{
    std::ifstream infile;
    infile.open( fname.c_str() );
    if ( !infile.good() )
    {
        std::cout << "ERROR: failed opening infile " << fname << std::endl;
        return false;
    }
    getline(infile, buffer);
    str2ints(buffer, container);
    _fpga_num      = container[0];
    _fpga_edge_num = container[1];
    _net_num       = container[2];
    _net_group_num = container[3];
    init_vectors();
    read_fpga_edges( infile );
    read_nets( infile );
    read_net_groups( infile );
    infile.close();
    return true;
}
void parser::init_vectors()
{
    get_mgr().init_nets(_net_num);
    get_mgr().init_netgroups(_net_group_num);
    get_topologyMgr().init(_fpga_edge_num, _net_num);
    get_graphMgr().init(_fpga_num, _fpga_edge_num);
}
void parser::read_fpga_edges( std::ifstream& infile )
{
    for ( fpga_edge_id i = 0; i < _fpga_edge_num; i++ )
    {
        getline( infile, buffer );
        str2ints( buffer, container );
        get_graphMgr().add_edge( container[0], container[1], i );
    }
}
void parser::read_nets( std::ifstream& infile )
{
    for ( net_id nid = 0; nid < _net_num; nid++ )
    {
        net& n = get_mgr().get_net( nid );
        std::getline( infile, buffer );
        str2ints( buffer, container );
        n.set_source( container[0] );
        n.reserve(container.size()-1);
        for (int i = 1; i < (int)container.size(); ++i)
        {
            n.add_target( container[i] );
        }
    }
}
void parser::read_net_groups( std::ifstream& infile )
{
    for ( net_group_id gid = 0; gid < _net_group_num; gid++ )
    {
        net_group& group = get_mgr().get_net_group( gid );
        std::getline( infile, buffer );
        str2ints( buffer, container );
        group.reserve(container.size());
        for (const auto& t : container)
        {
            group.add_net(gid, t);
        }
    }
}
bool parser::read_topology(const std::string& fname)
{
    std::ifstream infile;
    infile.open( fname.c_str() );
    if ( !infile.good() )
    {
        std::cout << "ERROR: failed opening infile " << fname << std::endl;
        return false;
    }

    net_id curNet = 0;
    while (curNet < get_mgr().get_net_size()) {
        tree newtree;
        getline(infile, buffer);
        str2ints(buffer, container);
        int edgeNum = container[0];
        for (int i = 0; i < edgeNum; ++i) {
            getline(infile, buffer);
            str2ints(buffer, container);
            newtree.add_edge(container[0]);
        }
        get_topologyMgr().set_tree(curNet, newtree);
        get_topologyMgr().add_tree_to_edge(curNet);
        ++curNet;
    }

    return true;
}
}
