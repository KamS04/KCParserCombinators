#include<math.h>
#include<string.h>
#include<stdio.h>
#include "state.h"
#include "parsers.h"
#include "util.h"
#include "kre.h"
#include "log.h"

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
    return create_error_state(error_st, i_state->index);
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
        state* s = error_here(i_state, "Target is not long enough to check");
        s->error_from_malloc = false;
        return s;
    }
    
    char* n_str = malloc(ss->len * sizeof(char));
    memcpy(n_str, target + i_state->index, ss->len-1);
    lowerString(n_str, ss->len-1);
    n_str[ss->len-1] = '\0';

    if (strncmp(ss->search, n_str, ss->len-1) == 0) {
        // SUCCESS
        kfree(n_str);
        char* ms = malloc(ss->len * sizeof(char));
        memcpy(ms, ss->search, ss->len);
        result* res = create_result(STRING, ms);
        return create_result_state(res, i_state->index + ss->len - 1);
    }

    kfree(n_str);
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
        LOG(puts("something matched"));
        if (matched > 0) {
            LOG(printf("%d matched chars\n", matched));
            res->data = malloc( (matched+1) * sizeof(char) );
            LOG(puts("finished malloc"));
            memcpy(res->data, target + i_state->index, matched);
            LOG(puts("finished data copy"));
            ((char*) res->data)[matched] = '\0';
            LOG(puts("finished term put"));
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
    mcs->errlen = snprintf(NULL, 0, err, 'c') + 1;
    mcs->errstr = malloc((strlen(err) + 1) * sizeof(char));
    strcpy(mcs->errstr, err);
    return dcreate_parser(&_mulCharMP, mcs);
}

#ifndef OLD_REGEX_LIB

typedef struct {
    int p_len;
    char* pat;
    re_t regex;
} kregit;
state* _regP(void* data, char* target, state* i_state) {
    kregit* krit = (kregit*)data;

    int m_len;
    int match_start = re_matchp(krit->regex, &target[i_state->index], &m_len);
    LOG(printf("match info %d %d\n", match_start, m_len));
    
    if (match_start == -1 || m_len == 0) {
        char* err = malloc( (krit->p_len + 27) * sizeof(char) );
        sprintf(err, "Could not match pattern: %s\n", krit->pat);
        return create_error_state(err, i_state->index);
    }
    
    char* mat = malloc( (m_len + 1) * sizeof(char) );
    LOG(printf("mat loc %p\n", mat));
    memcpy(mat, target + i_state->index + match_start, m_len * sizeof(char));
    mat[m_len] = '\0';
    result* res = create_result(STRING, mat);
    return create_result_state(res, i_state->index + match_start + m_len);
}

parser* regexP(char* pattern) {
    kregit* it = malloc(sizeof(kregit));
    it->p_len = strlen(pattern);
    it->pat = malloc( (it->p_len + 1) * sizeof(char) );
    it->regex = re_compile(pattern);
    strcpy(it->pat, pattern);
    return dcreate_parser(&_regP, it);
}

#else

typedef struct {
    int p_len;
    char* pat;
} regit;
state* _regP(void* data, char* target, state* i_state) {
    regit* rit = (regit*) data;

    static char* l_p_ptr;
    static re_t l_c_ptr;
    if (rit->pat != l_p_ptr) {
        l_c_ptr = re_compile(rit->pat);
        l_p_ptr = rit->pat;
    }

    int m_len;
    int match_idx = re_matchp(l_c_ptr, target + i_state->index, &m_len);
    LOG(printf("match info %d %d\n", m_len, match_idx));
    if (match_idx != 0 || m_len == 0) {
        char* err = malloc( (rit->p_len + 26) * sizeof(char) );
        sprintf(err, "Could not match pattern: %s", rit->pat);
        return create_error_state(err, i_state->index);
    }
    char* mat = malloc((m_len+1) * sizeof(char));
    LOG(printf("mat loc %p\n", mat));
    memcpy(mat, target + i_state->index, m_len * sizeof(char));
    mat[m_len] = '\0';
    result* res = create_result(STRING, mat);
    return create_result_state(res, i_state->index + m_len);
}
parser* regexP(char* pattern) {
    regit* it = malloc(sizeof(regit));
    it->p_len = strlen(pattern);
    it->pat = malloc( (it->p_len + 1) * sizeof(char));
    strcpy(it->pat, pattern);
    return dcreate_parser(&_regP, it);
}

#endif

parser* letters;
parser* digits;
parser* whitespace;

void init_core_string_parsers() {
    letters = mulCharMatchP(&is_letter, true, "Letters not found");
    digits = mulCharMatchP(&is_digit, true, "Digits not found");
    whitespace = mulCharMatchP(&is_whitespace, true, "Found '%c' instead of a whitespace");
}
