#ifndef _MGR_H_
#define _MGR_H_
#include <fstream>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include "net_group.h"
#include "net.h"
#include "LR.h"

#pragma GCC system_header
#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>
#include <tbb/global_control.h>

namespace router
{

enum TdmMethod      { mHeuristic, mLR };

class mgr
{
public:
    mgr() : _release(get_OptionMgr().Release),
            _analyze_runtime(get_OptionMgr().ReportRunTime),
            _parallel_level(get_OptionMgr().ThreadNum)
    { _parser_elapsed = _router_elapsed = _heuristic_elapsed = _LR_elapsed = _refine_elapsed = _dump_elapsed = std::numeric_limits<double>::max(); }

    void Run();
    void initBoostGraph();
    void initTopology();
    void assign_TDM(TdmMethod method);
    void CalcAndSortGroupTDM();

    void         reportTimer        ();
    void         reportResult       ();
    void         read_infile        ( const std::string& fname );
    net&         get_net            ( const net_id nid )       {return _nets[nid];}
    net_group&   get_net_group      ( const net_group_id gid ) {return _net_groups[gid];}
    int          get_net_size       () const                   {return ( int )_nets.size();}
    int          get_net_group_size () const                   {return ( int )_net_groups.size();}
    TdmType      GetMaxGroupTDM     () const                   {return _group_TDMs[0].second;}
    net_group_id GetMaxGroupID      () const                   {return _group_TDMs[0].first;}
    void         init_nets          (int n_num)                {_nets.resize(n_num);}
    void         init_netgroups     (int ng_num)               {_net_groups.resize(ng_num);}
    bool         isRelease          ()                         {return _release;}
    bool         timerActivated     ()                         {return _analyze_runtime;}
    void         print_group_info   (bool detailed = false) const;
    void         ripUpReroute       ();
    void         groupRoute         ();

public:
    static mgr& get_instance()
    {
        static mgr m;
        return m;
    }
private:
    bool                                            _release;
    bool                                            _analyze_runtime;
    int                                             _parallel_level;

    std::vector<net>                                _nets;
    std::vector<net_group>                          _net_groups;
    std::vector<std::pair<net_group_id, TdmType> >  _group_TDMs;

    double                                          _parser_elapsed;
    double                                          _router_elapsed;
    double                                          _heuristic_elapsed;
    double                                          _LR_elapsed;
    double                                          _refine_elapsed;
    double                                          _dump_elapsed;
};

inline mgr& get_mgr() {return mgr::get_instance();}
}

#endif
