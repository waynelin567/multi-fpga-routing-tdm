#include <iostream>
#include "ArgumentParser.h"
#include "mgr.h"
#include "file_writer.h"
#include "parser.h"
#include <tbb/global_control.h>

int main( int argc, char** argv )
{
    if ( !ParseArguments(argc, argv) ) return 0;
    auto& Param = router::get_OptionMgr();

    // Initialize TBB thread pool once at startup
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, Param.ThreadNum);

    if ( !Param.InputTopologyName.compare("None") )
    {
        router::get_mgr().read_infile( Param.InputFileName );
        router::get_mgr().Run();
        router::getWriter().output_result( Param.OutputFileName );
    }
    else
    {
        router::get_mgr().read_infile( Param.InputFileName );
        router::parser my_parser;
        my_parser.read_topology( Param.InputTopologyName );
        router::get_mgr().assign_TDM( router::TdmMethod(Param.TDMmethod) );
        router::get_mgr().reportResult();
        router::get_mgr().reportTimer();
    }
    return 0;
}
