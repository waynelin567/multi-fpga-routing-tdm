#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "../../src/mgr.h"
#include "../../src/file_writer.h"
#include <iostream>
BOOST_AUTO_TEST_CASE( routing )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.read_infile ("../../testcases/synopsys02.txt");
    router::topologyMgr& _tMgr = router::get_topologyMgr();
    _mgr.groupRoute();
    BOOST_CHECK (_tMgr.get_passing_tree_size (0) == 3069);
    BOOST_CHECK (_tMgr.get_passing_tree_size (1) == 3664);
    BOOST_CHECK (_tMgr.get_passing_tree_size (2) == 2785);
    BOOST_CHECK (_tMgr.get_passing_tree_size (3) == 2390);
    _mgr.initBoostGraph();
    _mgr.ripUpReroute();
    BOOST_CHECK (_tMgr.get_passing_tree_size (0) == 3336);
    BOOST_CHECK (_tMgr.get_passing_tree_size (1) == 3332);
    BOOST_CHECK (_tMgr.get_passing_tree_size (2) == 2485);
    BOOST_CHECK (_tMgr.get_passing_tree_size (3) == 2418);
    BOOST_CHECK (_tMgr.get_tree_size(11) == 55);
    BOOST_CHECK (_tMgr.get_tree_size(12) == 53);
    BOOST_CHECK (_tMgr.get_tree_size(13) == 53);
    BOOST_CHECK (_tMgr.get_tree_size(14) == 50);
    BOOST_CHECK (_tMgr.get_tree_size(193) == 15);
    BOOST_CHECK (_tMgr.get_tree_size(194) == 15);
    BOOST_CHECK (_tMgr.get_passing_tree_size(4) == 2413);
    BOOST_CHECK (_tMgr.get_passing_tree_size(5) == 2417);
    BOOST_CHECK (_tMgr.get_passing_tree_size(16) == 2473);
    BOOST_CHECK (_tMgr.get_passing_tree_size(47) == 2413);
}
    
BOOST_AUTO_TEST_CASE( tdm )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.assign_TDM(router::mLR);
    _mgr.CalcAndSortGroupTDM();
    _mgr.print_group_info(true);
    router::getWriter().output_result("out.txt");
    BOOST_CHECK(_mgr.GetMaxGroupID() == 0);
    BOOST_CHECK(_mgr.GetMaxGroupTDM() == 31615478);
}
