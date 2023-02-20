/*
 * Credits to the tiny-regex-c library by kokke
 * this is a remake of that library
 * BUT this library dynamically allocates
 * regex data, and makes it safe for multithreading
 * 
 * Credit to https://github.com/kokke/tiny-regex-c
 * https://github.com/kokke
*/

#ifndef KC_REGEX_C
#define KC_REGEX_C

#ifndef RE_DOT_MATCHES_NEWLINE
#define RE_DOT_MATCHES_NEWLINE 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct regex_t* re_t;

re_t re_compile(const char* pattern);

int re_matchp(re_t pattern, const char* text, int* matchlength);

int re_match(const char* pattern, const char* text, int* matchlength);

void re_dealloc(re_t regex);

void re_print(re_t pattern);

#ifdef __cplusplus
}
#endif

#endif