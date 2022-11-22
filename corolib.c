#include "corolib.h"

coroctx* default_ctx() {
    coroctx* ctx = malloc(sizeof(coroctx));
    ctx->state = 0;
    ctx->yield = NULL;
    ctx->result = NULL;
    ctx->lr = 0;
    ctx->fin = false;
    ctx->data = NULL;
}
