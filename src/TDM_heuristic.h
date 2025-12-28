#ifndef __TDM_HEURISTIC_H__
#define __TDM_HEURISTIC_H__

#include "mgr.h"

namespace router
{

class TdmHeuristic
{
public:
    TdmHeuristic();
    ~TdmHeuristic() {}

    void print_group_info     (bool detailed) const;
    void assign_TDM           ();
    void initTDM              ();
    void round                ();
    void calcWeightedTreeSize (std::vector<double>& weightedTreeSize);
    void CalcGroupTDM         ();
    void apply                () const;
    void getFinishTime        (double& assignEnd, double& legalizationEnd) { assignEnd = _assignEnd; legalizationEnd = _legalEnd; }

private:
    // records the passing trees on a specific edge, query by fpga_edge_id
    std::vector<std::unordered_map<net_id, UI> >   _passing_trees;

    // the tdm sum of each edge, used to check whether the condition is satisfied
    std::vector<long double>                            _tdmSum;

    // the maxinum edges used by a group that a net belongs to, query by net_id
    std::vector<double>                                 _maxUsedEdges;

    std::vector<std::pair<net_group_id, double> >       _group_TDMs;

    // store the maximum Group TDM that a net belongs to
    std::vector<double>                                 _maxGroupTDM;
    std::vector<double>                                 _nEdges;

    double                                              _assignEnd;
    double                                              _legalEnd;
};

}

#endif /* __TDM_HEURISTIC_H__ */
