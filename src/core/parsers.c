#include "parsers.h"
#include "state.h"
#include "korolib.h"
#include<stdio.h>
#include<stdlib.h>
#include "log.h"
#include<stdbool.h>

enum ParserType{
    Parser,
    Map,
    Chain,
    Then,
    Manipulate,
    // Coroutine,
    Koroutine,
};

typedef struct funcparse {
    parserfunc_t parserfunc;
    DataUnion data;
    ddallocfunc_t dealloc_data;
    bool noc;
} funcparse;

parser* create_parser(parserfunc_t parse) {
    return ddcreate_parser(parse, (DataUnion){ .ptr = NULL }, NULL, false);
}
parser* dcreate_parser(parserfunc_t parse, DataUnion data ) {
    return ddcreate_parser(parse, data, NULL, false);
}
parser* ddcreate_parser(parserfunc_t parse, DataUnion data, ddallocfunc_t dealloc_data, bool noc ) {
    parser* n_parser = malloc(sizeof(parser));
    funcparse* dpf = malloc(sizeof(funcparse));
    dpf->parserfunc = parse;
    dpf->data = data;
    dpf->noc = noc;
    dpf->dealloc_data = dealloc_data;

    n_parser->type = Parser;
    n_parser->data = dpf;
    return n_parser;
}
void deallocate_basic_parser(parser* p) {
    funcparse* dpf = (funcparse*) p->data;
    if (dpf->dealloc_data != NULL) {
        dpf->dealloc_data(dpf->data);
    }
    kfree(dpf);
    kfree(p);
}

// TODO add datadealloc to mapitem
typedef struct mapitem {
    parser* first;
    mappertype mapper;
    DataUnion mapper_data;
    bool noc;
} mapitem;
parser* map( parser* in, mappertype mapper, bool noc, DataUnion data) {
    mapitem* item = malloc(sizeof(mapitem));
    item->first = in;
    item->mapper = mapper;
    item->noc = noc;
    item->mapper_data = data;

    parser* n_parser = malloc(sizeof(parser));
    n_parser->type = Map;
    n_parser->data = item;

    return n_parser;
}
parser* cmap(parser* in, mappertype mapper) {
    return map(in, mapper, false, (DataUnion){ .ptr = NULL });
}
void deallocate_mapper(parser* p) {
    if (p->type != Map) return;

    mapitem *item = p->data;
    deallocate_parser(item->first);
    kfree(item);
    kfree(p);
    return;
}

typedef struct chainitem {
    parser* first;
    chooserfunc_t chooser;
    bool noc;
    bool dp;
} chainitem;
parser* chain( parser* in, chooserfunc_t chooser, bool noc, bool dp) {
    chainitem* item = malloc(sizeof(chainitem));
    item->first = in;
    item->chooser = chooser;
    item->noc = noc;
    item->dp = dp;

    parser* n_p = malloc(sizeof(parser));
    n_p->data = item;
    n_p->type = Chain;

    return n_p;
}
parser* cchain(parser* in, chooserfunc_t chooser) {
    return chain(in, chooser, false, true);
}
parser* ndchain(parser* in, chooserfunc_t chooser) {
    return chain(in, chooser, false, false);
}

void deallocate_chain(parser* p) {
    if (p->type != Map) return;

    chainitem* item = p->data;
    deallocate_parser(item->first);
    kfree(p);
    kfree(item);
}

typedef struct thenitem {
    parser* first;
    parser* second;
    bool noc;
} thenitem;
parser* then(parser* in, parser* next, bool noc) {
    thenitem* item = malloc(sizeof(thenitem));
    item->first = in;
    item->second = next;
    item->noc = noc;

    parser* n_p = malloc(sizeof(parser));
    n_p->type = Then;
    n_p->data = item;

    return n_p;
}
parser* cthen(parser* in, parser* next) {
    return then(in, next, false);
}
void deallocate_then(parser* p) {
    if (p->type != Then) return;

    thenitem* item = p->data;
    deallocate_parser(item->first);
    deallocate_parser(item->second);
    kfree(item);
    kfree(p);
    return;
}

typedef struct maniitem {
    parser* first;
    manipfunc_t manipulator;
    bool noc;
} maniitem;
parser* manipulate(parser* in, manipfunc_t manipulator, bool noc) {
    maniitem* item = malloc(sizeof(maniitem));
    item->first = in;
    item->manipulator = manipulator;
    item->noc = noc;

    parser* np = malloc(sizeof(parser));
    np->type = Manipulate;
    np->data = item;

    return np;
}
parser* cmanipulate(parser* in, manipfunc_t manipulator) {
    return manipulate(in, manipulator, false);
}
void deallocate_manipulate(parser* p) {
    if (p->type != Manipulate) return;
    maniitem* item = p->data;
    deallocate_parser(item->first);
    kfree(item);
    kfree(p);
    return;
}

typedef struct koroitem {
    koroutinefunc_t koroutine;
    bool noc;
} koroitem;
parser* korop(koroutinefunc_t koro, bool noc) {
    koroitem* item = malloc(sizeof(koroitem));
    item->koroutine = koro;
    item->noc = noc;

    parser* n_p = malloc(sizeof(parser));
    n_p->data = item;
    n_p->type = Koroutine;
    return n_p;
}
void deallocate_koro(parser* p) {
    if (p->type != Koroutine) return;
    koroitem* ki = p->data;
    kfree(ki);
    kfree(p);
    return;
}


