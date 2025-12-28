#include "steiner.h"
#include "mgr.h"

namespace router
{

void boostGraphMgr::SetDefaultGraph() {
    Graph g(get_graphMgr().get_fpga_size());
    graphMgr& gmgr = get_graphMgr();
    for (int eid = 0; eid < gmgr.get_fpga_edge_size(); ++eid) {
        add_edge(gmgr.get_fpga_edge_point(eid, 0), gmgr.get_fpga_edge_point(eid, 1), EdgeProp(0), g).second;
    }
    _g.swap(g);
}

void boostGraphMgr::SetWeight(const std::vector<EdgeCost>& weight) {
    graphMgr& gmgr = get_graphMgr();
    for (fpga_edge_id eid = 0; eid < gmgr.get_fpga_edge_size(); ++eid) {
        const auto& u = gmgr.get_fpga_edge_point(eid, 0);
        const auto& v = gmgr.get_fpga_edge_point(eid, 1);
        const auto& e = boost::edge(u, v, _g);
        boost::put(boost::edge_weight_t(), _g, e.first, weight[eid]);
    }
}


void Steiner::greedy(const std::vector<std::pair<net_id, std::tuple<double, double, double, double> > >& nids, int nParallel) {
    // graph and containers' initialization
    if ((int)nids.size() < nParallel) nParallel = (int)nids.size();
    std::vector<EdgeCost> NCcost(get_graphMgr().get_fpga_edge_size(), 0.01);
    get_boostGraphMgr().SetWeight(NCcost);
    auto g     = get_boostGraphMgr().GetGraph();
    auto index = get(boost::vertex_index, g);
    typedef boost::graph_traits<decltype(g)>::edge_descriptor Edge;
    std::vector<std::set<Edge> > steinerEdges(nids.size());
    std::vector<std::vector<int> > colors(nids.size());
    for (int i = 0; i < (int)colors.size(); ++i) colors[i].resize(num_vertices(g));

    // preparation for multi-thread
    std::vector<std::vector<net_id> > nids_t(nParallel);
    size_t iter = nids.size() / nParallel;
    if (nids.size() % nParallel) ++iter;
    for (auto& v : nids_t) v.reserve(iter);
    for (int i = 0; i < (int)nids.size(); ++i) nids_t[i%nParallel].push_back(nids[i].first);
    for (auto& v : nids_t) if (v.size() < iter) v.emplace_back(-1);

    // initialize targets and source
    std::vector<bool> check(get_graphMgr().get_fpga_size(), false);
    for (int i = 0; i < (int)nids.size(); ++i) {
        const net_id& nid = nids[i].first;
        const net& curNet = get_mgr().get_net(nid);

        auto c = &colors[nid][0];

        boost::put(c, curNet.get_source(), paal::Terminals::TERMINAL);
        check[curNet.get_source()] = true;
        for (int i = 0; i < curNet.get_target_size(); ++i) {
            boost::put(c, curNet.get_target(i), paal::Terminals::TERMINAL);
            check[curNet.get_target(i)] = true;
        }

        for (int fid = 0; fid < get_graphMgr().get_fpga_size(); ++fid) {
            if (!check[fid]) boost::put(c, fid, paal::Terminals::NONTERMINAL);
            else check[fid] = false;
        }
    }

    std::vector<net_id> net_t(nParallel, -1); // stores the net that the corresponding thread is processing
    std::vector<bool>   update(get_graphMgr().get_fpga_edge_size(), false); // whether an edge needs to be updated
    #pragma omp parallel num_threads(nParallel)
    {
        for (size_t i = 0; i < iter; ++i) {
            int tid = omp_get_thread_num(); // assert(tid < (int)nids_t.size());
            const net_id& nid = nids_t[tid][i];
            if (nid != -1) {
                paal::steiner_tree_greedy(g, std::inserter(steinerEdges[nid], steinerEdges[nid].begin()),
                boost::vertex_color_map(boost::make_iterator_property_map(colors[nid].begin(), index)));
                tree newTree;
                for (const auto& e : steinerEdges[nid]) {
                    auto v1 = source(e, g);
                    auto v2 = target(e, g);
                    newTree.add_edge(get_graphMgr().get_edge_id( (fpga_id)index[v1], (fpga_id)(index[v2]) ));
                }
                get_topologyMgr().set_tree(nid, newTree);
                net_t[tid] = nid;
            }

            #pragma omp barrier
            {
                #pragma omp single
                {
                    for (const auto& n : net_t) {
                        tree& curTree = get_topologyMgr().get_tree_ref(n);
                        const treeEdge& edges = curTree.get_edges();
                        for (const auto& eid : edges) {
                            NCcost[eid]++;
                            update[eid] = true;
                        }
                    }
                    // update cost on the graph directly
                    for (fpga_edge_id eid = 0; eid < get_graphMgr().get_fpga_edge_size(); ++eid) {
                        if (!update[eid]) continue;
                        const auto& u = get_graphMgr().get_fpga_edge_point(eid, 0);
                        const auto& v = get_graphMgr().get_fpga_edge_point(eid, 1);
                        const auto& e = boost::edge(u, v, g);
                        boost::put(boost::edge_weight_t(), g, e.first, NCcost[eid]);
                        update[eid] = false;
                    }
                }
            }
        }
    }

    for (net_id nid = 0; nid < get_mgr().get_net_size(); ++nid) {
        get_topologyMgr().add_tree_to_edge(nid);
    }
}

}
