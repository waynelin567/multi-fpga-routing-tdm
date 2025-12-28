#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "../../src/mgr.h"
#include "../../src/file_writer.h"
#include <iostream>
std::string exec(const char* cmd);
BOOST_AUTO_TEST_CASE( parser )
{
    router::mgr& _mgr = router::get_mgr();
    router::graphMgr& _ggr = router::get_graphMgr();
    _mgr.read_infile ("../../testcases/synopsys01.txt");
    BOOST_CHECK (_ggr.get_fpga_size() == 43);
    BOOST_CHECK (_ggr.get_fpga_edge_size() == 214);
    BOOST_CHECK (_mgr.get_net_size() == 68456);
    BOOST_CHECK (_mgr.get_net_group_size() == 40552);

    for (router::net_id i = 0; i < _mgr.get_net_size(); i++)
    {
        BOOST_CHECK (_mgr.get_net (i).get_target_size() == 1);

    }
    BOOST_CHECK (_mgr.get_net (68455).get_source() == 42);
    BOOST_CHECK (_mgr.get_net (68454).get_source() == 42);
}
BOOST_AUTO_TEST_CASE( routing )
{
    router::mgr& _mgr = router::get_mgr();
    router::topologyMgr& _tMgr = router::get_topologyMgr();
    _mgr.initBoostGraph();
    _mgr.initTopology();
    BOOST_CHECK (_tMgr.get_passing_tree_size (0) == 623);
    BOOST_CHECK (_tMgr.get_passing_tree_size (1) == 858);
    BOOST_CHECK (_tMgr.get_passing_tree_size (2) == 663);
    BOOST_CHECK (_tMgr.get_passing_tree_size (3) == 924);
}
BOOST_AUTO_TEST_CASE( tdm )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.assign_TDM(router::mLR);
    _mgr.CalcAndSortGroupTDM();
    _mgr.print_group_info(true);
    router::getWriter().output_result("out.txt");
    BOOST_CHECK(_mgr.GetMaxGroupID() == 26920);
    BOOST_CHECK(_mgr.GetMaxGroupTDM() == 37030);
    std::string result = exec("../../evaluator/evaluatorFast ../../testcases/synopsys01.txt out.txt");
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
    BOOST_CHECK(ans == 37030);
    BOOST_CHECK(_mgr.GetMaxGroupTDM() == (unsigned long long int)ans);
    std::cerr << "ans: " << ans << std::endl;
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
