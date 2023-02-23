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

typedef union DataUnion {
    char ch;
    int in;
    void* ptr;
} DataUnion;

typedef struct result {
    int data_type;
    DataUnion data;
} result;

typedef struct ResArrD {
    DataUnion* arr;
    // void** arr;
    int a_len;
    bool all_same_type;
    int all_type;
} ResArrD;

typedef struct state {
    bool is_error;
    char* error;
    int index;
    result* result;
    bool dealloc_old;
    bool error_from_malloc;
} state;

typedef void(*dallocresfunc_t)(result*);
typedef char*(*stringerfunc_t)(result*,bool);

typedef struct dealloc_str_data {
    int size;
    int* data_types;
    dallocresfunc_t* deallocers;
    stringerfunc_t* stringers;
} dealloc_str_data;

// -- parsers.h --
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

ResArrD* dcreate_res_arr(DataUnion* arr, int len, bool ast, int at);
ResArrD* create_res_arr(DataUnion* arr, int len);
result* dcreate_resarr_result(DataUnion* arr, int len, bool ast, int at);
result* create_resarr_result(DataUnion* arr, int len);

result* create_result(int data_type, DataUnion data);

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