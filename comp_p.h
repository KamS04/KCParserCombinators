#ifndef COMPOSITION_PARSERS
#define COMPOSITION_PARSERS
#include "parsers.h"

parser* possibly(parser* p);
parser* everythingUntil(parser* p);
parser* anyCharExcept(parser* p);
parser* choice(parser** parsers, int p_size);
parser* sequenceOf(parser** parsers, int p_size);
parser* lookAhead(parser* p);
parser* sepBy(parser* get, parser* sep);
parser* many(parser* p);
parser* between(parser* before, parser* get, parser* after);

extern parser* optionalWhitespace;

void init_comp_parsers();

#endif