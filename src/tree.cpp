#include "tree.h"

namespace router
{

void tree::print() const
{
    std::cout << "Edges: ";
    for ( const auto& it : _fpga_edges )
    {
        std::cout << it << " ";
    }
    std::cout << std::endl;
}

int tree::size() const
{
    return (int)_fpga_edges.size();
}

}
