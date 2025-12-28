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
    _mgr.read_infile("../../testcases/synopsys05.txt");
    _mgr.initBoostGraph();
    _mgr.initTopology();

    BOOST_CHECK(_tMgr.get_tree_size(11) == 80);
    BOOST_CHECK(_tMgr.get_tree_size(12) == 27);
    BOOST_CHECK(_tMgr.get_tree_size(13) == 26);
    BOOST_CHECK(_tMgr.get_tree_size(14) == 44);
    BOOST_CHECK(_tMgr.get_tree_size(193) == 42);
    BOOST_CHECK(_tMgr.get_tree_size(194) == 29);
    BOOST_CHECK(_tMgr.get_passing_tree_size(4) == 5958);
    BOOST_CHECK(_tMgr.get_passing_tree_size(5) == 5950);
    BOOST_CHECK(_tMgr.get_passing_tree_size(6) == 5951);
    BOOST_CHECK(_tMgr.get_passing_tree_size(7) == 5949);
}
BOOST_AUTO_TEST_CASE( heuristic_tdm )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.assign_TDM(router::mHeuristic);
    _mgr.CalcAndSortGroupTDM();
    _mgr.print_group_info();

    BOOST_CHECK(_mgr.GetMaxGroupID() == 644891);
    BOOST_CHECK(_mgr.GetMaxGroupTDM() == 5178556);
}
