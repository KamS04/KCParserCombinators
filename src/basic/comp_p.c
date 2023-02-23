#include<string.h>
#include<stdio.h>
#include "parsers.h"
#include "state.h"
#include "string_p.h"
#include "util_p.h"
#include "mutarr.h"
#include "log.h"

state* _posP(DataUnion data, char* target, state* i_state) {
    parser* orig = data.ptr;
    state* first = evaluate(orig, target, i_state);
    if (first->is_error) {
        state* final = result_here(first, NULL);
        kfree(first);
        return final;
    }
    return first;
}
parser* possibly(parser* p) {
    return ddcreate_parser(_posP, (DataUnion){ .ptr = p }, NULL, true);
}

typedef struct _euit {
    bool cue;
    parser* p;
} _euit;
state* _eUP(DataUnion data, char* target, state* i_state) {
    _euit* it = data.ptr;
    parser* p = it->p;
    int st = i_state->index;

    #ifdef SINGLE_THREADED_ONLY
    static char* last_ch = NULL;
    static int tlen = 0;
    
    if (target != last_ch) {
        tlen = strlen(target);
        last_ch = target;
    }
    #else
    int tlen = strlen(target);
    #endif
    
    state* c_state = default_state();
    state* n_state;
    c_state->index = i_state->index;
    while (c_state->index < tlen) {
        n_state = evaluate(p, target, c_state);
        if (!n_state->is_error) {
            break;
        }
        c_state->index += 1;
        deallocate_state(n_state);
    }

    int en = c_state->index;
    if (en >= tlen) {
        if (it->cue) {
            n_state = evaluate(p, target, c_state);
            if (n_state->error) {
                char* err = malloc(33 * sizeof(char));
                return create_error_state(err, i_state->index);
            }
        }
    }
    char* sres = malloc((en-st+1) * sizeof(char));
    memcpy(sres, target+st, (en-st));
    sres[en-st] = '\0';
    result* res = create_result(STRING, (DataUnion){ .ptr = sres });
    return create_result_state(res, en);
}
parser* everythingUntil(parser* p, bool check_till_end) {
    _euit* it = malloc(sizeof(_euit));
    it->p = p;
    it->cue = check_till_end;
    return ddcreate_parser(_eUP, (DataUnion){ .ptr = it }, NULL, false);
}

state* _aCE(DataUnion data, char* target, state* i_state) {
    parser* p = data.ptr;
    state* c_state = evaluate(p, target, i_state);
    if (c_state->is_error) {
        result* res = create_result(CHAR, (DataUnion){ .ch = target[i_state->index] });
        return create_result_state(res, i_state->index + 1);
    }
    char* err = malloc(34 * sizeof(char));
    strcpy(err, "anyCharExcept : Matched exception");
    return create_error_state(err, i_state->index);
}
parser* anyCharExcept(parser* p) {
    return dcreate_parser(_aCE, (DataUnion){ .ptr = p });
}

typedef struct _cit {
    int p_size;
    parser** parsers;
} _cit;
state* _cBP(DataUnion data, char* target, state* i_state) {
    _cit* it = data.ptr;
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
    return dcreate_parser(_cBP, (DataUnion){ .ptr = it });
}

