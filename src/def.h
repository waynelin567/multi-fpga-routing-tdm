#ifndef _DEF_H_
#define _DEF_H_
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <omp.h>
#include <set>
#include <unordered_map>
#include <map>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <time.h>
#include <sys/time.h>

using std::cout;
using std::endl;
using std::cerr;
using std::ofstream;

namespace router
{
#define TOL 1.0e-6
#define MAXTHREAD 8

typedef int net_group_id;
typedef int net_id;
typedef int fpga_id;
typedef int fpga_edge_id;
typedef int tree_id;
typedef double EdgeCost;
typedef unsigned long long int ULLI;
typedef unsigned int UI;
typedef ULLI TdmType;
typedef std::vector<std::pair<bool, UI> > vTrees;
typedef std::vector<fpga_edge_id> treeEdge;

bool PairSecondDescend(const std::pair<net_group_id, TdmType>& p1, const std::pair<net_group_id, TdmType>& p2);

inline double getWalltime ()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
inline double getCPUtime ()
{
    return (double)clock() / CLOCKS_PER_SEC;
}

struct ProgramOpt
{
    ProgramOpt()
    {
        ThreadNum = MAXTHREAD;
        TDMmethod = 1;
        RipUpCount = 1;
        Release = false;
        TDMrefinement = true;
        Epsilon = 0.0027;
    }
    static ProgramOpt& get_instance()
    {
        static ProgramOpt param;
        return param;
    }

    // global parameters
    int             ThreadNum;
    int             TDMmethod;
    int             RipUpCount;
    bool            ShowLRprocess;
    bool            ReportRunTime;
    bool            Release;
    bool            TDMrefinement;
    std::string     InputFileName;
    std::string     OutputFileName;
    std::string     InputTopologyName;
    double          Epsilon;
};
inline ProgramOpt& get_OptionMgr() { return ProgramOpt::get_instance(); }

}

#endif
