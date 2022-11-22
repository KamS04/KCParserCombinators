#include<math.h>
#include<string.h>
#include<stdio.h>
#include "state.h"
#include "parsers.h"
#include "util.h"

typedef struct {
    char* search;
    char* snf;
    int len; // Includes \0
} _ssit;
state* _strP(void* data, char* target, state* i_state) {
    _ssit* ss = (_ssit*) data;
    if (strncmp(ss->search, target + i_state->index, ss->len-1) == 0) {
        // SUCCESS
        char* ms = malloc(ss->len * sizeof(char));
        memcpy(ms, ss->search, ss->len * sizeof(char));
        result* res = create_result(STRING, ms);
        return create_result_state(res, i_state->index + ss->len - 1);
    }
    if (ss->snf == NULL) {
        ss->snf = malloc( (ss->len + 9) * sizeof(char));
        sprintf(ss->snf, "String \"%s\"", ss->search);
    }
    char* error_st = malloc( (ss->len + 9) * sizeof(char) );
    memcpy(error_st, ss->snf, ss->len + 9);
    return error_here(i_state, error_st);
}
parser* strP(char* search) {
    _ssit* ss = malloc(sizeof(_ssit));
    ss->search = search;
    ss->snf = NULL;
    ss->len = strlen(search) + 1;
    return dcreate_parser(&_strP, ss);
}

state* _lUstrP(void* data, char* target, state* i_state) {
    _ssit* ss = (_ssit*) data;
    int t_len = strlen(target);
    if (ss->len-1 > t_len - i_state->index) {
        return error_here(i_state, "Target is not long enough to check");
    }
    
    char* n_str = malloc(ss->len * sizeof(char));
    memcpy(n_str, target + i_state->index, ss->len-1);
    lowerString(n_str, ss->len-1);
    n_str[ss->len-1] = '\0';

    if (strncmp(ss->search, n_str, ss->len-1) == 0) {
        // SUCCESS
        free(n_str);
        char* ms = malloc(ss->len * sizeof(char));
        memcpy(ms, ss->search, ss->len);
        result* res = create_result(STRING, ms);
        return create_result_state(res, i_state->index + ss->len - 1);
    }

    free(n_str);
    if (ss->snf == NULL) {
        ss->snf = malloc( (ss->len + 20) * sizeof(char));
        sprintf(ss->snf, "UpperLower String \"%s\"", ss->search);
    }
    char* error_st = malloc( (ss->len + 20) * sizeof(char) );
    memcpy(error_st, ss->snf, ss->len + 20);
    return error_here(i_state, error_st);
}
parser* upperLowerStrP(char* search) {
    _ssit* ss = malloc(sizeof(_ssit));
    ss->len = strlen(search) + 1;
    ss->search = malloc( (ss->len)*sizeof(char) );
    ss->snf = NULL;
    memcpy(ss->search, search, ss->len);
    lowerString(ss->search, ss->len-1);
    return dcreate_parser(&_lUstrP, ss);
}

typedef struct {
    bool(*cmp)(char);
    bool ato;
    int errlen;
    char* errstr;
} _mcsit;
state* _mulCharMP(void* data, char* target, state* i_state) {
    _mcsit* mcs = (_mcsit*) data;
    int en = _find_last_match(i_state->index, strlen(target), target, mcs->cmp);
    int matched = en - i_state->index + 1;
    if (matched > 0 || !mcs->ato) {
        result* res = create_result(STRING, NULL);
        // puts("something matched");
        if (matched > 0) {
            // printf("%d matched chars\n", matched);
            res->data = malloc( (matched+1) * sizeof(char) );
            // puts("finished malloc");
            memcpy(res->data, target + i_state->index, matched);
            // puts("finished data copy");
            ((char*) res->data)[matched] = '\0';
            // puts("finished term put");
        }
        return create_result_state(res, i_state->index + matched);
    } else {
        char* err = malloc( mcs->errlen * sizeof(char) );
        sprintf(err, mcs->errstr, target[i_state->index]);
        return create_error_state(err, i_state->index);
    }
}
parser* mulCharMatchP(bool(*cmp)(char), bool ato, char* err) {
    _mcsit* mcs = malloc(sizeof(_mcsit));
    mcs->ato = ato;
    mcs->cmp = cmp;
    mcs->errlen = snprintf(NULL, 0, err, 'c');
    mcs->errstr = malloc(strlen(err) * sizeof(char));
    strcpy(mcs->errstr, err);
    return dcreate_parser(&_mulCharMP, mcs);
}

parser* letters;
parser* digits;
parser* whitespace;

void init_core_string_parsers() {
    letters = mulCharMatchP(&is_letter, true, "Letters not found");
    digits = mulCharMatchP(&is_digit, true, "Digits not found");
    whitespace = mulCharMatchP(&is_whitespace, true, "Found '%c' instead of a whitespace");
}
