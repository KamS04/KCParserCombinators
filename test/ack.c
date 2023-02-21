#include<math.h>
#include<stdio.h>
#include<stdlib.h>
// #include "core_p.h"
#include "mutarr.h"
// #include "kc_config.h"
// #include "state.h"
// #include "parsers.h"
// #include "character_p.h"
// #include "string_p.h"
// #include "util_p.h"
// #include "comp_p.h"
#include "log.h"
#include "parselib.h"

void print_state(state* s) {
    // printf("state p: %p ise: %d !ise: %d\n", s, s->is_error, !s->is_error);
    char* ss = state_to_string(s);
    printf("%s\n", ss );
    kfree(ss);
    if (!s->is_error) {
        if (s->result == NULL) {
            puts("Result: Null");
        } else {
            char* r = result_to_string(s->result);
            puts("Result:");
            printf("%s\n", r);
            kfree(r);
        }
    }
}

int str_to_int(char* o, int nd) {
    int s = 0;
    for (int i = 0; i < nd; i++) {
        s += pow(10, i) * (o[nd-i-1] - '0');
    }
    return s;
}

void test(parser** ps, int idx, char* target) {
    parser* p = ps[idx];
    printf("Testing parser #%d @%p with string %s\n", idx, p, target);
    state* s = run(p, target);
    puts("run complete");
    print_state(s);
    puts("finish test\n");
}

int START_LOOK_FOR_IDX = 0;
int FINISH_LOOK_FOR_IDX = 0;
int LOOK_FOR_STRING = 2;


int main() {
    KC_PL_DEBUG_MODE = 1;
    init_core_parsers();

    parser* fack = charP('q');
    printf("cp: %p\n", fack);

    parser* xs[] = { charP('a'), charP('b'), charP('s') };
    parser* ob; parser* cb;
    ob = charP('{');
    cb = charP('}');
    printf("{ digits } %p %p %p\n", ob, digits, cb);

    parser* ps[] = {
        fail("fuck off click clack plow"),
        anyChar,
        charP('c'),
        strP("hello world"),
        upperLowerStrP("hElLO WoRlD"),
        letter,
        letters,
        digit,
        digits,
        possibly(strP("wow")),
        whitespace,
        optionalWhitespace,
        everythingUntil(charP('0'), false),
        anyCharExcept(fack),
        choice(xs, 3),
        sequenceOf( xs, 3),
        lookAhead(strP("struct")),
        between(ob, digits, cb),
        sepBy(digits, charP(',')),
        many(charP('c')),
        regexP("[a-fA-F0-9]+"),
        regexP("^[a-zA-Z_][a-zA-Z0-9_]+")
    };

    #define rstr_type char
    FILE* tfile = fopen( "testplan.csv", "r" );
    if (tfile == NULL) {
        puts("The fucking test plan doesn't exist you dipshit");
        exit(3);
    }
    
    ALLOCATE_SN(rstr, 25, true, '\0');
    int state = START_LOOK_FOR_IDX;
    int csize = 0;
    char buffer[50];
    int rsize;

    int pidx = -1;

    while (rsize = fread(buffer, sizeof(char), 50, tfile)) {
        for (int i = 0; i < rsize; i++) {
            if (buffer[i] == '|') {
                if (state == FINISH_LOOK_FOR_IDX) {
                    APPEND(rstr, '\0')
                    pidx = str_to_int(MUTARR(rstr), SIZEOF(rstr) - 1);
                    CLEAR_SHRINK(rstr);
                    state = LOOK_FOR_STRING;
                } else if (state == LOOK_FOR_STRING) {
                    APPEND(rstr, '\0');
                    SHRINK_TO_NEEDED(rstr);
                    test(ps, pidx, MUTARR(rstr));
                    CLEAR_SHRINK(rstr);
                    state = START_LOOK_FOR_IDX;
                }
            } else if ((buffer[i] == '\n' || buffer[i] == '\r') && state == START_LOOK_FOR_IDX) {}
            else {
                APPEND(rstr, buffer[i]);
                if (state == START_LOOK_FOR_IDX) {
                    state = FINISH_LOOK_FOR_IDX;
                }
            }
        }
    }

    puts("All tests finished");
    fclose(tfile);
    #undef rstr_type
    return 0;
}
