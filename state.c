#include "state.h"
#include "util.h"
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<math.h>
#include "state_default_dealloc.h"

deallocation_data* global_state_deallocators = NULL;

deallocation_data* get_current_dealloc_data() {
    deallocation_data* d = malloc(sizeof(deallocation_data));

    // copy data_types array
    d->data_types = malloc(global_state_deallocators->size * sizeof(int));
    memcpy(d->data_types, global_state_deallocators->data_types, global_state_deallocators->size * sizeof(int));

    // copy data handlers array
    d->handlers = malloc(global_state_deallocators->size * sizeof(void*));
    memcpy(d->handlers, global_state_deallocators->handlers, global_state_deallocators->size * sizeof(void*));

    return d;
}

void set_global_dealloc_data(deallocation_data* d) {
    free(global_state_deallocators->data_types);
    free(global_state_deallocators->handlers);
    free(global_state_deallocators);
    global_state_deallocators = d;
}

void create_deafault_deallocators() {
    deallocation_data* tmp = malloc(sizeof(deallocation_data));
    tmp->size = 2;
    int tmp_data_types[] = { INTEGER, STRING, CHAR, RES_ARR };
    tmp->data_types = malloc(2 * sizeof(int));
    memcpy(tmp->data_types, tmp_data_types, 2 * sizeof(int));
    void* handlers[] = {
        &def_dealloc,
        &def_dealloc_string,
        &def_dealloc,
        &def_dealloc_resarr
    };
    tmp->handlers = malloc(2 * sizeof(void*));
    memcpy(tmp->handlers, handlers, 2 * sizeof(void*));
    global_state_deallocators = tmp;
}

void deallocate_result(result* res) {
    int to_f = res->data_type;
    int idx = linear_search_prim(to_f, global_state_deallocators->data_types, global_state_deallocators->size);
    if (idx == -1) {
        printf("You fucking dipshit, %i is not a valid data_type\n", to_f);
        puts("Specify your fucking data types and deallocators before using the Parserlib dumbass.");
        exit(3);
    }

    void (*dealloc_handler)(result*) = global_state_deallocators->handlers[idx];
    dealloc_handler(res);
}

void deallocate_state(state* st) {
    if (global_state_deallocators == NULL) {
        create_deafault_deallocators();
    }
    if (st->result != NULL) {
        deallocate_result(st->result);
    }
    if (st->error != NULL) {
        free(st->error);
    }

    free(st);
}

char* state_to_string(state* n_state) {
    int error_size = 0, res_add_size = 0, index_size = 0;
    int string_size = 9; // isError: 

    if (n_state->is_error) {
        string_size += 5; // true\n
        string_size += 7; // error: 
        error_size += strlen(n_state->error);
        string_size += error_size + 1; // %error%\n
    } else {
        string_size += 6; // false\n
    }

    if (n_state->result != NULL) {
        string_size += 9; // result: @
        res_add_size = pointer_size(n_state->result);
        string_size += res_add_size + 1; // %address%\n
    }

    string_size += 7; // "index: ";
    index_size = int_size(n_state->index);
    string_size += index_size + 2; // %index%\n\x00

    int curr_pos = 0;

    char* out = malloc(string_size * sizeof(char));

    strcpy(out, "isError: ");

    strcpy(out + 9, "true\nerror");

    if (n_state->is_error) {
        strcpy(out+9, "true\nerror: ");
        memcpy(out+21, n_state->error, error_size);
        out[21+error_size] = '\n';
        curr_pos = 22 + error_size;
    } else {
        strcpy(out+9, "false\n");
        curr_pos = 15;
    }

    if (n_state->result != NULL) {
        strcpy(out+curr_pos, "result: @");
        sprintf(out+curr_pos+9, "%p", n_state->result);
        curr_pos += 9 + res_add_size;
        out[curr_pos] = '\n';
        curr_pos += 1;
    }

    strcpy(out+curr_pos, "index: ");

    sprintf(out+curr_pos+7, "%i", n_state->index);
    strcpy(out+curr_pos+7+index_size, "\n\0");
    return out;
}

