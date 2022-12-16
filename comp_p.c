#include<string.h>
#include<stdio.h>
#include "parsers.h"
#include "state.h"
#include "string_p.h"
#include "util_p.h"
#include "mutarr.h"
#include "log.h"

state* _posP(void* data, char* target, state* i_state) {
    parser* orig = (parser*) data;
    state* first = evaluate(orig, target, i_state);
    if (first->is_error) {
        state* final = result_here(first, NULL);
        free(first);
        return final;
    }
    return first;
}
parser* possibly(parser* p) {
    return ddcreate_parser(_posP, p, NULL, true);
}

state* _eUP(void* data, char* target, state* i_state) {
    parser* p = (parser*) data;
    int st = i_state->index;
    int tlen = strlen(target);
    
    state* c_state = default_state();
    c_state->index = i_state->index;
    while (c_state->index < tlen) {
        state* n_state = evaluate(p, target, c_state);
        if (!n_state->is_error) {
            break;
        }
        c_state->index += 1;
        deallocate_state(n_state);
    }

    int en = c_state->index;
    if (en >= tlen) {
        char* err = malloc(33 * sizeof(char));
        return create_error_state(err, i_state->index);
    }
    char* sres = malloc((en-st+1) * sizeof(char));
    memcpy(sres, target+st, (en-st));
    sres[en-st] = '\0';
    result* res = create_result(STRING, sres);
    return create_result_state(res, en);
}
parser* everythingUntil(parser* p) {
    return dcreate_parser(_eUP, p);
}

state* _aCE(void* data, char* target, state* i_state) {
    parser* p = (parser*) data;
    state* c_state = evaluate(p, target, i_state);
    if (c_state->is_error) {
        result* res = create_result(CHAR, (void*) target[i_state->index]);
        return create_result_state(res, i_state->index + 1);
    }
    char* err = malloc(34 * sizeof(char));
    strcpy(err, "anyCharExcept : Matched exception");
    return create_error_state(err, i_state->index);
}
parser* anyCharExcept(parser* p) {
    return dcreate_parser(_aCE, p);
}

typedef struct {
    int p_size;
    parser** parsers;
} _cit;
state* _cBP(void* data, char* target, state* i_state) {
    _cit* it = (_cit*) data;
    parser** ps = it->parsers;
    state* s_state = NULL;
    for (int i = 0; i < it->p_size; i++) {
        parser* cp = ps[i];
        s_state = evaluate(cp, target, i_state);
        if (!s_state->is_error) {
            break;
        }
        deallocate_state(s_state);
        s_state = NULL;
    }
    if (s_state != NULL) {
        return s_state;
    }
    char* err = malloc( 28 * sizeof(char));
    strcpy(err, "Choice - No parsers matched");
    return create_error_state(err, i_state->index);
}
parser* choice(parser** parsers, int p_size) {
    if (p_size <= 0) {
        puts("You fucking dumbass, we can't choose between <= 0 parsers.");
        puts("Give me actual parsers u dumb bitch");
        exit(3);
    }
    _cit* it = malloc(sizeof(_cit));
    it->p_size = p_size;
    it->parsers = parsers;
    return dcreate_parser(_cBP, it);
}

// typedef struct {
//     int
// } _sopit;
state* _sOP(void* data, char* target, state* i_state) {
    _cit* it = (_cit*) data;
    parser** ps = it->parsers;
    state* s_state = i_state;
    result** resarr = malloc( it->p_size * sizeof(result*) );
    for (int i = 0; i < it->p_size; i++) {
        resarr[i] = NULL;
    }
    bool sequenceBreak = false;
    for (int i = 0; i < it->p_size; i++) {
        parser* cp = ps[i];
        s_state = evaluate(cp, target, s_state);
        if (s_state->is_error) {
            sequenceBreak = true;
            break;
        }
        resarr[i] = s_state->result;
    }
    if (sequenceBreak) {
        for (int i = 0; i < it->p_size; i++) {
            if (resarr[i] != NULL) {
                deallocate_result(resarr[i]);
            }
        }
        free(resarr);
        int s_len = strlen(s_state->error);
        char* err = malloc( (s_len + 18) * sizeof(char) );
        sprintf(err, "Sequence Break : %s", s_state->error);
        free(s_state->error);
        s_state->error = err;
        return s_state;
    }

    result* res = create_resarr_result(resarr, it->p_size);
    return create_result_state(res, s_state->index);
}
parser* sequenceOf(parser** parsers, int p_size) {
    _cit* it = malloc(sizeof(_cit));
    it->parsers = malloc( p_size * sizeof(parser*) );
    it->p_size = p_size;
    for (int i = 0; i < p_size; i++) {
        it->parsers[i] = parsers[i];
    }
    return dcreate_parser(_sOP, it);
}


