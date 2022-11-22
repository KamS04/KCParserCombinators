#include<stdbool.h>
#include<string.h>
#include "state.h"
#include "parsers.h"
#include "util.h"

result* create_char_result(char c) {
    return create_result(CHAR, (void*) c);
}

state* _anyChar(void* _, char* target, state* i_state) {
    if (strlen(target) <= i_state->index) {
        char* err = malloc(32 * sizeof(char));
        return create_error_state(err, i_state->index);
    }
    result* res = create_char_result(target[i_state->index]);
    return create_result_state(res, i_state->index + 1);
}
parser* anyChar;

typedef struct {
    char find;
    char* cnf;
} _csit;
state* _charP(void* data, char* target, state* i_state) {
    _csit* cs = (_csit*) data;
    if (target[i_state->index] == cs->find) {
        // SUCCCESS
        result* res = create_char_result(cs->find);
        return create_result_state(res, i_state->index + 1);
    }

    // FAIL
    if (cs->cnf == NULL) {
        cs->cnf = malloc(sizeof(char)*20);
        sprintf(cs->cnf, "Char '%c' not found", cs->find);
    }
    char* error_msg = malloc(sizeof(char)*20);
    memcpy(error_msg, cs->cnf, 20);
    return create_error_state(error_msg, i_state->index);
}
parser* charP(char c) {
    _csit* cs = malloc(sizeof(_csit));
    cs->find = c;
    cs->cnf = NULL;
    return dcreate_parser(&_charP, cs);
}

state* _letP(void* _, char* target, state* i_state) {
    char sc = target[i_state->index];
    if (is_letter(sc)) {
        result* res = create_char_result(target[i_state->index]);
        return create_result_state(res, i_state->index+1);
    }
    char* er = malloc(30*sizeof(char));
    sprintf(er, "Found '%c' instead of a letter", sc);
    return create_error_state(er, i_state->index);
}
parser* letter;

state* _digP(void* _, char* target, state* i_state) {
    char l = target[i_state->index];
    if (is_digit(l)) {
        result* res = create_char_result(l);
        return create_result_state(res, i_state->index + 1);
    }
    char* err = malloc( 29 * sizeof(char));
    sprintf(err, "Found '%c' instead of a digit", l);
    return create_error_state(err, i_state->index);
}
parser* digit;

void init_core_char_parsers() {
    anyChar = create_parser(&_anyChar);
    letter = create_parser(&_letP);
    digit = create_parser(&_digP);
}
