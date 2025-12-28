#include "def.h"
namespace router
{
bool PairSecondDescend(const std::pair<net_group_id, TdmType>& p1, const std::pair<net_group_id, TdmType>& p2) {
    return p1.second > p2.second;
}
}
