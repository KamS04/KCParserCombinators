#include "state.h"
#include "util.h"
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<math.h>
#include "state_default_dealloc.h"
#include "log.h"

dealloc_str_data* global_state_deallocators = NULL;
bool defaults_changed = false;

void create_deafault_deallocators() {
    dealloc_str_data* tmp = malloc(sizeof(dealloc_str_data));
    int pdeallocs = 4;
    tmp->size = pdeallocs;
    int tmp_data_types[] = { STRING, INTEGER, CHAR, RES_ARR };
    tmp->data_types = malloc(pdeallocs * sizeof(int));
    memcpy(tmp->data_types, tmp_data_types, pdeallocs * sizeof(int));
    void* deallocers[] = {
        &def_dealloc_string,
        &def_dealloc,
        &def_dealloc,
        &def_dealloc_resarr
    };
    tmp->deallocers = malloc(pdeallocs * sizeof(void*));
    memcpy(tmp->deallocers, deallocers, pdeallocs * sizeof(void*));
    void* stringers[] = {
        NULL,
        NULL,
        NULL,
        NULL
    };
    tmp->stringers = malloc(pdeallocs * sizeof(void*));
    memcpy(tmp->stringers, stringers, pdeallocs * sizeof(void*));
    global_state_deallocators = tmp;
}

dealloc_str_data* prepare_global_typedata() {
    if (global_state_deallocators == NULL) {
        create_deafault_deallocators();
    }
    return global_state_deallocators;
}

dealloc_str_data* get_current_dealloc_data() {
    prepare_global_typedata();

    dealloc_str_data* d = malloc(sizeof(dealloc_str_data));
    d->size = global_state_deallocators->size;

    // copy data_types array
    d->data_types = malloc(global_state_deallocators->size * sizeof(int));
    memcpy(d->data_types, global_state_deallocators->data_types, global_state_deallocators->size * sizeof(int));

    // copy data deallocers array
    d->deallocers = malloc(global_state_deallocators->size * sizeof(void*));
    memcpy(d->deallocers, global_state_deallocators->deallocers, global_state_deallocators->size * sizeof(void*));

    // copy data stringers
    d->stringers = malloc(global_state_deallocators->size * sizeof(void*));
    memcpy(d->stringers, global_state_deallocators->stringers, global_state_deallocators->size * sizeof(void*));

    return d;
}

bool check_default_change(int type) {
    prepare_global_typedata();

    int idx = linear_search_prim(type, global_state_deallocators->data_types, global_state_deallocators->size);
    if (idx == -1) {
        puts("New Deallocers+Stringers MUST specify deallocers+stringers for default data types!");
        puts("Deallocers MUST point to actual functions, while default stringers MAY be null");
        printf("Datatype %d was not defined\n", type);
    }

    if (global_state_deallocators->stringers[idx] != NULL) {
        return true;
    }
    return false;
}

void set_global_dealloc_data(dealloc_str_data* d) {
    kfree(global_state_deallocators->data_types);
    kfree(global_state_deallocators->deallocers);
    kfree(global_state_deallocators);
    global_state_deallocators = d;

    defaults_changed = check_default_change(STRING) ||
                        check_default_change(CHAR) ||
                        check_default_change(INTEGER) ||
                        check_default_change(RES_ARR);
}

void deallocate_result(result* res) {
    prepare_global_typedata();
    int to_f = res->data_type;
    int idx = linear_search_prim(to_f, global_state_deallocators->data_types, global_state_deallocators->size);
    if (idx == -1) {
        printf("You fucking dipshit, %i is not a valid data_type\n", to_f);
        puts("Specify your fucking data types and deallocators before using the Parserlib dumbass.");
        exit(3);
    }

    void (*dealloc_handler)(result*) = global_state_deallocators->deallocers[idx];
    dealloc_handler(res);
}

