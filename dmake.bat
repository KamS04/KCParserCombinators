@echo off

gcc %* parsers.c state.c korolib.c ^
    state_default_dealloc.c util.c ^
    character_p.c comp_p.c core_p.c ^
    string_p.c util_p.c kc_config.c re.c -ggdb -DDEBUG
