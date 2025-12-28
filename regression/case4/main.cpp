#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "../../src/mgr.h"
#include "../../src/db.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
BOOST_AUTO_TEST_CASE( init_net )
{
    router::mgr& _mgr = router::get_mgr();
    router::topologyMgr& _tMgr = router::get_topologyMgr();
    _mgr.read_infile("../../testcases/synopsys04.txt");
    _mgr.initBoostGraph();
    _mgr.initTopology();

    BOOST_CHECK(_tMgr.get_tree_size(11) == 60);
    BOOST_CHECK(_tMgr.get_tree_size(12) == 64);
    BOOST_CHECK(_tMgr.get_tree_size(13) == 50);
    BOOST_CHECK(_tMgr.get_tree_size(14) == 96);
    BOOST_CHECK(_tMgr.get_tree_size(193) == 38);
    BOOST_CHECK(_tMgr.get_tree_size(194) == 23);

    BOOST_CHECK(_tMgr.get_passing_tree_size(4) == 7637);
    BOOST_CHECK(_tMgr.get_passing_tree_size(5) == 7640);
    BOOST_CHECK(_tMgr.get_passing_tree_size(6) == 7635);
    BOOST_CHECK(_tMgr.get_passing_tree_size(7) == 7639);
}
BOOST_AUTO_TEST_CASE( heuristic_tdm )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.assign_TDM(router::mHeuristic);
    _mgr.CalcAndSortGroupTDM();
    _mgr.print_group_info();

    BOOST_CHECK(_mgr.GetMaxGroupID() == 0);
    BOOST_CHECK(_mgr.GetMaxGroupTDM() == 7055976);
}