state* _loA(void * data, char* target, state* i_state) {
    state* t_state = evaluate( (parser*) data, target, i_state );
    if (t_state->is_error) {
        return t_state;
    }
    result* tr = t_state->result;
    memcpy(t_state, i_state, sizeof(state));
    t_state->result = tr;
    return t_state;
}
parser* lookAhead(parser* p) {
    return dcreate_parser(_loA, p);
}

mapresult* _bM(result* res, void *data) {
    mapresult* mr = malloc(sizeof(mapresult));
    mr->dealloc_old = false;
    if (res->data_type != RES_ARR) {
        puts("You've got to be bullshitting me. How tf do u have a sequence");
        puts("That isn't outputting an array? WTF");
        exit(3);
    }

    ResArrD* rad = (ResArrD*) res->data;
    result** p = (result**) rad->arr;
    mr->res = p[1];

    if (p[0] != NULL) {
        deallocate_result(p[0]);
    }
    if (p[2] != NULL) {
        deallocate_result(p[2]);
    }
    free(p);
    return mr;
}
parser* between(parser* before, parser* get, parser* after) {
    parser** ps = malloc( 3 * sizeof(parser**));
    ps[0] = before;
    ps[1] = get;
    ps[2] = after;
    parser* seq = sequenceOf( ps, 3);
    return cmap(seq, _bM);
}

typedef struct {
    parser* p1;
    parser* p2;
} _tpit;
state* _sPB(void* data, char* target, state* i_state) {
    _tpit* it = (_tpit*) data;
    #define res_arr_type result*
    ALLOCATE_SN(res_arr, 10, true, NULL);
    
    state* v_state = NULL;
    state* s_state = NULL;
    state* e_state = NULL;
    state* n_state = i_state;

    while (true) {
        LOG(printf("%s\n", state_to_string(n_state)))
        v_state = evaluate(it->p1, target, n_state);
        if (v_state->is_error) {
            LOG(puts("vstate errored so breaking"));
            e_state = v_state;
            break;
        }
        APPEND(res_arr, v_state->result);

        s_state = evaluate(it->p2, target, v_state);
        if (s_state->is_error) {
            break;
        }

        n_state = s_state;
    }

    if (e_state != NULL) {
        LOG(puts("in estate is not null"));
        for (int i = 0 ; i < ARR_SIZE(res_arr); i++) {
            LOG(printf("estate dealloc loop #%d\n", i));
            if (MUTARR(res_arr)[i] != NULL) {
                deallocate_result(MUTARR(res_arr)[i]);
            }
        }
        LOG(puts("done estate loop"));
        free(MUTARR(res_arr));
        LOG(puts("freed arr"));
        if (s_state != NULL) {
            LOG(puts("deallocing s_state"));
            deallocate_state(s_state);
        }
        LOG(puts("retting e_state"));
        LOG(printf("%p retting\n", e_state));
        return e_state;
    } else {
        LOG(puts("succ"));
        SHRINK_TO_NEEDED(res_arr);
        LOG(printf("after shrink %d\n", SIZEOF(res_arr)));
        LOG(puts("shrink fin"));
        result* res = create_resarr_result(MUTARR(res_arr), SIZEOF(res_arr));
        LOG(puts("res made"));
        state* f_state = create_result_state(res, v_state->index);
        LOG(printf("dtype %d\n", f_state->result->data_type));
        LOG(puts("f_state made"));

        if (s_state != NULL) {
            LOG(puts("s_state dealloc"));
            deallocate_state(s_state);
        }
        LOG(puts("s_state dealloc check done"));
        if (v_state != NULL) {
            LOG(puts("v_state free"));
            free(v_state);
        }
        LOG(puts("finned v_state dealloc check"));

        return f_state;
    }
    #undef res_arr_type
}
parser* sepBy(parser* get, parser* sep) {
    _tpit* it = malloc(sizeof(_tpit));
    it->p1 = get;
    it->p2 = sep;
    return dcreate_parser(_sPB, it);
}

state* _mP(void* data, char* target, state* i_state) {
    #define res_arr_type result*
    ALLOCATE(res_arr, 10);
    parser* p = (parser*) data;

    state* n_state = i_state;
    while (true) {
        state* v_state = evaluate(p, target, n_state);
        if (v_state->is_error) {
            break;
        } else {
            APPEND(res_arr, v_state->result);
            if (n_state != i_state) {
                free(n_state);
            }
            n_state = v_state;
        }
    }

    SHRINK_TO_NEEDED(res_arr);
    result* res = create_resarr_result(MUTARR(res_arr), SIZEOF(res_arr));
    return create_result_state(res, n_state->index);
    #undef res_arr_type
}
parser* many(parser* p) {
    return dcreate_parser(_mP, p);
}

parser* optionalWhitespace;

void init_comp_parsers() {
    optionalWhitespace = possibly(whitespace);
}
