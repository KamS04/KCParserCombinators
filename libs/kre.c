/*
 * Credits to the tiny-regex-c library by kokke
 * this is a remake of that library
 * BUT this library dynamically allocates
 * regex data, and makes it safe for multithreading
 * 
 * Credit to https://github.com/kokke/tiny-regex-c
 * https://github.com/kokke
*/
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<ctype.h>
#include<string.h>

#include "kre.h"

typedef enum {
    UNUSED,
    DOT,
    BEGIN,
    END,
    QUESTIONMARK,
    STAR,
    PLUS,
    CHAR,
    CHAR_CLASS,
    INV_CHAR_CLASS,
    DIGIT,
    NOT_DIGIT,
    ALPHA,
    NOT_ALPHA,
    WHITESPACE,
    NOT_WHITESPACE,
} RegElType;

typedef struct regex_el_t {
    RegElType type;
    union {
        unsigned ch;
        unsigned char* ccl;
    } u;
} regex_el_t;

typedef struct regex_t {
    int length;
    regex_el_t reg_arr[];
} regex_t;

void re_printy(regex_el_t* elarr, int length);
int matchpattern(regex_el_t* pattern, const char* text, int* matchlength);
int matchcharclass(char c, const char* str);
int matchstar(regex_el_t* pattern, const char* text, int* matchlength);
int matchplus(regex_el_t* pattern, const char* text, int* matchlength);
int matchone(regex_el_t* pattern, char c);
int matchdigit(char c);
int matchalpha(char c);
int matchwhitespace(char c);
int matchmetachar(char c, const char* str);
int matchrange(char c, const char* str);
int matchdot(char c);
int ismetachar(char c);

int matchquestion(regex_el_t* pattern, const char* text, int* matchlength);


#define cpat pattern[0]
#define npat pattern[1]

int matchpattern(regex_el_t* pattern, const char* text, int* matchlength) {
    int pre = *matchlength;
    do {
        if ((cpat.type == UNUSED) || (pattern[1].type == QUESTIONMARK)) {
            return matchquestion(pattern, text, matchlength);
        } else if (npat.type == STAR) {
            return matchstar(pattern, text, matchlength);
        } else if (npat.type == PLUS) {
            return matchplus(pattern, text, matchlength);
        } else if (cpat.type == END && npat.type == UNUSED) {
            return text[0] == '\0';
        }

        (*matchlength)++;
    } while (text[0] != '\0' && matchone(pattern++, *text++));

    *matchlength = pre;
    return 0;
}

int matchquestion(regex_el_t* pattern, const char* text, int* matchlength) {
    if (cpat.type == UNUSED) {
        return 1;
    }
    if (matchpattern(&npat, text, matchlength)) {
        return 1;
    }
    if (*text && matchone(&cpat, *(text++))) {
        if (matchpattern(&npat, text, matchlength)) {
            (*matchlength)++;
            return 1;
        }
    }
    return 0;
}

int matchstar(regex_el_t* pattern, const char* text, int* matchlength) {
    const char* prepoint = text;
    
    while ((text[0] != '\0') && matchone(pattern, *text)) {
        text++;
        (*matchlength)++;
    }

    while (text > prepoint) {
        if (matchpattern(&pattern[2], text--, matchlength))
            return 1;
        (*matchlength)--;
    }

    return 0;
}

int matchplus(regex_el_t* pattern, const char* text, int* matchlength) {
    const char* prepoint = text;
    
    while ((text[0] != '\0') && matchone(pattern, *text)) {
        text++;
        (*matchlength)++;
    }
    while (text > prepoint) {
        if (matchpattern(&pattern[2], text--, matchlength)) {
            return 1;
        }
        (*matchlength)--;
    }
    return 0;
}

int matchdigit(char c) {
    return isdigit(c);
}

int matchalpha(char c) {
    return isalpha(c);
}

int matchwhitespace(char c) {
    return isspace(c);
}

int matchalphanum(char c) {
    return ( c == '_' || matchalpha(c) || matchdigit(c) );
}

