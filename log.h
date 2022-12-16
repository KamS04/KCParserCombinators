#ifndef LOG_HEADER
#include "kc_config.h"

#ifdef DEBUG
#define LOG(x) KC_PL_DEBUG_MODE && x;
#else
#define LOG(x)
#endif

#endif