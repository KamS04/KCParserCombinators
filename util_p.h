#ifndef CORE_UTIL_PARSERS
#define CORE_UTIL_PARSERS

#include "state.h"
#include "parsers.h"

parser* fail(char* error);
extern parser* endOfInput;

void init_core_util_parsers();

#endif