void deallocate_parser(parser* p) {
    switch (p->type) {
        case Parser:
            return deallocate_basic_parser(p);
        case Map:
            return deallocate_mapper(p);
        case Chain:
            return deallocate_chain(p);
        case Then:
            return deallocate_then(p);
        case Manipulate:
            return deallocate_manipulate(p);
        // case Coroutine:
            // return deallocate_coro(p);
        case Koroutine:
            return deallocate_koro(p);
    };
}

state* run(parser* p, char* c) {
    return evaluate(p, c, default_state());
}

state* evaluate_basic_parser(parser* p, char* c, state* i_state) {
    funcparse* dpf = (funcparse*) p->data;
    if (i_state->is_error && !dpf->noc) return i_state;
    parserfunc_t parse = dpf->parserfunc;
    state* n_state = parse(dpf->data, c, i_state);
    return n_state;
}

state* evaluate_map(parser* p, char* c, state* i_state) {
    mapitem* mi = p->data;
    state* f_state = evaluate(mi->first, c, i_state);
    if (f_state->is_error && !mi->noc) return f_state;
    mapresult* mr = (mi->mapper)(f_state->result, mi->mapper_data);
    state* n_state = result_here(f_state, mr->res );
    if (mr->dealloc_old) {
        LOG(puts("MAP: deallocing fstate"));
        deallocate_state(f_state);
    } else {
        kfree(f_state);
    }
    kfree(mr);
    return n_state;
}

state* evaluate_chain(parser* p, char* c, state* i_state) {
    chainitem* ci = p->data;
    state* f_state = evaluate(ci->first, c, i_state);
    if (f_state->is_error && !ci->noc) return f_state;
    parser* next = (ci->chooser)(f_state);
    state* n_state = evaluate(next, c, f_state);
    if (n_state->dealloc_old) {
        deallocate_state(f_state);
    }
    if (ci->dp) {
        deallocate_parser(next);
    }
    return n_state;
}

state* evaluate_then(parser* p, char* c, state* i_state) {
    thenitem* ti = p->data;
    state* f_state = evaluate(ti->first, c, i_state);
    if (f_state->is_error && !ti->noc) return f_state;
    state* n_state = evaluate(ti->second, c, f_state);
    if (n_state->dealloc_old) {
        deallocate_state(f_state);
    }
    return n_state;
}

state* evaluate_manipulate(parser* p, char* c, state* i_state) {
    maniitem* mni = p->data;
    state* f_state = evaluate(mni->first, c, i_state);
    if (f_state->is_error && !mni->noc) return f_state;
    state* n_state = (mni->manipulator)(f_state);
    if (n_state->dealloc_old) {
        deallocate_state(f_state);
    }
    return n_state;
}

state* evaluate_koro(parser* p, char* c, state* i_state) {
    koroitem* ki = p->data;
    if (i_state->is_error && !ki->noc) return i_state;
    koroctx* kctx = default_kctx();
    void(*koroutine)(koroctx*) = ki->koroutine;
    bool ready_dealloc = false;
    state* n_state;

    while (1) {
        koroutine(kctx);
        if (kctx->fin)
            break;
        
        LOG(puts("KORO: yielded"));
        LOG(kctx->stringer && printf("KORO: dat after yield %s\n", kctx->stringer(kctx->data)));

        parser* next_parser = kctx->yield;
        LOG(printf("KORO: idx %d with %p\n", i_state->index, next_parser));
        n_state = evaluate(next_parser, c, i_state);
        LOG(puts("KORO: coro yield eval'd"));
        if (ready_dealloc && n_state->dealloc_old) {
            if (kctx->using_last_value) {
                LOG(printf("KORO: safe dealloc start --"));
                kfree(i_state->result); // kfree without deallocing values
                kfree(i_state);
                LOG(puts("end"));
            } else {
                LOG(printf("KORO; unsafe dealloc start -- "));
                deallocate_state(i_state); // deallocate everything
                LOG(puts("end"));
            }
        }
        ready_dealloc = true;
        if (n_state->is_error) {
            return n_state;
        }
        kctx->last_result = n_state->result;
        i_state = n_state;
    }

    LOG(printf("KORO: res %p rtype %d rdpoint %d\n", kctx->fin_result, ((result*)kctx->fin_result)->data_type, ((result*)kctx->fin_result)->data_type));
    state* res = create_result_state(kctx->fin_result, n_state->index);
    kfree(kctx);
    return res;
}

state* evaluate(parser* p, char* c, state* i_state) {
    switch (p->type) {
        case Parser: {
            return evaluate_basic_parser(p, c, i_state);
        }
        case Map:
            return evaluate_map(p, c, i_state);
        case Chain:
            return evaluate_chain(p, c, i_state);
        case Then:
            return evaluate_then(p, c, i_state);
        case Manipulate:
            return evaluate_manipulate(p, c, i_state);
        case Koroutine:
            return evaluate_koro(p, c, i_state);
        default:
            printf("Unknown Parser Type: %d\n", p->type);
            exit(3);
    }
}
