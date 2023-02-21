#ifndef CORE_CHAR_PARSERS
#define CORE_CHAR_PARSERS

#include "state.h"
#include "parsers.h"

extern parser* anyChar;
extern parser* letter;
extern parser* digit;
parser* charP(char c);

void init_core_char_parsers();

#endif