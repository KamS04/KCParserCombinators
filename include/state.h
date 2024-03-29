#ifndef STATE_HEADER
#define STATE_HEADER
#include<stdbool.h>

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

#endif