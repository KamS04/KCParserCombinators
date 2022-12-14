#include "korolib.h"

koroctx* default_kctx() {
    koroctx* ctx = malloc(sizeof(koroctx));
    ctx->state = 0;
    ctx->yield = NULL;
    ctx->fin_result = NULL;
    ctx->fin = false;
    ctx->last_result = NULL;
    ctx->data = NULL;
    ctx->using_last_value = false;
    ctx->deallocer = NULL;
    ctx->stringer = NULL;
}
