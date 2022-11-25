#ifndef CORE_STRING_PARSERS
#define CORE_STRING_PARSERS
#include<stdbool.h>
#include "state.h"
#include "parsers.h"

parser* strP(char* search);
parser* upperLowerStrP(char* search);

parser* mulCharMatchP( bool(*cmp), bool ato, char* err);

parser* regexP(char* pattern);

extern parser* letters;
extern parser* digits;
extern parser* whitespace;

void init_core_string_parsers();

#endif