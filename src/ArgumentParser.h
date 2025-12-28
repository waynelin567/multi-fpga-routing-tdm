#ifndef __ARG_PARSER__
#define __ARG_PARSER__

#include "def.h"

#pragma GCC system_header
#include <boost/program_options.hpp>

bool ParseArguments(int, char**);
void ShowConfig();

#endif /* __ARG_PARSER__ */
