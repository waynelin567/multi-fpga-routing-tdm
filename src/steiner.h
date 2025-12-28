#ifndef __STEINER_H__
#define __STEINER_H__

#pragma GCC system_header
#include "paal/greedy/steiner_tree_greedy.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/version.hpp>

#include "def.h"
#include "net.h"

namespace router
{

struct boostGraphMgr
{
    typedef boost::property<boost::edge_weight_t, EdgeCost>                                           EdgeProp;
    typedef boost::property<boost::vertex_color_t, int>                                               VertexProp;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProp, EdgeProp> Graph;
    typedef std::pair<router::fpga_id, router::fpga_id>                                               Edge;
    typedef std::vector<router::fpga_id>                                                              Terminals;

    Graph   _g;

    void    SetDefaultGraph ();
    void    SetWeight       (const std::vector<EdgeCost>&);
    Graph&  GetGraph        () { return _g; }

    static boostGraphMgr& get_instance() {
        static boostGraphMgr m;
        return m;
    }
};

inline boostGraphMgr& get_boostGraphMgr() { return boostGraphMgr::get_instance(); }

class Steiner
{
public:
    Steiner() { /*this->version_check();*/ }
    ~Steiner() {}

    void greedy(const std::vector<std::pair<net_id, std::tuple<double, double, double, double> > >&, int);

private:
    void version_check() {
        auto major = BOOST_VERSION / 100000;
        auto minor = BOOST_VERSION / 100 % 1000;
        auto patch = BOOST_VERSION % 100;
        std::cout << "Using Boost "
                  << major << "."  // major version
                  << minor << "."  // minor version
                  << patch;        // patch level
        if (major < 1) { cout << "Minimum Boost Version: 1.58.0! Program Abort." << endl; exit(-1); }
        if (major == 1 && minor < 58) { cout << "Minimum Boost Version: 1.58.0! Program Abort." << endl; exit(-1); }
        if (major == 1 && minor == 58 && patch < 0) { cout << "Minimum Boost Version: 1.58.0! Program Abort." << endl; exit(-1); }
        cout << " >= Minumim Requirement: 1.58.0" << endl;
    }
};

};

#endif /* __STEINER_H__ */
