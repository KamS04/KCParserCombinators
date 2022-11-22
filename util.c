#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include "util.h"

int int_size(int c) {
    if (c == 0) { return 1; }
    if (c < 0) { return 1 + int_size(c*-1); }
    return (int)log10( (double) c ) + 1;
}

char* int_to_string(int i) {
    int s = int_size(i) + 1;
    char* c = malloc( s * sizeof(char) );
    sprintf(c, "%d", i);
    return c;
}

int pointer_size(void* c) {
    return snprintf(NULL, 0, "%p", c);
}

char* pointer_to_string(void* i) {
    int s = pointer_size(i) + 1;
    char* c = malloc( s * sizeof(char) );
    sprintf(c, "%p", i);
    return c;
}

int linear_search_prim(int f, int* arr, int size) {
    int idx = -1;
    for (int i = 0; i < size; i++) {
        if (arr[i] == f) {
            idx = i;
            break;
        }
    }

    return idx;
}

int linear_search(void* f, void** arr, int size, bool(*cmp)(void*,void*)) {
    int idx = -1;
    for (int i = 0; i < size; i++) {
        if (cmp(f, arr[i])) {
            idx = i;
            break;
        }
    }
    return idx;
}


char toUpper(char c) {
    if ('a' <= c && c <= 'z') {
        return c - 32;
    }
    return c;
}
void upperString(char* c, int len) {
    for (int i = 0; i < len; i++) {
        c[i] = toUpper(c[i]);
    }
}

char toLower(char c) {
    if ('A' <= c && c <= 'Z') {
        return c + 32;
    }
    return c;
}
void lowerString(char* c, int len) {
    for (int i = 0; i < len; i++) {
        c[i] = toLower(c[i]);
    }
}

bool is_letter(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

int _find_last_match(int st, int st_len, char* str, bool(*cmp)(char c)) {
    int en = st-1;
    for (int i = st; i <= st_len; i++) {
        if (cmp(str[i])) {
            en = i;
        } else {
            break;
        }
    }
    return en;
}
