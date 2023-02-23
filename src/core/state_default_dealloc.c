#include "state.h"
#include<stdlib.h>
#include<stdio.h>
#include "log.h"

void def_dealloc_string(result* res) {
    if (res->data.ptr != NULL) {
        kfree(res->data.ptr);
    }
    kfree(res);
}

void def_dealloc(result* res) {
    kfree(res);
}

void def_dealloc_resarr(result* res) {
    if (res->data.ptr != NULL) {
        ResArrD* rad = res->data.ptr;

        result* r;
        for (int i = 0; i < rad->a_len; i++) {
            if (rad->all_same_type) {
                r = create_result(rad->all_type, rad->arr[i]);
            } else {
                r = rad->arr[i].ptr;
            }
            if (r != NULL) {
                deallocate_result(r);                
            }
        }
        kfree(rad->arr);
        kfree(rad);
    }
    kfree(res);
}
