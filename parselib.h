#ifndef KC_PARSE_LIB
#define KC_PARSE_LIB
#include "korolib.h"
#include<stdbool.h>

// Structs

// -- state.h --
enum DataType {
    STRING = 0,
    INTEGER = 1,
    CHAR = 2,
    RES_ARR = 3
};

typedef struct {
    int data_type;
    void* data;
} result;

typedef struct {
    void** arr;
    int a_len;
    bool all_same_type;
    int all_type;
} ResArrD;

typedef struct {
    bool is_error;
    char* error;
    int index;
    result* result;
    bool dealloc_old;
} state;

typedef struct {
    int size;
    int* data_types;
    void(**deallocers)(result*);
    char*(**stringers)(result*, bool);
} dealloc_str_data;

// -- parsers.h --
typedef struct {
    int type;
    void* data;
} parser;
typedef struct {
    result* res;
    bool dealloc_old;
} mapresult;

typedef mapresult*(*mappertype)(result*,void*);

typedef struct {
    parser* original;
    result* (*mapper)(result*);
} map_parser;

// Functions

// -- util.h --
int int_size(int c);

char* int_to_string(int i);

int pointer_size(void* c);

char* pointer_to_string(void* i);

int linear_search_prim(int f, int* arr, int size);

int linear_search(void* f, void** arr, int size, bool(*cmp)(void*,void*));

char toUpper(char c);
void lowerString(char* c, int len);

char toLower(char c);
void upperString(char* c, int len);

bool is_letter(char c);
bool is_digit(char c);
bool is_whitespace(char c);

int _find_last_match(int st, int st_len, char* str, bool(*cmp)(char c));

// -- state.h --
dealloc_str_data* get_current_dealloc_data();

void set_global_dealloc_data(dealloc_str_data* d);

void deallocate_result(result* res);
void deallocate_state(state* st);

ResArrD* dcreate_res_arr(void** arr, int len, bool ast, int at);
ResArrD* create_res_arr(result** arr, int len);
result* dcreate_resarr_result(void** arr, int len, bool ast, int at);
result* create_resarr_result(result** arr, int len);

result* create_result(int data_type, void* data);

state* default_state();

char* state_to_string(state* st);
char* result_to_string(result* rs);
char* dresult_to_string(result* rs, bool nl);

state* error_here(state* o_state, char* error);
state* update_error(state* o_state, char* error, int index);
state* result_here(state* o_state, result* n_res);

state* create_result_state(result* res, int index);
state* create_error_state(char* error, int index);

void deallocate_state(state* st);

// -- state_default_dealloc.h --
void def_dealloc_string(result* res); 

void def_dealloc(result* res);

void def_dealloc_resarr(result* res);

// -- parsers.h --
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

// -- string_p.h --
parser* strP(char* search);
parser* upperLowerStrP(char* search);

parser* mulCharMatchP( bool(*cmp), bool ato, char* err);

parser* regexP(char* pattern);

extern parser* letters;
extern parser* digits;
extern parser* whitespace;

void init_core_string_parsers();

// -- character_p.h --
extern parser* anyChar;
extern parser* letter;
extern parser* digit;
parser* charP(char c);

void init_core_char_parsers();

// -- comp_p.h --
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

// -- util_p.h --
parser* fail(char* error);
extern parser* endOfInput;

void init_core_util_parsers();

// -- core_p.h --
void init_core_parsers();

// -- kc_config.h --
extern int KC_PL_DEBUG_MODE;
extern int KC_PL_DEBUG_DAT_SIZE;
extern int KC_PL_HDEBUG_MODE;

#endif