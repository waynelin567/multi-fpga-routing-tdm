#include "BFS.h"
#include "db.h"
#include <iomanip>
#include <queue>
namespace router
{

BFSdb::BFSdb()
{
    int fpga_size = get_graphMgr().get_fpga_size();
    _dists.resize(fpga_size);
    for (int i = 0; i < (int)_dists.size(); i++)
    {
        _dists[i].resize(fpga_size, -1);
    }
}
void BFSdb::calcBFSdb()
{
    tbb::parallel_for(int(0), (int)_dists.size(), int(1),
        [&](const int& fid){BFSfrom_src(fid);});

}
void BFSdb::print()
{
    std::cout << "   ";
    for (int i = 0; i < (int)_dists.size(); i++)
        std::cout << std::setw(3) << i;
    std::cout << std::endl;
    for (int i = 0; i < (int)_dists.size(); i++)
    {
        std::cout << std::setw(3) << i;
        for (int j = 0; j < (int)_dists[i].size(); j++)
        {
            if (_dists[i][j] == -1)
                std::cout << std::setw(3) << "_";
            else 
                std::cout << std::setw(3) << _dists[i][j];
        }
        std::cout << std::endl;
    }
}
void BFSdb::BFSfrom_src(fpga_id src)
{
    std::vector<bool> visited(get_graphMgr().get_fpga_size(), false);
    
    std::queue<fpga_id> Q;
    Q.push(src);
    _dists[src][src] = 0;
    visited[src] = true;

    while (!Q.empty())
    {
        fpga_id now_fpga = Q.front();
        Q.pop();

        const auto& neighbors = get_graphMgr().get_adjlist(now_fpga);
        for (const auto& neighbor_fid : neighbors)
        {
            if (!visited[neighbor_fid])
            {
                visited[neighbor_fid] = true;
                _dists[src][neighbor_fid] = _dists[src][now_fpga] + 1;
                Q.push(neighbor_fid);
            }
        }

    }
}


}
