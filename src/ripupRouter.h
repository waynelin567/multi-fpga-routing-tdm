#ifndef _RIPUPROUTER_H_
#define _RIPUPROUTER_H_
#include <iostream>
#include "def.h"
namespace router
{

class RipUpRouter
{
public:
    RipUpRouter();
    void calcNGImportance(std::vector<std::pair<double, net_group_id> >& NGImportance);
    void setNetColors(std::vector<std::vector<int> >& colors);
    void route();
    void pickNets();
    void ripup();
    void run();
private:
    std::vector<EdgeCost>   _edgeWeight;
    std::vector<net_id>     _candidates;
    int                     _ripUpCount;
};

}
#endif

