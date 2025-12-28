#include "TDM_heuristic.h"
#include "db.h"
#include <algorithm>

namespace router
{

TdmHeuristic::TdmHeuristic() {
    _assignEnd = _legalEnd = std::numeric_limits<double>::max();

    mgr& m = get_mgr();
    graphMgr& g = get_graphMgr();

    _tdmSum.resize(g.get_fpga_edge_size(), 0);
    _maxUsedEdges.resize(m.get_net_size(), 0);
    _passing_trees.resize(g.get_fpga_edge_size());
    std::vector<double> weightedTreeSize(m.get_net_size(), 0);
    _nEdges.resize(m.get_net_group_size(), 0);
    calcWeightedTreeSize(weightedTreeSize);
    for (net_group_id gid = 0; gid < m.get_net_group_size(); ++gid) {
        const auto& curGroup = m.get_net_group(gid);
        for (int i = 0; i < curGroup.get_size(); ++i) {
            _nEdges[gid] += weightedTreeSize[curGroup.get_net(i)]; 
        }
        for (int i = 0; i < curGroup.get_size(); ++i) {
            if (_maxUsedEdges[curGroup.get_net(i)] < _nEdges[gid])
                _maxUsedEdges[curGroup.get_net(i)] = _nEdges[gid];
        }
    }
}

void TdmHeuristic::calcWeightedTreeSize(std::vector<double>& weightedTreeSize) {
    mgr& m = get_mgr();
    graphMgr& g = get_graphMgr();
    std::vector<double> biggestGroupSize(m.get_net_size(), 0);
    std::vector<double> netGroupEdgeSize(m.get_net_group_size(), 0);
    std::vector<double> edgeWeight(g.get_fpga_edge_size(), 0);
    #pragma omp parallel num_threads(get_OptionMgr().ThreadNum)
    {
        #pragma omp for
        for (net_group_id ngid = 0; ngid < m.get_net_group_size(); ngid++)
            netGroupEdgeSize[ngid] = (double)m.get_net_group(ngid).get_edge_size();
        #pragma omp for
        for (net_id nid = 0; nid < m.get_net_size(); nid++)
        {
            const auto& ngs = m.get_net(nid).get_netgroups();
            for (int i = 0; i < (int)ngs.size(); i++)
                biggestGroupSize[nid] = std::max(biggestGroupSize[nid], netGroupEdgeSize[ngs[i]]);
        }
        #pragma omp for
        for (fpga_edge_id eid = 0; eid < g.get_fpga_edge_size(); eid++) {
            const vTrees& psstrees = get_topologyMgr().get_passing_trees(eid);
            for (int nid = 0; nid < (int)psstrees.size(); nid++)
                if (psstrees[nid].first==true) edgeWeight[eid] += biggestGroupSize[nid];
        }

        #pragma omp for
        for (net_id nid = 0; nid < m.get_net_size(); ++nid) {
            const treeEdge& edges = get_topologyMgr().get_tree_ref(nid).get_edges();
            for (const auto& eid : edges)
                weightedTreeSize[nid] += edgeWeight[eid];
        }

        #pragma omp for
        for (fpga_edge_id eid = 0; eid < g.get_fpga_edge_size(); ++eid) {
            get_topologyMgr().copy_passing_trees(eid, _passing_trees[eid]);
        }
    }
}

void TdmHeuristic::assign_TDM() {
    this->initTDM(); _assignEnd = _legalEnd = getWalltime();
    if ( get_OptionMgr().TDMrefinement )
    {
        this->round();
        _legalEnd = getWalltime();
    }
    this->apply();
}

void TdmHeuristic::initTDM() {
    // calculate the init TDM by ratio
    // for example, if on an edge and the _maxUsedEdges of the nets are 2 : 1
    // than the ratio should be 2/3 : 1/3
    // the tdm would in turn be 3/2 and 3/1
    // round them afterwards

    #pragma omp parallel for num_threads(get_OptionMgr().ThreadNum)
    for (fpga_edge_id eid = 0; eid < get_graphMgr().get_fpga_edge_size(); ++eid) {
        double sum = 0;
        for (const auto& net_tdm_pair : _passing_trees[eid]) {
            sum += _maxUsedEdges[net_tdm_pair.first];
        }
        for (auto& net_tdm_pair : _passing_trees[eid]) {
            double  tdm = sum/_maxUsedEdges[net_tdm_pair.first];
            auto    tmp = std::ceil(tdm);
            if ((double)std::numeric_limits<TdmType>::max() < tmp) {
                cerr << "ERROR: Overflow detected on edge " << eid << "! " << tmp << " > " << std::numeric_limits<TdmType>::max() << ", TDM_heuristic.cpp:line " << __LINE__ << endl;
            }
            TdmType raw = (TdmType)tmp;
            if (raw % 2) raw += 1;
            net_tdm_pair.second = (UI)raw;
            _tdmSum[eid] += 1 / (long double)net_tdm_pair.second;
        }
    }
}

void TdmHeuristic::round() {
    this->CalcGroupTDM();
    auto roundingThreshold = _group_TDMs[2].second;

    #pragma omp parallel for num_threads(get_OptionMgr().ThreadNum)
    for (fpga_edge_id eid = 0; eid < get_graphMgr().get_fpga_edge_size(); ++eid) {
        if ( _tdmSum[eid] >= 1 - TOL ) continue;
        if ( _passing_trees[eid].size() < 2 ) continue;

        std::vector<std::pair<double, net_id> > ordered_pst;
        ordered_pst.reserve(_passing_trees[eid].size());
        for (const auto& net_tdm_pair : _passing_trees[eid]) {
            ordered_pst.emplace_back(_maxGroupTDM[net_tdm_pair.first], net_tdm_pair.first);
        }
        std::sort(ordered_pst.begin(), ordered_pst.end(), std::greater<std::pair<double, net_id> >());

        // only decrease TDM on nets that belongs to a group with TDM-sum greater than the threshold
        for (int i = 0; i < (int)ordered_pst.size(); ++i) {
            if ( ordered_pst[i].first < roundingThreshold ) {
                ordered_pst.resize(i);
                break;
            }
        }
        if ( ordered_pst.empty() ) continue;

        // sort them again according to the tdm value
        for (int i = 0; i < (int)ordered_pst.size(); ++i) {
            ordered_pst[i].first = (double)(*(_passing_trees[eid].find(ordered_pst[i].second))).second;
        }
        std::sort(ordered_pst.begin(), ordered_pst.end(), std::greater<std::pair<double, net_id> >());

        // target decrement 'x*', number of the max value 'n' and the margin
        double n = 1.0, x_star, margin = 1.0 - TOL - (double)_tdmSum[eid];

        while ( margin > 0 )
        {
            // the max tdm on the edge is 2, which means that there is no space left for improvement
            if ( ordered_pst[0].first == 2.0 ) break;

            double oldtdm = ordered_pst[0].first;

            // calculate 'n' and 'x'
            for (; n < (double)ordered_pst.size(); n += 1)
            {
                if ( ordered_pst[(int)n].first != ordered_pst[0].first ) break;
            }
            if ( (size_t)n == ordered_pst.size() )
            {
                x_star = std::numeric_limits<decltype(x_star)>::max();
            }
            else
            {
                x_star = ordered_pst[(int)(n-1)].first - ordered_pst[(int)n].first;
            }

            double decrement = ( oldtdm * oldtdm * margin ) / ( n + oldtdm * margin );
            assert( decrement > 0 );
            if ( decrement > x_star )
            {
                decrement = x_star;
                for (int i = 0; i < (int)n; ++i)
                {
                    ordered_pst[i].first = ordered_pst[(int)n].first;
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
                    ordered_pst[i].first -= decrement;
                }

                // update margin
                margin -= ( n * decrement ) / ( oldtdm * oldtdm - oldtdm * decrement );

                break;
            }
        }

        // incremental refinement
        _tdmSum[eid] = 1.0 - TOL - margin;
        if ( ordered_pst[0].first != 2.0 )
        {
            for (auto& tdm_net_pair : ordered_pst)
            {
                double oldtdm = tdm_net_pair.first;
                double newtdm = oldtdm - 2;
                _tdmSum[eid] += -1.0 / oldtdm + 1.0 / newtdm;
                if ( _tdmSum[eid] < 1 - TOL )
                {
                    tdm_net_pair.first = newtdm;
                }
                else
                {
                    auto target_tree = _passing_trees[eid].find(tdm_net_pair.second);
                    (*target_tree).second = (UI)tdm_net_pair.first;
                    break;
                }
            }
        }
    }
}


void TdmHeuristic::CalcGroupTDM() {
    mgr& m = get_mgr();
    std::vector<double> net_TDMs(m.get_net_size(), 0);
    _group_TDMs.resize(m.get_net_group_size());

    # pragma omp parallel num_threads(get_OptionMgr().ThreadNum)
    {
        # pragma omp for
        for (net_id nid = 0; nid < m.get_net_size(); ++nid) {
            auto& edges = get_topologyMgr().get_tree_edges(nid);
            for (auto& eid : edges) {
                net_TDMs[nid] += (double)(*(_passing_trees[eid].find(nid))).second;
            }
        }

        # pragma omp for
        for (net_group_id gid = 0; gid < m.get_net_group_size(); ++gid) {
            const auto& g = m.get_net_group(gid);
            double tdm = 0;
            for (int i = 0; i < g.get_size(); ++i) {
                tdm += net_TDMs[g.get_net(i)];
            }
            _group_TDMs[gid] = std::make_pair(gid, tdm);
        }
    }

    _maxGroupTDM.clear();
    _maxGroupTDM.resize(m.get_net_size(), 0);
    for (net_group_id gid = 0; gid < m.get_net_group_size(); ++gid) {
        const auto& g = m.get_net_group(gid);
        for (int i = 0; i < g.get_size(); ++i) {
            _maxGroupTDM[g.get_net(i)] = std::max(_maxGroupTDM[g.get_net(i)], _group_TDMs[gid].second);
        }
    }

    std::sort(_group_TDMs.begin(), _group_TDMs.end(), PairSecondDescend);
}

void TdmHeuristic::apply() const {
    for (fpga_edge_id eid = 0; eid < get_graphMgr().get_fpga_edge_size(); ++eid) {
        const auto& curPassingTree = _passing_trees[eid];
        for (const auto& net_tdm_pair : curPassingTree) {
            get_topologyMgr().add_TDM(eid, net_tdm_pair.first, net_tdm_pair.second);
        }
    }
}

void TdmHeuristic::print_group_info(bool detailed) const {
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
    cout << endl;
}

}
