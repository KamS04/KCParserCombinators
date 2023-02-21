@echo off

gcc -c parsers.c state.c state_default_dealloc.c util.c ^
    character_p.c comp_p.c core_p.c string_p.c util_p.c ^
    kc_config.c kre.c ^
    -DDEBUG -ggdb

ar rcs %* parsers.o state.o state_default_dealloc.o ^
    util.o character_p.o comp_p.o core_p.o string_p.o ^
    util_p.o kc_config.o kre.o

del *.o
