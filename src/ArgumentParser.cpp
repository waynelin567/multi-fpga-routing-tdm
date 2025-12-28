#include "ArgumentParser.h"

namespace po = boost::program_options;

bool ParseArguments( int argc, char** argv)
{
    try
    {
        auto& Param = router::get_OptionMgr();

        // Generic options
        po::options_description generic("Generic Options");
        generic.add_options()
            ("help,h", "show this message\n")
            ("verbose,v", po::value<int>()->default_value(1)->implicit_value(1), "verbosity\n0 for nothing\n1 for showing LR process\n2 for additional runtime report")
            ;

        // options for configurations
        po::options_description config("Configuration");
        config.add_options()
            ("release", "release version or not, if enabled\nthread will be forced to 8\nverbose will be forced to 0\n")
            ("ripUp", po::value<int>(&Param.RipUpCount)->default_value(1), "number of rounds to perform ripUp & reroute\n")
            ("epsilon,e", po::value<double>(&Param.Epsilon)->default_value(0.0027), "the stopping criterion for LR\n")
            ("thread,t", po::value<int>(&Param.ThreadNum)->default_value(8), "number of threads\n")
            ("TDMmethod,m", po::value<int>(&Param.TDMmethod)->default_value(1), "the method for TDM ratio assignment\n0 for fast heuristic\n1 for quality-oriented LR\n")
            ("TDMrefinement,r", po::value<bool>(&Param.TDMrefinement)->default_value(true), "whether to perform TDM ratio refinement")
            ;

        // these are defined just for positional options and will not be showned to the user
        po::options_description hidden("Hidden Options");
        hidden.add_options()
            ("inputFile,i", po::value<std::string>(&Param.InputFileName)->default_value("None"), "input filename")
            ("outputFile,o", po::value<std::string>(&Param.OutputFileName)->default_value("None"), "output filename")
            ("topologyFile,T", po::value<std::string>(&Param.InputTopologyName)->default_value("None"), "input topology filename")
            ;

        po::options_description cmdlineOpt;
        cmdlineOpt.add(generic).add(config).add(hidden);

        po::options_description visible("Allowed Options");
        visible.add(generic).add(config);

        po::positional_options_description posOpt;
        posOpt.add("inputFile", 1);
        posOpt.add("outputFile", 1);
        posOpt.add("topologyFile", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(cmdlineOpt).positional(posOpt).run(), vm);
        po::notify(vm);

        if ( vm.count("help") )
        {
            std::cout << std::endl << "USAGE: ./router [-h, --release, -v<verbosity level>] [-etmr ARG] [--ripUp ARG] input_file output_file [topology_file]" << std::endl << std::endl;
            std::cout << "-- Option Descriptions --" << std::endl << std::endl;
            std::cout << visible << std::endl;
            return false;
        }
        if ( vm.count("inputFile") )
        {
            if ( !vm["inputFile"].as<std::string>().compare("None") )
            {
                cerr << "input file not specified" << endl;
                std::cout << std::endl << "USAGE: ./router [-h, --release, -v<verbosity level>] [-etmr ARG] input_file output_file [topology_file]" << std::endl << std::endl;
                return false;
            }
        }
        if ( vm.count("outputFile") )
        {
            if ( !vm["outputFile"].as<std::string>().compare("None") )
            {
                cerr << "output file not specified" << endl;
                std::cout << std::endl << "USAGE: ./router [-h, --release, -v<verbosity level>] [-etmr ARG] input_file output_file [topology_file]" << std::endl << std::endl;
                return false;
            }
        }
        if ( vm.count("verbose") )
        {
            const auto verbosity = vm["verbose"].as<int>();
            if ( verbosity == 0 )
            {
                Param.ShowLRprocess = Param.ReportRunTime = false;
            }
            else if ( verbosity == 1 )
            {
                Param.ShowLRprocess = true;
                Param.ReportRunTime = false;
            }
            else
            {
                Param.ShowLRprocess = Param.ReportRunTime = true;
            }
        }
        if ( vm.count("thread") )
        {
            Param.ThreadNum = std::min(MAXTHREAD, Param.ThreadNum);
        }
        if ( vm.count("release") )
        {
            Param.Release = true;
            Param.ThreadNum = 8;
            Param.ShowLRprocess = false;
            Param.ReportRunTime = false;
            Param.TDMrefinement = true;
        }
        else
        {
            Param.Release = false;
        }
        ShowConfig();
    }
    catch (boost::program_options::required_option& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        return false;
    }
    catch (boost::program_options::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        return false;
    }
    catch (std::exception& e)
    {
        std::cerr << "Unhandled Exception reached: " << e.what() << ", exiting" << std::endl;
        return false;
    }

    return true;
}

void ShowConfig()
{
    auto& Param = router::get_OptionMgr();
    cout << "------- Option Configuration -------" << endl;

    cout << "-- Input file           : " << Param.InputFileName << endl;
    cout << "-- Output file          : " << Param.OutputFileName << endl;
    cout << "-- Topology file        : ";
    if ( !Param.InputTopologyName.compare("None") ) cout << "[None]" << endl;
    else                                            cout << Param.InputTopologyName << endl;

    cout << "-- LR process display   : ";
    if ( Param.ShowLRprocess ) cout << "Enabled" << endl;
    else                       cout << "Disabled" << endl;

    cout << "-- Runtime report       : ";
    if ( Param.ReportRunTime ) cout << "Enabled" << endl;
    else                       cout << "Disabled" << endl;

    cout << "-- Rip Up & Reroute     : " << Param.RipUpCount << endl;

    cout << "-- TDM ratio assignment : ";
    if ( Param.TDMmethod == 1 ) cout << "LR" << endl;
    if ( Param.TDMmethod == 0 ) cout << "Heuristic" << endl;

    cout << "-- TDM ratio refinement : ";
    if ( Param.TDMrefinement ) cout << "Enabled" << endl;
    else                       cout << "Disabled" << endl;

    cout << "-- Epsilon for LR       : ";
    if ( !Param.TDMrefinement ) cout << "[None]" << endl;
    else                        cout << Param.Epsilon << endl;

    cout << "-- Number of threads    : " << Param.ThreadNum << endl;
    cout << "------------------------------------" << endl;
}
