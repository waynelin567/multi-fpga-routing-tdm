#include <fstream>
#include <limits>
#include <iomanip>
#include "file_writer.h"
#include "mgr.h"
namespace router
{
void file_writer::output_result(const std::string& fname)
{
    double startTime = getWalltime();

    mgr& _m = get_mgr();
    std::string out_str;
    out_str.reserve(1 << 30);
    for (int nid = 0; nid < _m.get_net_size(); nid++)
    {
        tree& now_tree = get_topologyMgr().get_tree_ref(nid);
        append_ints(out_str, (TdmType)now_tree.get_edge_size());
        out_str.push_back('\n');
        const treeEdge& edges = now_tree.get_edges();
        for (const auto& eid : edges) {
            append_ints(out_str, (TdmType)eid);
            out_str.push_back(' ');
            append_ints(out_str, get_topologyMgr().get_TDM_value(eid, nid));
            out_str.push_back('\n');
        }
    }
    std::ofstream outfile;
    outfile.open(fname.c_str());
    outfile << out_str;
    outfile.close();

    if ( get_mgr().timerActivated() )
    {
        cout << "- Filewriter:   " << getWalltime() - startTime << "s" << endl;
        cout << "---------------------------" << endl;
    }
}
}