char* result_to_string(result* rs) {
    return dresult_to_string(rs, true);
}
char* dresult_to_string(result* rs, bool nl) {
    int dtype = rs->data_type;
    int len_dtype = int_size(dtype);
    int t_size = len_dtype + 10; // DataType: %dtype%
    // if (nl) {
    //     t_size += 1; // \n
    // }

    t_size += 7;
    int v_size = 0;
    ResArrD* rad = NULL;
    char** _rad_ar = NULL;
    puts("in drts");
    switch(dtype) {
        case INTEGER:
            puts("int match");
            v_size = int_size((int) rs->data);
            break;
        case STRING:
            puts("int string");
            if (rs->data != NULL) {
                v_size = strlen(rs->data);
            } else {
                v_size += 4;
            }
            break;
        case CHAR:
            puts("int char");
            v_size = 1;
            break;
        case RES_ARR:
            puts("int res_arr");
            rad = rs->data;
            v_size = 2 + (rad->a_len * 2); // Commas, Spaces, Brackets
            _rad_ar = malloc(rad->a_len * sizeof(char*) );
            for (int i = 0; i < rad->a_len; i++) {
                printf("rstring #%d\n", i);
                _rad_ar[i] = dresult_to_string(rad->arr[i], false);
                printf("rstred $%d\n", i);
                v_size += strlen(_rad_ar[i]);
            }
            puts("finish array rstr");
    }
    int e_size = 3;
    // if (nl) {
    //     e_size = 2; // \n\n\0
    // } else {
    //     e_size = 3;// , \0
    // }

    char* str = malloc( (t_size + v_size + e_size)* sizeof(char) );
    char* s = rs->data;
    char _nl[] = "NULL";
    char* format;
    char _fn1[] = "DataType: %d\nValue: %s\n";
    char _fn2[] = "DataType: %d, Value: %s";
    int fi = 21;
    if (nl) {
        format = _fn1;
    } else {
        format = _fn2;
        fi = 22;
    }

    switch(dtype) {
        case STRING:
            if (rs->data == NULL) {
                s = _nl;
            }
            break;
        case INTEGER:
            format[fi] = 'd';
            break;
        case CHAR:
            format[fi] = 'c';
            break;
        case RES_ARR:
            s = malloc((v_size+1) * sizeof(char));
            int off = 0;
            puts("creating full s str");
            for (int i = 0; i < rad->a_len; i++) {
                int slen = strlen(_rad_ar[i]);
                memcpy(s+off, _rad_ar[i], slen);
                printf("#1 %s\n", _rad_ar[i]);
                off += slen;
                if (i + 1 != rad->a_len) {
                    memcpy(s+off+slen, ", ", 2);
                    off += 2;
                }
                free(_rad_ar[i]);
            }
            puts("finish s str");
            s[off] = '\0';
            printf("wowo: %s\n", s);
            free(_rad_ar);
            printf("%s\n", s);
    }
    sprintf(str, format, dtype, s);
    return str;
}

ResArrD* create_res_arr(result** arr, int len) {
    ResArrD* raD = malloc(sizeof(ResArrD));
    raD->a_len = len;
    raD->arr = arr;
    return raD;
}

result* create_resarr_result(result** arr, int len) {
    return create_result(RES_ARR,create_res_arr(arr, len));
}

result* create_result(int data_type, void* data) {
    result* n_result = malloc(sizeof(result));
    n_result->data_type = data_type;
    n_result->data = data;
    return n_result;
}

state* copy_state(state* o_state) {
    state* n_state = malloc(sizeof(state));
    memcpy(n_state, o_state, sizeof(state));
    return n_state;
}

state* default_state() {
    state* n_state = malloc(sizeof(state));
    n_state->error = NULL;
    n_state->is_error = false;
    n_state->index = 0;
    n_state->result = NULL;
    n_state->dealloc_old = true;
    return n_state;
}

state* error_here(state* o_state, char* error) {
    state* n_state = copy_state(o_state);
    n_state->is_error = true;
    n_state->error = error;
    return n_state;
}

state* update_error(state* o_state, char* error, int index) {
    state* n_state = copy_state(o_state);
    n_state->is_error = true;
    n_state->error = error;
    n_state->index = index;
    return n_state;
}

state* result_here(state* o_state, result* n_res) {
    state* n_state = copy_state(o_state);
    n_state->result = n_res;
    n_state->is_error = false;
    n_state->error = NULL;
    return n_state;
}

state* create_result_state(result* res, int index) {
    state* n_state = default_state();
    n_state->result = res;
    n_state->index = index;
    return n_state;
}

state* create_error_state(char* error, int index) {
    state* n_state = default_state();
    n_state->is_error = true;
    n_state->error = error;
    n_state->index = index;
    return n_state;
}
