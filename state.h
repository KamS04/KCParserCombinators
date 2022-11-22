#ifndef STATE_HEADER
#define STATE_HEADER
#include<stdbool.h>

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
    result** arr;
    int a_len;
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
    void(**handlers)(result*);
} deallocation_data;

deallocation_data* get_current_dealloc_data();

void set_global_dealloc_data(deallocation_data* d);

void deallocate_result(result* res);
void deallocate_state(state* st);

ResArrD* create_res_arr(result** arr, int len);
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

#endif