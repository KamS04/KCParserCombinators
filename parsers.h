#ifndef PARSERS_HEADER
#define PARSERS_HEADER ANYTHING
#include "state.h"
#include "korolib.h"
#include<stdbool.h>

typedef struct {
    int type;
    void* data;
} parser;
typedef struct {
    result* res;
    bool dealloc_old;
} mapresult;

typedef mapresult*(*mappertype)(result*,void*);

state* run(parser* p, char* c);

state* evaluate(parser* p, char* c, state* i_state);

parser* create_parser( state* (*parse)(void*,char*, state*) );
parser* dcreate_parser( state* (*parse)(void*,char*, state*), void* data );
parser* ddcreate_parser( state* (*parse)(void*,char*, state*), void* data, void(*dealloc_data)(void*), bool noc );

void deallocate_parser(parser* p);

parser* map( parser* in, mappertype mapper, bool noc, void* data);
parser* cmap(parser* in, mappertype mapper);

parser* chain( parser* in, parser*(*chooser)(state*), bool noc, bool dp);
parser* cchain(parser* in, parser*(*chooser)(state*));

parser* then( parser* in, parser* next, bool noc);
parser* cthen(parser* in, parser* next);

parser* manipulate( parser* in, state*(*manipulator)(state*), bool noc);
parser* cmanipulate(parser* in, state*(*manipulator)(state*));

parser* korop(void(*koro)(koroctx*), bool noc);

typedef struct {
    parser* original;
    result* (*mapper)(result*);
} map_parser;

#endif