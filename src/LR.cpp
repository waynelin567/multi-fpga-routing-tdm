#include "LR.h"
#include "mgr.h"
#include "db.h"
#include "TDM_heuristic.h"
#include <tuple>
namespace router
{

LR::LR():
    _bestZ(std::numeric_limits<double>::max()),
    _z(0),
    _window_size(10)
{
    _assignEnd = _legalEnd = std::numeric_limits<double>::max();
    _eps = get_OptionMgr().Epsilon;

    mgr& _m = get_mgr();
    graphMgr& _g = get_graphMgr();

    _lambdaNG.resize(_m.get_net_group_size());
    _lambdaT.resize(_m.get_net_size(), 0);

    _patterns.resize(_g.get_fpga_edge_size());

    _sigmas.resize(_m.get_net_group_size());
    _netGroupTDMs.resize(_m.get_net_group_size(), 0);
    _mean_norm_GTDM.resize(_m.get_net_group_size(), 0);
    _std_norm_GTDM.resize(_m.get_net_group_size(), 0);
    _samples_GTDM.resize(_m.get_net_group_size());

    for (auto& pt : _samples_GTDM) {
        pt = new double[_window_size];
        for (int i = 0; i < _window_size; ++i) pt[i] = 0.0;
    }

    initVecs();
}
void LR::initVecs()
{
    initLambdaNG();
    calcLambdaT();
    initSigmasAndPatterns();
    calcCauchy();
}
void LR::optimize()
{
    _iteration = 0; 
    int limit = 500;
    calcZ(_patterns, true);
    while(_iteration <= limit)
    {
        _iteration ++;
        updateLambdaNG();
        calcLambdaT();
        double L_of_lambda = calcCauchy();
        calcZ(_patterns, true);

        if ( get_OptionMgr().ShowLRprocess )
        {
            std::cout << "iteration " << _iteration << " "
                      << std::fixed << "_z " << _z << " "
                      << std::fixed << "L_of_lambda " << L_of_lambda << std::endl;
        }

        if (std::fabs(_z-L_of_lambda)/L_of_lambda <= _eps) break;
    }

    takeCeiling(); _assignEnd = _legalEnd = getWalltime();
    if ( get_OptionMgr().TDMrefinement )
    {
        round();
        _legalEnd = getWalltime();
    }
    apply();
}
void LR::initLambdaNG()
{
    std::fill(_lambdaNG.begin(), _lambdaNG.end(), 1.0/get_mgr().get_net_group_size());
}
void LR::calcLambdaT()
{
    mgr& _m = get_mgr();
    tbb::parallel_for(int(0), (int)_lambdaT.size(), int(1),
        [&](const int& nid){
            _lambdaT[nid] = 0;
            const auto& NG_ids = _m.get_net(nid).get_netgroups();
            for (int i = 0; i < (int)NG_ids.size(); i++)
            {
                _lambdaT[nid] += _lambdaNG[NG_ids[i]];
            }
            assert(_lambdaT[nid] != 0);
        }
    );
}
void LR::initSigmasAndPatterns()
{
    mgr& _m = get_mgr();
    graphMgr& _g = get_graphMgr();
    std::vector<std::vector<int> > edgeTreeIDs (_m.get_net_size());
    for (int i = 0; i < (int)edgeTreeIDs.size(); i++)
    {
        edgeTreeIDs[i].resize (_g.get_fpga_edge_size(), -1);
    }
    tbb::parallel_for (int(0), _g.get_fpga_edge_size(), int(1),
    [&] (const int& eid)
    {
        const vTrees& trees = get_topologyMgr().get_passing_trees(eid);
        _patterns[eid].reserve (get_topologyMgr().get_passing_tree_size(eid));
        for (int nid = 0; nid < (int)trees.size(); nid++)
        {
            if (trees[nid].first == true)
            {
                _patterns[eid].emplace_back (nid, 0);
                edgeTreeIDs[nid][eid] = ((int)_patterns[eid].size()-1);
            }
        }
    }
    );
    tbb::parallel_for (int(0), _m.get_net_group_size(), int(1),
                       [&] (const int& gid)
    {
        const auto& nets = _m.get_net_group (gid).get_nets();
        for (int i = 0; i < (int)nets.size(); i++)
        {
            for (int eid = 0; eid < (int) edgeTreeIDs[nets[i]].size(); eid++)
            {
                if (edgeTreeIDs[nets[i]][eid] != -1)
                    _sigmas[gid].emplace_back (eid, edgeTreeIDs[nets[i]][eid]);
            }
        }
    }
    );
}
void LR::calcZ(std::vector<std::vector<std::pair<net_id, double> > >& patternVec, bool recordBest)
{
    tbb::parallel_for(size_t(0), _sigmas.size(), size_t(1),
        [&](const size_t& ngid) {
            _netGroupTDMs[ngid] = 0;
            for (int i = 0; i < (int)_sigmas[ngid].size(); i++)
            {
                _netGroupTDMs[ngid] += patternVec[_sigmas[ngid][i].first][_sigmas[ngid][i].second].second;
            }
        }
    );
    _z = *std::max_element(_netGroupTDMs.begin(), _netGroupTDMs.end());
    if (recordBest && ( _bestZ > _z ))
    {
        _bestZ = _z;
    }
    tbb::parallel_for(size_t(0), _netGroupTDMs.size(), size_t(1), 
            [&](const size_t& i)
        { 
            double newVal = _netGroupTDMs[i] / _z;
            _samples_GTDM[i][_iteration%10] = newVal;
            _mean_norm_GTDM[i] = 0;
            int nSamples = 0;
            for (; nSamples < 10; ++nSamples) {
                if ( _samples_GTDM[i][nSamples] == 0.0 ) break;
                _mean_norm_GTDM[i] += _samples_GTDM[i][nSamples];
            }
            _mean_norm_GTDM[i] /= (nSamples+1); 
        }
    );
    tbb::parallel_for(size_t(0), _netGroupTDMs.size(), size_t(1), 
            [&](const size_t& i)
        { 
            auto& std = _std_norm_GTDM[i];
            int nSamples = 0;
            for (; nSamples < _window_size; ++nSamples) {
                if ( _samples_GTDM[i][nSamples] == 0.0 ) break;
                std += std::pow(_samples_GTDM[i][nSamples] - _mean_norm_GTDM[i], 2);
            }
            std /= (double)(nSamples + 1);
            std = std ? std::sqrt(std) : 1;
        }
    );
} 
double LR::calcCauchy()
{
    double ret = tbb::parallel_reduce(tbb::blocked_range<int>(0, (int)_patterns.size()), 0.0,
        [&](const tbb::blocked_range<int>& r, double total) {
            for (int eid = r.begin(); eid < r.end(); eid++)
            {
                double nominator = 0;
                for (int i = 0; i < (int)_patterns[eid].size(); i++)
                {
                    nominator += std::sqrt (_lambdaT[_patterns[eid][i].first]);
                }
                total += std::pow(nominator, 2);
                for (int i = 0; i < (int)_patterns[eid].size(); i++)
                {
                    _patterns[eid][i].second = nominator / std::sqrt (_lambdaT[_patterns[eid][i].first]);
                }
            } 
            return total;
        }, std::plus<double>() );    
    return ret;
}
void LR::updateLambdaNG()
{
    tbb::parallel_for(int(0), (int)_lambdaNG.size(), int(1),
        [&](const int& ngid)
        {
            double tmp = ( _netGroupTDMs[ngid] / _z );
            _lambdaNG[ngid] = _lambdaNG[ngid] * std::pow(tmp , getExponent(tmp, ngid));
        }
    );
    double lambdaSum = tbb::parallel_reduce(tbb::blocked_range<int>(0, (int)_lambdaNG.size()), 0.0,
        [&](const tbb::blocked_range<int>& r, double total)
        {
            for (int i = r.begin(); i < r.end(); ++i)
            {
                total += _lambdaNG[i];
            }
            return total;
        }, std::plus<double>() );
    tbb::parallel_for_each(_lambdaNG.begin(), _lambdaNG.end(), [&](double& val){ val /= lambdaSum; });
}
void LR::takeCeiling()
{
    for (int eid = 0; eid < (int)_patterns.size(); eid++)
    {
        for (int i = 0; i < (int)_patterns[eid].size(); i++ )
        {
            _patterns[eid][i].second = std::ceil(_patterns[eid][i].second);
            if (( (TdmType)_patterns[eid][i].second ) % 2 == 1)
            {
                _patterns[eid][i].second ++;
            }
        }
    }
    calcZ(_patterns, false); 
    std::cout << "_z after ceiling " << _z << std::endl;
}
void LR::calcMaxGroupTDM(std::vector<double>& maxGroupTDM)
{
    mgr& _m = get_mgr();
    for (net_group_id gid = 0; gid < _m.get_net_group_size(); ++gid) 
    {
        const auto& g = _m.get_net_group(gid);
        for (int i = 0; i < g.get_size(); ++i) 
        {
            maxGroupTDM[g.get_net(i)] = std::max(maxGroupTDM[g.get_net(i)], _netGroupTDMs[gid]);
        }
    }
}
void LR::round()
{
    std::vector<double> maxGroupTDM(get_mgr().get_net_size());
    calcMaxGroupTDM(maxGroupTDM);
    std::vector<double> checkSum(get_graphMgr().get_fpga_edge_size(), 0.0);

    tbb::parallel_for(int(0), (int)get_graphMgr().get_fpga_edge_size(), int(1),
        [&](const int& eid)
        {
        if (_patterns[eid].empty()) return;
        for (const auto& val : _patterns[eid]) checkSum[eid] += 1.0 / val.second;
        if ( checkSum[eid] >= 1.0 - TOL ) return;

        // the graeter the tdm value, the higher the priority
        auto cmp = [&](const std::pair<net_id, double>& d1, const std::pair<net_id, double>& d2)
                    {
                        if ( maxGroupTDM[d1.first] == maxGroupTDM[d2.first] ) return d1.second > d2.second;
                        return maxGroupTDM[d1.first] > maxGroupTDM[d2.first];
                    };
        std::sort(_patterns[eid].begin(), _patterns[eid].end(), cmp);

        // determine threshold
        auto roundingThreshold = maxGroupTDM[_patterns[eid][0].first];
        if (_patterns[eid].size() > 1) {
            const auto& Fi = maxGroupTDM[_patterns[eid][0].first];
            const auto& Se = maxGroupTDM[_patterns[eid][1].first];
            auto diff = ( Fi - Se ) / Se;
            if ( diff < 0.0023 ) roundingThreshold = maxGroupTDM[_patterns[eid][1].first];
        }

        size_t virtualSize = _patterns[eid].size();
        for (size_t i = 1; i < _patterns[eid].size(); ++i)
        {
            if ( maxGroupTDM[_patterns[eid][i].first] < roundingThreshold)
            {
                virtualSize = i;
                break;
            }
        }

        // target decrement 'x*', number of the max value 'n' and the margin
        double n = 1.0, x_star, margin = 1.0 - TOL - checkSum[eid];

        // make a reference
        decltype(_patterns[eid])& ordered_pst = _patterns[eid];

        while ( margin > 0 )
        {
            // the max tdm on the edge is 2, which means that there is no space left for improvement
            if ( ordered_pst[0].second == 2.0 ) return;

            double oldtdm = ordered_pst[0].second;

            // calculate 'n' and 'x'
            for (; n < (double)virtualSize; n += 1)
            {
                if ( ordered_pst[(int)n].second != ordered_pst[0].second ) break;
            }
            if ( (size_t)n == virtualSize )
            {
                x_star = std::numeric_limits<decltype(x_star)>::max();
            }
            else
            {
                x_star = ordered_pst[(int)(n-1)].second - ordered_pst[(int)n].second;
            }

            double decrement = ( oldtdm * oldtdm * margin ) / ( n + oldtdm * margin );
            assert( decrement > 0 );
            if ( decrement > x_star )
            {
                decrement = x_star;
                for (int i = 0; i < (int)n; ++i)
                {
                    ordered_pst[i].second = ordered_pst[(int)n].second;
                }

                // update margin
                margin -= ( n * decrement ) / ( oldtdm * oldtdm - oldtdm * decrement );
            }
            else
            {
                decrement = std::floor(decrement);
                if ( (TdmType)decrement % 2 ) decrement -= 1;
                if ( !decrement ) break;
                for (int i = 0; i < (int)n; ++i)
                {
                    ordered_pst[i].second -= decrement;
                }

                // update margin
                margin -= ( n * decrement ) / ( oldtdm * oldtdm - oldtdm * decrement );

                break;
            }
        }

        // incremental refinement
        checkSum[eid] = 1.0 - TOL - margin;
        if ( ordered_pst[0].second != 2.0 )
        {
            for (auto& net_tdm_pair : ordered_pst)
            {
                double oldtdm = net_tdm_pair.second;
                double newtdm = oldtdm - 2;
                checkSum[eid] += -1.0 / oldtdm + 1.0 / newtdm;
                if ( checkSum[eid] < 1 - TOL ) net_tdm_pair.second = newtdm;
                else break;
            }
        }
    }
    );
}
double LR::getExponent(const double& normGTDM, const net_group_id& ngid)
{
#define AMP 3.0
#define INPUT_SCALE 10
    auto x = ( normGTDM - _mean_norm_GTDM[ngid] ) / _std_norm_GTDM[ngid];
    double exponent;
    exponent = AMP / ( 1 + std::exp( - INPUT_SCALE * ( x ) ) ) + 1;
    // cout << "x: " << x << endl;
    // cout << " exponent: " << exponent << endl;
    return exponent;
}
void LR::apply() const {
    for (fpga_edge_id eid = 0; eid < get_graphMgr().get_fpga_edge_size(); ++eid) {
        const auto& curPassingTree = _patterns[eid];
        for (const auto& net_tdm_pair : curPassingTree) {
            get_topologyMgr().add_TDM(eid, net_tdm_pair.first, (UI)net_tdm_pair.second);
        }
    }
}
}
