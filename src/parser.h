#ifndef _PARSER_H_
#define _PARSER_H_
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
namespace router
{
template <class T>
void str2ints(const std::string& str, std::vector<T>& container, char delimiter = ' ') {
    container.resize(str.length()); // pre-allocate memory
    int j = 0;
    container[j] = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == delimiter) {
            ++j;
            container[j] = 0;
        }
        else {
            container[j] = container[j]*10 + (str[i] - 48);
        }
    }
    container.resize(j+1);
}
class parser
{
public:
    parser():
        _fpga_num(0),
        _fpga_edge_num(0),
        _net_num(0),
        _net_group_num(0)
    {}

    bool read            ( const std::string& fname );
    void read_fpga_edges ( std::ifstream& infile );
    void read_nets       ( std::ifstream& infile );
    void read_net_groups ( std::ifstream& infile );
    void init_vectors ();
    bool read_topology   ( const std::string& fname);
private:
    int _fpga_num;
    int _fpga_edge_num;
    int _net_num;
    int _net_group_num;
};
}
#endif
