#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "../../src/mgr.h"
#include "../../src/db.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
BOOST_AUTO_TEST_CASE( init_net )
{
    router::mgr& _mgr = router::get_mgr();
    router::topologyMgr& _tMgr = router::get_topologyMgr();
    _mgr.read_infile("../../testcases/synopsys06.txt");
    _mgr.initBoostGraph();
    _mgr.initTopology();

    BOOST_CHECK(_tMgr.get_tree_size(11) == 258);
    BOOST_CHECK(_tMgr.get_tree_size(12) == 298);
    BOOST_CHECK(_tMgr.get_tree_size(13) == 332);
    BOOST_CHECK(_tMgr.get_tree_size(14) == 259);
    BOOST_CHECK(_tMgr.get_tree_size(193) == 61);
    BOOST_CHECK(_tMgr.get_tree_size(194) == 11);
    BOOST_CHECK(_tMgr.get_passing_tree_size(4) == 24866);
    BOOST_CHECK(_tMgr.get_passing_tree_size(5) == 22444);
    BOOST_CHECK(_tMgr.get_passing_tree_size(6) == 22436);
    BOOST_CHECK(_tMgr.get_passing_tree_size(7) == 22443);
}
BOOST_AUTO_TEST_CASE( heuristic_tdm )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.assign_TDM(router::mHeuristic);
    _mgr.CalcAndSortGroupTDM();
    _mgr.print_group_info();

    BOOST_CHECK(_mgr.GetMaxGroupID() == 0);
    BOOST_CHECK(_mgr.GetMaxGroupTDM() == 15765425828);
}
