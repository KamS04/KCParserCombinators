#ifndef PARSERS_HEADER
#define PARSERS_HEADER ANYTHING
#include "state.h"
#include "korolib.h"
#include<stdbool.h>

typedef struct parser {
    int type;
    void* data;
} parser;
typedef struct mapresult {
    result* res;
    bool dealloc_old;
} mapresult;

typedef state*(*parserfunc_t)(DataUnion,char*,state*);
typedef void(*ddallocfunc_t)(DataUnion);
typedef parser*(*chooserfunc_t)(state*);
typedef state*(*manipfunc_t)(state*);
typedef mapresult*(*mappertype)(result*,DataUnion);

state* run(parser* p, char* c);

state* evaluate(parser* p, char* c, state* i_state);

parser* create_parser( parserfunc_t parse );
parser* dcreate_parser( parserfunc_t parse, DataUnion data );
parser* ddcreate_parser( parserfunc_t parser, DataUnion data, ddallocfunc_t dealloc_data, bool noc);

void deallocate_parser(parser* p);

parser* map( parser* in, mappertype mapper, bool noc, DataUnion data);
parser* cmap(parser* in, mappertype mapper);

parser* chain( parser* in, chooserfunc_t chooser, bool noc, bool dp);
parser* cchain(parser* in, chooserfunc_t chooser);
parser* ndchain(parser* in, chooserfunc_t chooser);

parser* then( parser* in, parser* next, bool noc);
parser* cthen(parser* in, parser* next);

parser* manipulate( parser* in, manipfunc_t manipulator, bool noc);
parser* cmanipulate(parser* in, manipfunc_t manipulator);

parser* korop(koroutinefunc_t koro, bool noc);

#endif