void deallocate_state(state* st) {
    prepare_global_typedata();

    if (st->result != NULL) {
        deallocate_result(st->result);
    }
    if (st->error != NULL && st->error_from_malloc) {
        kfree(st->error);
    }

    kfree(st);
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
    // BASIC SWITCH
    prepare_global_typedata();
    
    if (defaults_changed) {
        int idx = linear_search_prim(rs->data_type, global_state_deallocators->data_types, global_state_deallocators->size);
        if (idx == -1) {
            printf("WTF stringer for %d was not defined!!!", rs->data_type);
        }
        return global_state_deallocators->stringers[idx](rs, nl);
    }
    
    int h_size = 17; // DataType: Value: 
    h_size += int_size(rs->data_type);

    // STUFF IN CASE OF ARRAY
    char** _rarr;
    ResArrD* _rad;
    // END STUFF FOR ARRAY

    int v_size;

    switch (rs->data_type) {
        case INTEGER:
            v_size = int_size((int) rs->data);
            break;
        case STRING:
            v_size = strlen(rs->data);
            break;
        case CHAR:
            v_size = 1;
            break;
        case RES_ARR:
            v_size = 4; // {  }
            _rad = (ResArrD*) rs->data;
            _rarr = malloc( _rad->a_len * sizeof(char*) );
            if (_rad->all_same_type) {
                result* r = create_result(_rad->all_type, NULL);
                for (int i = 0; i < _rad->a_len; i++) {
                    r->data = _rad->arr[i];
                    _rarr[i] = dresult_to_string(r, false);
                    v_size += strlen(_rarr[i]);
                    if (i + 1 != _rad->a_len) {
                        v_size += 2; // ", "
                    }
                }
            } else {
                for (int i = 0; i < _rad->a_len; i++) {
                    _rarr[i] = dresult_to_string(_rad->arr[i], false);
                    v_size += strlen(_rarr[i]);
                    if (i + 1 != _rad->a_len) {
                        v_size += 2; // ", "
                    }
                }
            }

            
            break;
        default: {
            int idx = linear_search_prim(rs->data_type, global_state_deallocators->data_types, global_state_deallocators->size);
            if (idx == -1) {
                printf("WTF stringer for %d was not defined!!!", rs->data_type);
            }
            return global_state_deallocators->stringers[idx](rs, nl);
        }
    }

    int e_size = 3; // "\n\n\0" or ", \0"

    // Writing type;
    char* fstr = malloc( (h_size + v_size + e_size) * sizeof(char));

    char* format;
    char f_1[] = "DataType: %d\nValue: %s\n";
    char f_2[] = "DataType: %d, Value: %s";\
    int dpos;
    if (nl) {
        format = f_1;
        dpos = 21;
    } else {
        format = f_2;
        dpos = 22;
    }

    char* s = rs->data;
    switch (rs->data_type) {
        case INTEGER:
            format[dpos] = 'd';
            break;
        case STRING:
            break;
        case CHAR:
            format[dpos] = 'c';
            break;
        case RES_ARR:
            // WRITE DATA STRING TO s
            s = malloc( (v_size + 1) * sizeof(char) );
            s[0] = '{'; s[1] = ' ';
            int off = 2;
            for (int i = 0; i < _rad->a_len; i++) {
                int l = strlen(_rarr[i]);
                if (i + 1 != _rad->a_len) {
                    sprintf(s + off, "%s; ", _rarr[i]);
                    off += l + 2;
                } else {
                    memcpy(s + off, _rarr[i], l);
                    off += l;
                }
                kfree(_rarr[i]);
            }
            kfree(_rarr);
            strcpy(s+off, " }");
    }
    sprintf(fstr, format, rs->data_type, s);
    if (s != rs->data) {
        kfree(s);
    }

    return fstr;
}

ResArrD* dcreate_res_arr(void** arr, int len, bool ast, int at) {
    ResArrD* rad = malloc(sizeof(ResArrD));
    rad->a_len = len;
    rad->arr = arr;
    rad->all_same_type = ast;
    rad->all_type = at;
    return rad;
}
ResArrD* create_res_arr(result** arr, int len) {
    return dcreate_res_arr(arr, len, 0, 0);
}

result* dcreate_resarr_result(void** arr, int len, bool ast, int at) {
    return create_result(RES_ARR, dcreate_res_arr(arr, len, ast, at) );
}

result* create_resarr_result(result** arr, int len) {
    return dcreate_resarr_result(arr, len, 0, 0);
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
    n_state->error_from_malloc = true;
    return n_state;
}

state* error_here(state* o_state, char* error) {
    state* n_state = copy_state(o_state);
    n_state->is_error = true;
    n_state->error = error;
    n_state->result = NULL;
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
