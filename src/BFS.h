#ifndef _BFS_H_
#define _BFS_H_
#include <iostream>
#include "def.h"

#pragma GCC system_header
#include <tbb/parallel_for.h>
#include <tbb/global_control.h>

namespace router
{

class BFSdb
{
public:
    BFSdb();
    void BFSfrom_src(fpga_id src);
    void calcBFSdb();
    void print();
    int  getDist(fpga_id src, fpga_id dest) const {return _dists[src][dest];}

    std::vector<int>& operator [] (int idx) {return _dists[idx];}
private:
    std::vector<std::vector<int> > _dists;




};


}
#endif
