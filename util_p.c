#include "state.h"
#include "parsers.h"
#include<stdlib.h>
#include<string.h>

state* _fail(void* error, char* target, state* i_state) {
    return create_error_state((char*) error, i_state->index);
}

parser* fail(char* error) {
    return dcreate_parser(&_fail, error);
}

state* _eOI(void* data, char* target, state* i_state) {
    if (i_state->index >= strlen(target)) {
        result* res = create_result(INTEGER, 1);
        return create_result_state(res, i_state->index);
    } else {
        char* err = malloc( 17 * sizeof(char));
        strcpy(err, "Not end of input");
        return create_error_state(err, i_state->index);
    }
}
parser* endOfInput;

void init_core_util_parsers() {
    endOfInput = create_parser(_eOI);
}