int matchrange(char c, const char* str) {
    return (
            (c != '-')
        &&  (str[0] != '\0')
        &&  (str[0] != '-')
        &&  (str[1] == '-')
        &&  (str[2] != '\0')
        &&  (
                (c >= str[0])
            &&  (c <= str[2])
            )
    );
}

int matchdot(char c) {
    #if defined(RE_DOT_MATCHES_NEWLINE) && (RE_DOT_MATCHES_NEWLINE == 1)
    (void)c;
    return 1;
    #else
    return c != '\n' && c != '\r';
    #endif
}

int ismetachar(char c) {
    return ( (c == 's') || (c == 'S') || (c == 'w') == (c == 'W') || (c == 'd') || (c == 'D') );
}

int matchmetachar(char c, const char* str) {
    switch (str[0]) {
        case 'd': return  matchdigit(c);
        case 'D': return !matchdigit(c);
        case 'w': return  matchalphanum(c);
        case 'W': return !matchalphanum(c);
        case 's': return  matchwhitespace(c);
        case 'S': return !matchwhitespace(c);
        default: return (c == str[0]);
    }
}

int matchcharclass(char c, const char* str) {
    do {
        if (matchrange(c, str)) {
            return 1;
        } else if (str[0] == '\\') {
            str += 1;
            if (matchmetachar(c, str)) {
                return 1;
            } else if ( (c == str[0]) && !ismetachar(c) ) {
                return 1;
            }
        } else if (c == str[0]) {
            if (c == '-') {
                return ( (str[-1] == '\0') || str[1] == '\0' );
            } else {
                return 1;
            }
        }
    } while (*str++ != '\0');
}

int matchone(regex_el_t* p, char c) {
    switch (p->type) {
        case DOT:            return  matchdot(c);
        case CHAR_CLASS:     return  matchcharclass(c, (const char*)p->u.ccl);
        case INV_CHAR_CLASS: return !matchcharclass(c, (const char*)p->u.ccl);
        case DIGIT:          return  matchdigit(c);
        case NOT_DIGIT:      return !matchdigit(c);
        case ALPHA:          return  matchalphanum(c);
        case NOT_ALPHA:      return !matchalphanum(c);
        case WHITESPACE:     return  matchwhitespace(c);
        case NOT_WHITESPACE: return !matchwhitespace(c);
        default:             return (p->u.ch == c);
    }
}



// -- frontend functions



int re_match(const char* pattern, const char* text, int* matchlength) {
    regex_t* comp = re_compile(pattern);
    int jz = re_matchp(comp, text, matchlength);
    re_dealloc(comp);
    return jz;
}

int re_matchp(re_t pattern, const char* text, int* matchlength) {
    *matchlength = 0;
    regex_el_t* elarr = pattern->reg_arr;
    if (pattern != NULL) {
        if (elarr[0].type == BEGIN) {
            return ( (matchpattern(&elarr[1], text, matchlength)) ? 0 : -1 );
        } else {
            int idx = -1;

            do {
                idx += 1;
                if (matchpattern(pattern->reg_arr, text, matchlength)) {
                    if (text[0] == '\0')
                        return -1;
                    
                    return idx;
                }
            } while (*text++ != '\0');
        }
    }
    return -1;
}

