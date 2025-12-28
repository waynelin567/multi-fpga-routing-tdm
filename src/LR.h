#ifndef _LR_H_
#define _LR_H_
#include <iostream>
#include "def.h"

#pragma GCC system_header
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_reduce.h>
#include <tbb/global_control.h>

namespace router
{

class LR
{
    typedef int tree_index;
    typedef std::pair<fpga_edge_id, tree_index> edgeTreeID;
public:
    LR();
    ~LR() { for (auto& pt : _samples_GTDM) delete [] pt; }
    void optimize();
    void getFinishTime(double& assignEnd, double& legalizationEnd) {assignEnd = _assignEnd; legalizationEnd = _legalEnd;}
private:
    void   initVecs              ();
    void   initSigmasAndPatterns ();
    void   initLambdaNG          ();
    void   calcZ                 (std::vector<std::vector<std::pair<net_id, double> > >& patternVec, bool recordBest);
    void   calcLambdaT           ();
    double calcCauchy            ();
    void   calcMaxGroupTDM       (std::vector<double>& maxGroupTDM);
    void   updateLambdaNG        ();
    void   takeCeiling           ();
    void   round                 ();
    void   apply                 () const;
    double getExponent           (const double&, const net_group_id&);
private:
    int _iteration;
    double _bestZ;
    double _z;
    double _eps;
    std::vector<double> _lambdaNG;

    //sum lambdaNG for each net
    std::vector<double> _lambdaT;
    //TDMs on edge
    std::vector<std::vector<std::pair<net_id, double> > >   _patterns;
    //sigma of each netgroup
    std::vector<std::vector<edgeTreeID> >                   _sigmas;
    std::vector<double>                                     _netGroupTDMs;

    // machenism for subgradient method
    int                                                     _window_size;
    std::vector<double>                                     _mean_norm_GTDM;
    std::vector<double>                                     _std_norm_GTDM;
    std::vector<double*>                                    _samples_GTDM;

    // runtime
    double                                                  _assignEnd;
    double                                                  _legalEnd;
};

}
#endif
