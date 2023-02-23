#ifndef COMPOSITION_PARSERS
#define COMPOSITION_PARSERS
#include "parsers.h"

parser* possibly(parser* p);
parser* everythingUntil(parser* p, bool check_till_end);
parser* anyCharExcept(parser* p);
parser* choice(parser** parsers, int p_size);
parser* sequenceOf(parser** parsers, int p_size);
parser* lookAhead(parser* p);
parser* sepBy(parser* get, parser* sep);
parser* between(parser* before, parser* get, parser* after);
parser* dmany(parser* p, bool atleast1, bool all_same_type, int all_type);
parser* many(parser* p);
parser* many1(parser* p);
parser* manyAS(parser* p, int atype);
parser* manyAS1(parser* p, int atype);

extern parser* optionalWhitespace;

void init_comp_parsers();

#endif