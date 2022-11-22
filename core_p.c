#include "string_p.h"
#include "character_p.h"
#include "util_p.h"
#include "comp_p.h"

void init_core_parsers() {
    init_core_string_parsers();
    init_core_char_parsers();
    init_core_util_parsers();
    init_comp_parsers();
}