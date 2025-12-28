#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "../../src/mgr.h"
#include "../../src/db.h"
#include "../../src/file_writer.h"
#include <iostream>
std::string exec(const char* cmd);
BOOST_AUTO_TEST_CASE( parser )
{
    router::mgr& _mgr = router::get_mgr();
    router::graphMgr& _ggr = router::get_graphMgr();
    _mgr.read_infile("../../testcases/SampleInput");
    BOOST_CHECK(_ggr.get_fpga_size() == 8);
    BOOST_CHECK(_ggr.get_fpga_edge_size() == 11);
    BOOST_CHECK(_mgr.get_net_size() == 5);
    BOOST_CHECK(_mgr.get_net_group_size() == 3);
    BOOST_CHECK(_ggr.get_degree(0) == 3);
    BOOST_CHECK(_ggr.get_degree(1) == 4);
    BOOST_CHECK(_ggr.get_degree(2) == 2);
    BOOST_CHECK(_ggr.get_degree(3) == 1);
    BOOST_CHECK(_ggr.get_degree(4) == 2);
    BOOST_CHECK(_ggr.get_degree(5) == 3);
    BOOST_CHECK(_ggr.get_degree(6) == 4);
    BOOST_CHECK(_ggr.get_degree(7) == 3);

    BOOST_CHECK(_mgr.get_net(0).get_source() == 0);
    BOOST_CHECK(_mgr.get_net(1).get_source() == 1);
    BOOST_CHECK(_mgr.get_net(2).get_source() == 5);
    BOOST_CHECK(_mgr.get_net(3).get_source() == 0);
    BOOST_CHECK(_mgr.get_net(4).get_source() == 5);

    BOOST_CHECK(_mgr.get_net(0).get_target_size() == 1);
    BOOST_CHECK(_mgr.get_net(1).get_target_size() == 1);
    BOOST_CHECK(_mgr.get_net(2).get_target_size() == 1);
    BOOST_CHECK(_mgr.get_net(3).get_target_size() == 3);
    BOOST_CHECK(_mgr.get_net(4).get_target_size() == 1);

    BOOST_CHECK(_mgr.get_net_group(0).get_size() == 3);
    BOOST_CHECK(_mgr.get_net_group(1).get_size() == 1);
    BOOST_CHECK(_mgr.get_net_group(2).get_size() == 1);

    BOOST_CHECK(_ggr.get_fpga_edge_point(7, 0) == 3 &&
                _ggr.get_fpga_edge_point(7, 1) == 7);

}
BOOST_AUTO_TEST_CASE( init_net )
{
    router::mgr& _mgr = router::get_mgr();
    router::topologyMgr& _tMgr = router::get_topologyMgr();
    _mgr.initBoostGraph();
    _mgr.initTopology();

    BOOST_CHECK(_tMgr.get_passing_tree_size(0) == 1);
    BOOST_CHECK(_tMgr.get_passing_tree_size(1) == 1);
    BOOST_CHECK(_tMgr.get_passing_tree_size(2) == 0);
    BOOST_CHECK(_tMgr.get_passing_tree_size(3) == 0);
    BOOST_CHECK(_tMgr.get_passing_tree_size(4) == 1);
    BOOST_CHECK(_tMgr.get_passing_tree_size(5) == 0);
    BOOST_CHECK(_tMgr.get_passing_tree_size(6) == 0);
    BOOST_CHECK(_tMgr.get_passing_tree_size(7) == 0);
    BOOST_CHECK(_tMgr.get_passing_tree_size(8) == 1);
    BOOST_CHECK(_tMgr.get_passing_tree_size(9) == 3);
    BOOST_CHECK(_tMgr.get_passing_tree_size(10) == 1);
}
BOOST_AUTO_TEST_CASE( TDM_assignment )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.assign_TDM(router::mLR);
    _mgr.CalcAndSortGroupTDM();
    router::getWriter().output_result("out.txt");
    std::string result = exec("../../evaluator/evaluatorFast ../../testcases/SampleInput out.txt");
    std::istringstream iss(result);
    std::string dummy;
    std::getline(iss, dummy);
    std::getline(iss, dummy);
    std::getline(iss, dummy);
    std::getline(iss, dummy);
    std::getline(iss, dummy);
    for (int i = 0; i < 11; i++)
        iss >> dummy;
    int ans;
    iss >> ans;
    BOOST_CHECK(ans == 8);
    BOOST_CHECK(_mgr.GetMaxGroupTDM() == (unsigned long long int)ans);
    std::cerr << std::fixed << "ans: " << ans << std::endl;
}
std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");

    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();

    }
    return result;

}

