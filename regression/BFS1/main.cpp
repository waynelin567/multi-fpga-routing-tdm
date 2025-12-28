#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "../../src/mgr.h"
#include "../../src/BFS.h"
#include "../../src/db.h"
#include <iostream>
BOOST_AUTO_TEST_CASE( parser )
{
    router::mgr& _mgr = router::get_mgr();
    router::graphMgr _gMgr = router::get_graphMgr();
    _mgr.read_infile("../../testcases/synopsys01.txt");
    router::BFSdb bfs;
    bfs.calcBFSdb();
    bfs.print();
    for (int i = 0; i < _gMgr.get_fpga_size(); i++)
    {
        for (int j = 0; j < _gMgr.get_fpga_size(); j++)
        {
            BOOST_CHECK(bfs.getDist(i, j) == bfs.getDist(j, i));
        }
    }
    BOOST_CHECK(bfs.getDist(0, 40) == 2);
    BOOST_CHECK(bfs.getDist(11, 40) == 1);
    BOOST_CHECK(bfs.getDist(2, 40) == 1);
    BOOST_CHECK(bfs.getDist(13, 40) == 1);
    BOOST_CHECK(bfs.getDist(4, 40) == 1);
    BOOST_CHECK(bfs.getDist(0, 5) == 1);
    BOOST_CHECK(bfs.getDist(19, 4) == 2);
    BOOST_CHECK(bfs.getDist(3, 21) == 1);
    BOOST_CHECK(bfs.getDist(10, 33) == 3);
    BOOST_CHECK(bfs.getDist(5, 24) == 2);
    BOOST_CHECK(bfs.getDist(0, 8) == 2);
}
