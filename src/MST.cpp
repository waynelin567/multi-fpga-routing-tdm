#include "MST.h"
#include "fiboqueue.h"

namespace router
{

struct fiboComp
{
    std::vector<int>& key;

    fiboComp(std::vector<int>& val) : key(val) {}
    inline bool operator() (int lhs, int rhs) const
    {
        return key[lhs] < key[rhs];
    }
};

MST::MST(BFSdb& bfsdb):
    _bfsdb(bfsdb)
{
    _sorted_mst.resize(get_mgr().get_net_size());
    _mst_cost.resize(get_mgr().get_net_size(), 0);
}

void MST::run() {
    tbb::parallel_for(0, get_mgr().get_net_size(), 1,
        [&](const int& nid) {
            this->generateMST(nid);
        }
    );
}

void MST::generateMST(const int& nid) {
    const auto& netSize    = get_mgr().get_net(nid).get_target_size() + 1;
    assert (netSize > 1);
    fpga_id*    indexMap   = new fpga_id[netSize];
    int**       adjMaxtrix = new int*[netSize];
    for (int i = 0; i < netSize; ++i) adjMaxtrix[i] = new int[netSize];

    // fill in values
    indexMap[0] = get_mgr().get_net(nid).get_source();
    for (int i = 1; i < netSize; ++i) indexMap[i] = get_mgr().get_net(nid).get_target(i-1);

    // only the upper half is used
    for (int id_1 = 0; id_1 < netSize; ++id_1) {
        for (int id_2 = id_1 + 1; id_2 < netSize; ++id_2) {
            adjMaxtrix[id_1][id_2] = _bfsdb[indexMap[id_1]][indexMap[id_2]];
        }
    }
    std::vector<std::vector<int> > mstAdjList;
//    this->Prim(nid, netSize, mstAdjList, adjMaxtrix);
    this->Kruskal(nid, netSize, mstAdjList, adjMaxtrix);

    //this->topologicalSortBFS(nid, indexMap, mstAdjList);
    this->topologicalSortUCS(nid, indexMap, mstAdjList, adjMaxtrix);

    delete [] indexMap;
    for (int i = 0; i < netSize; ++i) delete [] adjMaxtrix[i];
    delete [] adjMaxtrix;
}

void MST::Prim(const int& nid, const int& netSize, std::vector<std::vector<int> >& mstAdjList, int** adjMaxtrix) {
    std::vector<bool> visited(netSize, false);
    std::vector<int>  predecessor(netSize, -1);
    std::vector<int>  key(netSize, std::numeric_limits<int>::max());
    mstAdjList.resize(netSize);

    // start from index 0
    key[0] = 0;
    fiboComp cmp(key);
    FibQueue<int, fiboComp> Q( cmp );
    for (int i = 0; i < netSize; ++i) Q.push(i);

    while ( !Q.empty() ) {
        auto curNode = Q.top(); Q.pop();
        visited[curNode] = true;
        if (predecessor[curNode]!=-1)
        {
            const auto& endpoint = predecessor[curNode];
            mstAdjList[curNode].push_back(endpoint);
            mstAdjList[endpoint].push_back(curNode);
            if ( curNode < endpoint ) _mst_cost[nid] += adjMaxtrix[curNode][endpoint];
            else                      _mst_cost[nid] += adjMaxtrix[endpoint][curNode];
        }
        for (int node = 1; node < netSize; ++node) {
            if ( visited[node] || node == curNode)   continue;
            int cost;
            if ( curNode < node ) cost = adjMaxtrix[curNode][node];
            else                  cost = adjMaxtrix[node][curNode];
            if ( cost < key[node] ) {
                predecessor[node] = curNode;
                key[node] = cost;
                auto* fnode = Q.findNode(node);
                Q.decrease_key(fnode, node);
            }
        }
    }
}

void MST::Kruskal(const int& nid, const int& netSize, std::vector<std::vector<int> >& mstAdjList, int** adjMaxtrix) {
    std::vector<bool> connected(netSize, false);
    int isolatedNum = netSize;
    mstAdjList.resize(netSize);

    int point1 = 0, point2 = 1, min = adjMaxtrix[0][1];
    for (int id_1 = 0; id_1 < netSize-1; ++id_1) {
        for (int id_2 = id_1 + 1; id_2 < netSize; ++id_2) {
            if ( adjMaxtrix[id_1][id_2] < min ) {
                point1 = id_1;
                point2 = id_2;
                min = adjMaxtrix[id_1][id_2];
            }
        }
    }
    mstAdjList[point1].push_back(point2);
    mstAdjList[point2].push_back(point1);
    _mst_cost[nid] += min;
    assert( !connected[point1] ); connected[point1] = true;
    assert( !connected[point2] ); connected[point2] = true;
    isolatedNum -= 2;

    KruskalQueue Q;
    this->addToQueue(connected, Q, point1, netSize, adjMaxtrix);
    this->addToQueue(connected, Q, point2, netSize, adjMaxtrix);

    while ( isolatedNum ) {
        edge minEdge = Q.top(); Q.pop();
        const int& cost = std::get<0>(minEdge);
        const int& p1   = std::get<1>(minEdge);
        const int& p2   = std::get<2>(minEdge);
        if ( connected[p1] && connected[p2] ) continue;
        mstAdjList[p1].push_back(p2);
        mstAdjList[p2].push_back(p1);
        isolatedNum -= 1;
        _mst_cost[nid] += cost;
        assert( connected[p1]^connected[p2] );
        if ( !connected[p1] ) {
            connected[p1] = true;
            this->addToQueue(connected, Q, p1, netSize, adjMaxtrix);
        }
        else {
            connected[p2] = true;
            this->addToQueue(connected, Q, p2, netSize, adjMaxtrix);
        }
    }

    // this is necessary
    //for (int n = 0; n < netSize; ++n) std::sort(mstAdjList[n].begin(), mstAdjList[n].end());
}

void MST::topologicalSortUCS(const int& nid, const int* indexMap, const std::vector<std::vector<int> >& mstAdjList, int** adjMaxtrix) {
    _sorted_mst[nid].reserve(mstAdjList.size());
    std::vector<bool> visited(mstAdjList.size(), false);

    // start from the node that connects to the minimum edge
    int start = 0, min = std::numeric_limits<int>::max();
    for (int p1 = 0; p1 < (int)mstAdjList.size(); ++p1) {
        for (int i = 0; i < (int)mstAdjList[p1].size(); ++i) {
            const auto& p2 = mstAdjList[p1][i];
            if ( p1 >= p2 ) continue;
            if ( min > adjMaxtrix[p1][p2] ) {
                min = adjMaxtrix[p1][p2];
                start = p1;
            }
        }
    }

    typedef std::pair<int, int> int2d;
    std::priority_queue<int2d, std::vector<int2d>, std::greater<int2d> > ucsQ; ucsQ.emplace(0, start);
    while ( !ucsQ.empty() ) {
        const auto cur = ucsQ.top(); ucsQ.pop();
        const auto& next = cur.second;
        visited[next] = true;
        _sorted_mst[nid].push_back(indexMap[next]);
        for (const auto& n : mstAdjList[next]) {
            if ( !visited[n] ) {
                if ( next < n ) ucsQ.emplace(adjMaxtrix[next][n], n);
                else            ucsQ.emplace(adjMaxtrix[n][next], n);
            }
        }
    }
}

void MST::topologicalSortBFS(const int& nid, const int* indexMap, const std::vector<std::vector<int> >& mstAdjList) {
    _sorted_mst[nid].reserve(mstAdjList.size());
    std::vector<bool> visited(mstAdjList.size(), false);

    // start from id 0
    std::queue<int> Q; Q.push(0);
    while ( !Q.empty() ) {
        const auto cur = Q.front(); Q.pop();
        visited[cur] = true;
        _sorted_mst[nid].push_back(indexMap[cur]);
        for (const auto& n : mstAdjList[cur]) {
            if ( !visited[n] ) {
                Q.push(n);
            }
        }
    }
}

void MST::addToQueue(const std::vector<bool>& connected, KruskalQueue& Q, const int& latestPoint, const int& netSize, int** adjMaxtrix) {
    for (int node = latestPoint + 1; node < netSize; ++node) {
        if ( !connected[node] ) {
            Q.emplace(adjMaxtrix[latestPoint][node], latestPoint, node);
        }
    }
    for (int node = latestPoint - 1; node >= 0; --node) {
        if ( !connected[node] ) {
            Q.emplace(adjMaxtrix[node][latestPoint], node, latestPoint);
        }
    }
}

}
