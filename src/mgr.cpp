#include "db.h"
#include "mgr.h"
#include "parser.h"
#include "file_writer.h"
#include "steiner.h"
#include "TDM_heuristic.h"
#include "ripupRouter.h"
#include "BFS.h"
#include "groupRouter.h"

namespace router
{
void mgr::Run() {
    this->initBoostGraph();
    this->initTopology();
    this->assign_TDM(TdmMethod(get_OptionMgr().TDMmethod));
    this->reportResult();
    this->reportTimer();
}
void mgr::groupRoute()
{
    groupRouter gR;
    gR.route(_parallel_level);
}
void mgr::ripUpReroute()
{
    RipUpRouter ruR;
    ruR.run();
}
void mgr::initBoostGraph()
{
    get_boostGraphMgr().SetDefaultGraph();
}
void mgr::initTopology()
{
    double startTime = getWalltime();

    this->groupRoute();
    this->ripUpReroute();

    if ( _analyze_runtime ) _router_elapsed = getWalltime() - startTime;
}
void mgr::CalcAndSortGroupTDM()
{
    std::vector<TdmType> net_TDMs(get_net_size(), 0);
    _group_TDMs.resize(_net_groups.size());
    tbb::parallel_for(0, (int)_nets.size(), 1,
        [&](const int& nid)
        {
            net_TDMs[nid] = get_topologyMgr().CalcTDM(nid);
        }
    );
    tbb::parallel_for(0, (int)_net_groups.size(), 1,
        [&](const int& gid)
        {
            const auto& g = _net_groups[gid];
            TdmType tdm = 0;
            for (int i = 0; i < g.get_size(); ++i) {
                tdm += net_TDMs[g.get_net(i)];
            }
            _group_TDMs[gid] = std::make_pair(gid, tdm);
        }
    );
    tbb::parallel_sort(_group_TDMs.begin(), _group_TDMs.end(), PairSecondDescend);
}
void mgr::print_group_info(bool detailed) const {
    cout << "===== routed result =====" << endl;
    cout << "Group ID, total TDM" << endl;
    cout << std::setw(8) << _group_TDMs[0].first << ", " << _group_TDMs[0].second  << " (MAX)" << endl;
    cout << std::setw(8) << _group_TDMs[1].first << ", " << _group_TDMs[1].second << endl;
    if (!detailed) {
        cout << "=========================" << endl;
        cout << endl;
        return;
    }
    cout << std::setw(8) << _group_TDMs[2].first << ", " << _group_TDMs[2].second << endl;

    cout << std::setw(8) << '.' << endl;
    cout << std::setw(8) << '.' << endl;
    cout << std::setw(8) << '.' << endl;
    cout << std::setw(8) << _group_TDMs[_group_TDMs.size()-3].first << ", " << _group_TDMs[_group_TDMs.size()-3].second << endl;
    cout << std::setw(8) << _group_TDMs[_group_TDMs.size()-2].first << ", " << _group_TDMs[_group_TDMs.size()-2].second << endl;
    cout << std::setw(8) << _group_TDMs[_group_TDMs.size()-1].first << ", " << _group_TDMs[_group_TDMs.size()-1].second << endl;
    cout << "=========================" << endl;

}
void mgr::reportTimer()
{
    cout << "----- Runtime Analysis ----" << endl;

    if ( !_analyze_runtime )
    {
        cout << "-       " << std::left << std::setw(16) << "Deactivated!" << "  -"<< endl;
        cout << "---------------------------" << endl;
        return;
    }

    if ( _parser_elapsed != std::numeric_limits<double>::max() )    cout << "- Parser:       " << _parser_elapsed << "s" << endl;
    if ( _router_elapsed != std::numeric_limits<double>::max() )    cout << "- Router:       " << _router_elapsed << "s" << endl;
    if ( _heuristic_elapsed != std::numeric_limits<double>::max() ) cout << "- HeuristicTDM: " << _heuristic_elapsed << "s" << endl;
    if ( _LR_elapsed != std::numeric_limits<double>::max() )        cout << "- LRTDM:        " << _LR_elapsed << "s" << endl;
    if ( _refine_elapsed    != std::numeric_limits<double>::max() ) cout << "- LRrefine:     " << _refine_elapsed    << "s" << endl;
    if ( _dump_elapsed != std::numeric_limits<double>::max() )      cout << "- ShowResult:   " << _dump_elapsed << "s" << endl;
}
void mgr::reportResult()
{
    if (!isRelease())
    {
        double startTime = getWalltime();

        this->CalcAndSortGroupTDM();
        this->print_group_info(true);

        if ( _analyze_runtime ) _dump_elapsed = getWalltime() - startTime;
    }
}
void mgr::read_infile( const std::string& fname )
{
    double startTime = getWalltime();

    parser my_parser;
    if ( !my_parser.read( fname ) )
    {
        exit( -1 );
    }

    if ( _analyze_runtime ) _parser_elapsed = getWalltime() - startTime;
}
void mgr::assign_TDM(TdmMethod method)
{
    double startTime = getWalltime();

    if (method == mHeuristic) {
        TdmHeuristic assigner;
        assigner.assign_TDM();

        if ( _analyze_runtime )
        {
            double assignEnd, legalizationEnd;
            assigner.getFinishTime(assignEnd, legalizationEnd);
            _heuristic_elapsed = assignEnd - startTime;
            _refine_elapsed    = legalizationEnd - assignEnd;
        }
    }
    else if (method == mLR){
        LR LRassigner;
        LRassigner.optimize();

        if ( _analyze_runtime )
        {
            double assignEnd, legalizationEnd;
            LRassigner.getFinishTime(assignEnd, legalizationEnd);
            _LR_elapsed = assignEnd - startTime;
            _refine_elapsed    = legalizationEnd - assignEnd;
        }
    }
    else assert(0 && "invalid method for TDM assignment!");
}
}
