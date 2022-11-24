@echo off

gcc -c parsers.h parsers.c state.h state.c corolib.h corolib.c ^
    state_default_dealloc.h state_default_dealloc.c util.h util.c ^
    character_p.h character_p.c comp_p.h comp_p.c core_p.h core_p.c ^
    mutarr.h string_p.h string_p.c util_p.h util_p.c

ar rcs %* parsers.o state.o corolib.o state_default_dealloc.o ^
                    util.o character_p.o comp_p.o core_p.o string_p.o ^
                    util_p.o
del *.o
del *.gch