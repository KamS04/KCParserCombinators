#ifndef KOROLIB_HEADER
#define KOROLIB_HEADER
#include<stdbool.h>
#include<stdlib.h>

#define GET_5_MACRO(_0, _1, _2, _3, _4, _5, NAME, ...) NAME
#define GET_1_MACRO(_0, _1, NAME, ...) NAME

#define _KCTX kctx
#define KDDAT _KCTX->data
#define KMDAT(mem) ((_kdtype*)_KCTX->data)->mem
#define KCDAT (_kdtype*)(_KCTX->data)

#define KLAST _KCTX->last_result
#define KCLAST (_krtype*)KLAST

#define crBegin _KCTX->deallocer = NULL; \
                switch(_KCTX->state) { case 0:

#define ALLOCATE_DAT_NULL() KDDAT = malloc(sizeof(_kdtype));

#define ALLOCATE_DAT_DEALLOC(deallocer) _KCTX->dealloc = deallocer; \
                                ALLOCATE_DAT_NULL()

#define ALLOCATE_DAT(...) GET_1_MACRO(_0, ##__VA_ARGS__, ALLOCATE_DAT_DEALLOC, ALLOCATE_DAT_NULL)(__VA_ARGS__)

// yield NULL;
#define crYieldEmpty() _KCTX->state = __LINE__; \
                        _KCTX->yield = NULL; \
                        return; \
                        case __LINE__:

// yield x;
#define crYieldDirect(yield_item) _KCTX->state = __LINE__; \
                        _KCTX->yield = yield_item; \
                        return; \
                        case __LINE__:

// out = yield x;
#define crYieldSet(out, yield_item) crYieldDirect(yield_item) \
                                out = KLAST;

// out = (cast) yield x;
#define crYieldCSet(out, cast, yield_item) crYieldDirect(yield_item) \
                                        out = (cast) KLAST;

// out = (cast) ((_krtype) yield x)->member
#define crYieldCMSet(out, cast, yield_item, member) crYieldDirect(yield_item) \
                                                out = (cast) (KCLAST)->member;

// out = (ocast) ((icast) yield x)->member
#define crYieldCMCSet(out, ocast, icast, yield_item, member) crYieldDirect(yield_item) \
                                                        out = (ocast) ( (icast)KLAST )->member;

#define crYield(...) GET_5_MACRO(_0, ##__VA_ARGS__, crYieldCMCSet, crYieldCMSet, crYieldCSet, crYieldSet, crYieldDirect, crYieldEmpty)(__VA_ARGS__)

// use cru yield if you would like to keep the last value
#define cruYield(...) _KCTX->using_last_value = true; \
                                crYield(__VA_ARGS__) \
                                _KCTX->using_last_value = false;

#define crReturn(x) _KCTX->fin_result = x; \
                    _KCTX->fin = true; \
                    return; }

typedef struct koroctx {
    int state;
    void* yield;
    void* fin_result;
    bool fin;
    void* last_result;
    void* data;
    bool using_last_value;
    void(*deallocer)(void*);
    char*(*stringer)(void*);
} koroctx;

koroctx* default_kctx();

typedef void(*koroutinefunc_t)(koroctx*);

#endif
