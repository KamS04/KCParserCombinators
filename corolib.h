#ifndef COROLIB_HEADER
#define COROLIB_HEADER
#include<stdbool.h>
#include<stdlib.h>

#define GET_MACRO(_0,_1,_2,_3,_4,_5,NAME,...) NAME

#define crBegin switch(ctx->state) { case 0:

#define crYield0 { \
                    ctx->state = __LINE__; \
                    ctx->yield = NULL; \
                    return; \
                    case __LINE__: \
                }
#define crYield1(yr) { \
                        ctx->state = __LINE__; \
                        ctx->yield = yr; \
                        return; \
                        case __LINE__: \
                    }
#define crYield2(o, yr) { \
                            crYield1(yr); \
                            o = ctx->lr; \
                        }
#define crYield3(type, o, yr) { \
                                crYield1(yr); \
                                o = (type) ctx->lr; \
                            }
#define crYield4(type, o, yr, mem) { \
                                crYield1(yr); \
                                o = (type) ( (lrtype) (ctx->lr))->mem; \
                            }
#define crYield5(type, o, yr, rtype, mem) { \
                                crYield1(yr); \
                                o = (type) ( (rtype) (ctx->lr))->mem; \
                            }
#define crYield(...) GET_MACRO(_0, __VA_ARGS__, crYield5, crYield4, crYield3, crYield2, crYield1, crYield0)(__VA_ARGS__)

#define crReturn(x) { \
                        ctx->result = x; \
                        ctx->fin = true; \
                        return; \
                    }}

#define dat(m) ((dtype*)(ctx->data))->m
#define ddat (dtype)(ctx->data)

typedef struct {
    int state;
    void* yield;
    void* result;
    bool fin;
    void* lr;
    void* data;
} coroctx;

coroctx* default_ctx();
#endif
