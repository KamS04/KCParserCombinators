#ifndef LOG_HEADER
#include "kc_config.h"

#ifdef DEBUG

#define LOG(x) KC_PL_DEBUG_MODE && x;
#define HLOG(x) KC_PL_HDEBUG_MODE && x;
#define kfree(x) HLOG(printf("freeing %p\n", x)); free(x);

#else

#define LOG(x)
#define HLOG(x)
#define kfree(x) free(x)

#endif

#endif