// typedef struct _sopit {
//     int
// } _sopit;
state* _sOP(DataUnion data, char* target, state* i_state) {
    _cit* it = data.ptr;
    parser** ps = it->parsers;
    state* s_state = i_state;
    // in this case every element will be a ptr
    DataUnion* resarr = malloc( it->p_size * sizeof(result*) );
    for (int i = 0; i < it->p_size; i++) {
        resarr[i].ptr = NULL;
    }
    bool sequenceBreak = false;
    for (int i = 0; i < it->p_size; i++) {
        parser* cp = ps[i];
        s_state = evaluate(cp, target, s_state);
        if (s_state->is_error) {
            sequenceBreak = true;
            break;
        }
        resarr[i].ptr = s_state->result;
    }
    if (sequenceBreak) {
        for (int i = 0; i < it->p_size; i++) {
            if (resarr[i].ptr != NULL) {
                deallocate_result(resarr[i].ptr);
            }
        }
        kfree(resarr);
        int s_len = strlen(s_state->error);
        char* err = malloc( (s_len + 18) * sizeof(char) );
        sprintf(err, "Sequence Break : %s", s_state->error);
        if (s_state->error_from_malloc) {
            kfree(s_state->error);
        }
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
    return dcreate_parser(_sOP, (DataUnion){ .ptr = it });
}


state* _loA(DataUnion data, char* target, state* i_state) {
    state* t_state = evaluate( data.ptr, target, i_state );
    if (t_state->is_error) {
        return t_state;
    }
    result* tr = t_state->result;
    memcpy(t_state, i_state, sizeof(state));
    t_state->result = tr;
    return t_state;
}
parser* lookAhead(parser* p) {
    return dcreate_parser(_loA, (DataUnion){ .ptr = p });
}

mapresult* _bM(result* res, DataUnion data) {
    mapresult* mr = malloc(sizeof(mapresult));
    mr->dealloc_old = false;
    if (res->data_type != RES_ARR) {
        puts("You've got to be bullshitting me. How tf do u have a sequence");
        puts("That isn't outputting an array? WTF");
        exit(3);
    }

    ResArrD* rad = (ResArrD*) res->data.ptr;
    DataUnion* p = rad->arr;

    mr->res = p[1].ptr;

    if (p[0].ptr != NULL) {
        deallocate_result(p[0].ptr);
    }
    if (p[2].ptr != NULL) {
        deallocate_result(p[2].ptr);
    }
    kfree(p);
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

typedef struct _tpit {
    parser* p1;
    parser* p2;
} _tpit;
state* _sPB(DataUnion data, char* target, state* i_state) {
    _tpit* it = data.ptr;
    #define res_arr_type DataUnion
    ALLOCATE_SN(res_arr, 10, true, (DataUnion){ .ptr = NULL });
    
    state* v_state = NULL;
    state* s_state = NULL;
    state* e_state = NULL;
    state* n_state = i_state;

    while (true) {
        LOG(printf("sepby: %s\n", state_to_string(n_state)))
        v_state = evaluate(it->p1, target, n_state);
        LOG(puts("sepby: getter eval'd"));
        if (v_state->is_error) {
            LOG(puts("sepby: vstate errored so breaking"));
            e_state = v_state;
            break;
        }
        APPEND(res_arr, (DataUnion){ .ptr = v_state->result });
        LOG(puts("sepby: added to array"));

        s_state = evaluate(it->p2, target, v_state);
        LOG(puts("sepby: separator eval'd"));
        if (s_state->is_error) {
            LOG(puts("sepby: seperator failed"));
            break;
        }

        n_state = s_state;
    }

    if (e_state != NULL) {
        LOG(puts("in estate is not null"));
        for (int i = 0 ; i < ARR_SIZE(res_arr); i++) {
            LOG(printf("estate dealloc loop #%d\n", i));
            if (MUTARR(res_arr)[i].ptr != NULL) {
                deallocate_result(MUTARR(res_arr)[i].ptr);
            }
        }
        LOG(puts("done estate loop"));
        kfree(MUTARR(res_arr));
        LOG(puts("kfreed arr"));
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
            LOG(puts("v_state kfree"));
            kfree(v_state);
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
    return dcreate_parser(_sPB, (DataUnion){ .ptr = it });
}

typedef struct ManyData {
    bool atleast1;
    parser* p;
    bool all_same_type;
    int all_type;
} ManyData;
state* _mP(DataUnion data, char* target, state* i_state) {
    #define res_arr_type DataUnion
    ALLOCATE_SN(res_arr, 10, 0, (DataUnion){ .ptr = NULL });
    ManyData* md = data.ptr;
    parser* p = md->p;

    state* n_state = i_state;
    state* v_state;
    while (true) {
        v_state = evaluate(p, target, n_state);
        if (v_state->is_error) {
            break;
        } else {
            if (md->all_same_type) {
                APPEND(res_arr, v_state->result->data);
            } else {
                APPEND(res_arr, (DataUnion){ .ptr = v_state->result });
            }
            if (n_state != i_state) {
                if (md->all_same_type) {
                    kfree(n_state->result);
                }
                kfree(n_state);
            }
            n_state = v_state;
        }
    }

    if (md->atleast1 && CARR_SIZE(res_arr) < 1) {
        DELETE(res_arr);
        state* st = create_error_state("Many1: 0 Items matched", i_state->index);
        st->error_from_malloc = false;
        return st;
    }

    SHRINK_TO_NEEDED(res_arr);

    result* res;
    if (md->all_same_type) {
        res = dcreate_resarr_result(MUTARR(res_arr), SIZEOF(res_arr), true, md->all_type);
    } else {
        res = create_resarr_result(MUTARR(res_arr), SIZEOF(res_arr));
    }

    state* out = create_result_state(res, n_state->index);
    out->error_from_malloc = v_state->error_from_malloc;
    out->error = v_state->error;
    v_state->error = NULL;

    deallocate_state(v_state);

    return out;
    #undef res_arr_type
}
parser* dmany(parser* p, bool atleast1, bool all_same_type, int all_type) {
    ManyData* md = malloc(sizeof(ManyData));
    md->p = p;
    md->atleast1 = atleast1;
    md->all_same_type = all_same_type;
    md->all_type = all_type;
    return dcreate_parser(_mP, (DataUnion){ .ptr = md });
}
parser* many(parser* p) {
    return dmany(p, 0, 0, 0);
}
parser* many1(parser* p) {
    return dmany(p, true, false, 0);
}
parser* manyAS(parser* p, int atype) {
    return dmany(p, false, true, atype);
}
parser* manyAS1(parser* p, int atype) {
    return dmany(p, true, true, atype);
}

parser* optionalWhitespace;

void init_comp_parsers() {
    optionalWhitespace = possibly(whitespace);
}
