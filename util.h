#ifndef UTIL_HEADER
#define UTIL_HEADER
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<ctype.h>

int int_size(int c);

char* int_to_string(int i);

int pointer_size(void* c);

char* pointer_to_string(void* i);

int linear_search_prim(int f, int* arr, int size);

int linear_search(void* f, void** arr, int size, bool(*cmp)(void*,void*));

char toUpper(char c);
void lowerString(char* c, int len);

char toLower(char c);
void upperString(char* c, int len);

bool is_letter(char c);
bool is_digit(char c);
bool is_whitespace(char c);

int _find_last_match(int st, int st_len, char* str, bool(*cmp)(char c));
#endif