#ifndef _FILE_WRITER_H
#define _FILE_WRITER_H
#include <iostream>
#include <string>
#include <sstream>
#include "def.h"
static char buf[64];
static int count = 0;
namespace router
{

class file_writer
{
public:
    void output_result (const std::string& fname);
    inline void append_ints(std::string& out_str, TdmType data) {
        if (!data) buf[count++] = '0';
        else {
            while (data > 0) {
                buf[count++] = (char)(data % 10 + '0');
                data /= 10;
            }
        }
        while (count > 0) out_str.push_back(buf[--count]);
    }
public:
    static file_writer & get_instance()
    {
        static file_writer w;
        return w;
    }
};

inline file_writer& getWriter() {return file_writer::get_instance();}
}
#endif 
