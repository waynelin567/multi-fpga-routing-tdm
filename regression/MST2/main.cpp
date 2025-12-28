#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "../../src/mgr.h"
#include "../../src/MST.h"
#include "../../src/BFS.h"
#include "../../src/db.h"
#include <iostream>
BOOST_AUTO_TEST_CASE( parser )
{
    router::mgr& _mgr = router::get_mgr();
    _mgr.read_infile("../../testcases/synopsys02.txt");
    router::BFSdb bfsdb;
    router::MST mst(bfsdb);
    bfsdb.calcBFSdb();
    mst.run();
    BOOST_CHECK(mst.getMSTCost(0) == 2);
    BOOST_CHECK(mst.getMSTCost(100) == 17);
    BOOST_CHECK(mst.getMSTCost(10000) == 3);
    BOOST_CHECK(mst.getMSTCost(12345) == 4);
    BOOST_CHECK(mst.getMSTCost(13579) == 5);
    BOOST_CHECK(mst.getMSTCost(24862) == 3);
    BOOST_CHECK(mst.getMSTCost(1024) == 3);
    BOOST_CHECK(mst.getMSTCost(8787) == 47);
    BOOST_CHECK(mst.getMSTCost(7878) == 8);
    BOOST_CHECK(mst.getMSTCost(6969) == 19);
    BOOST_CHECK(mst.getMSTCost(35154) == 2);
    BOOST_CHECK(mst.getMSTCost(31524) == 4);
    BOOST_CHECK(mst.getMSTCost(8621) == 3);

    auto& nodes = mst.getSortedFPGAs(100);
    int   check[15] = {1, 6, 49, 17, 18, 27, 34, 38, 32, 41, 42, 51, 0, 21, 22};
    for (int i = 0; i < (int)nodes.size(); ++i) {
        BOOST_CHECK( nodes[i] == check[i] );
    }
}
