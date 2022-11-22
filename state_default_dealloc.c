#include "state.h"
#include<stdlib.h>

void def_dealloc_string(result* res) {
    free(res->data);
    free(res);
}

void def_dealloc(result* res) {
    free(res);
}

void def_dealloc_resarr(result* res) {
    ResArrD* rad = res->data;
    for (int i = 0; i < rad->a_len; i++) {
        if (rad->arr[i] != NULL) {
            deallocate_result(rad->arr[i]);
        }
    }
    free(rad->arr);
    free(rad);
    free(res);
}