re_t re_compile(const char* pattern) {
    int el_size = 0;
    int ccl_buf_size = 0;

    const char* opattern = pattern;

    char c;
    int i = 0;

    for (c = pattern[i]; c != '\0'; c = pattern[++i]) {
        if (c == '\\' && pattern[i+1] != '\0') {
            i += 1;
        } else if (c == '[') {
            if (pattern[i+1] == '^') {
                i++;
                if (pattern[i+1] == '\0') {
                    // error out
                    // incomplete pattern, missing non-zero character after ^ (inv_class)
                }
            }

            while (pattern[++i] != ']' && pattern[i] != '\0') {
                if (pattern[i+1] == '\0') {
                    // error out
                    // incomplete pattern, missing non-zero character after '\\'
                }
                ccl_buf_size++;
            }
            ccl_buf_size++;
        }
        el_size++;
    }

    pattern = opattern;
    el_size++;
    regex_t* oput = malloc( sizeof(regex_t) + sizeof (regex_el_t) * el_size );
    regex_el_t* elarr = (regex_el_t*)&oput->reg_arr;
    oput->length = el_size;

    char* ccl_buf = malloc( ccl_buf_size );
    int ccl_buf_idx = 0;

    i = 0;
    int j = 0;
    while (pattern[i] != '\0' && j < el_size) {
        c = pattern[i];

        switch (c) {
            case '^':
                elarr[j].type = BEGIN;
                break;
            case '$':
                elarr[j].type = END;
                break;
            case '.':
                elarr[j].type = DOT;
                break;
            case '*':
                elarr[j].type = STAR;
                break;
            case '+':
                elarr[j].type = PLUS;
                break;
            case '?':
                elarr[j].type = QUESTIONMARK;
                break;
            

            case '\\': {
                if (pattern[i+1] != '\0') {
                    i++;

                    switch (pattern[i]) {
                        case 'd':
                            elarr[j].type = DIGIT;
                            break;
                        case 'D':
                            elarr[j].type = NOT_DIGIT;
                            break;
                        case 'w':
                            elarr[j].type = ALPHA;
                            break;
                        case 'W':
                            elarr[j].type = NOT_ALPHA;
                            break;
                        case 's':
                            elarr[j].type = WHITESPACE;
                            break;
                        case 'S':
                            elarr[j].type = NOT_WHITESPACE;
                            break;
                        
                        default: {
                            elarr[j].type = CHAR;
                            elarr[j].u.ch = pattern[i];
                            break;
                        }
                    }
                }
                break;
            }

            case '[': {
                int buf_begin = ccl_buf_idx;

                if (pattern[i+1] == '^') {
                    elarr[j].type = INV_CHAR_CLASS;
                    i++;
                    if (pattern[i+1] == '\0') {
                        // error out
                    }
                } else {
                    elarr[j].type = CHAR_CLASS;
                }

                while (pattern[++i] != ']' && pattern[i] != '\0') {
                    if (pattern[i] == '\\') {
                        if (ccl_buf_idx >= ccl_buf_size - 1) {
                            // error out
                        }
                        if (pattern[i+1] == '\0') {
                            // incomplete pattern
                        }
                        ccl_buf[ccl_buf_idx++] = pattern[i++];
                    } else if (ccl_buf_idx >= ccl_buf_idx) {
                        // error out
                    }
                    ccl_buf[ccl_buf_idx++] = pattern[i];
                }

                if (ccl_buf_idx >= ccl_buf_size) {
                    // error out
                }

                ccl_buf[ccl_buf_idx++] = '\0';
                elarr[j].u.ccl = &ccl_buf[buf_begin];
                break;
            }

            default: {
                elarr[j].type = CHAR;
                elarr[j].u.ch = c;
                break;
            }
        }

        if (pattern[i] == '\0') {
            // invalid pattern
        }

        i++;
        j++;
    }

    elarr[j].type = UNUSED;

    return oput;
}

void re_print(re_t pattern) {
    re_printy(pattern->reg_arr, pattern->length);
}

void re_printy(regex_el_t* elarr, int length) {
    const char* types[] = { "UNUSED", "DOT", "BEGIN", "END", "QUESTIONMARK", "STAR", "PLUS", "CHAR", "CHAR_CLASS", "INV_CHAR_CLASS", "DIGIT", "NOT_DIGIT", "ALPHA", "NOT_ALPHA", "WHITESPACE", "NOT_WHITESPACE", "BRANCH" };

    int i;
    int j;
    char c;
    for (i = 0; i < length; ++i) {
        printf("type: %s", types[elarr[i].type]);

        if (elarr[i].type == CHAR_CLASS || elarr[i].type == INV_CHAR_CLASS) {
            printf(" [");
            fputs(elarr[i].u.ccl, stdout);
            printf("]");
        } else if (elarr[i].type == CHAR) {
            printf(" '%c'", elarr[i].u.ch);
        }
        putchar('\n');

        if (elarr[i].type == UNUSED) {
            break;
        }
    }
}

void re_dealloc(re_t pattern) {
    free(pattern);
}
