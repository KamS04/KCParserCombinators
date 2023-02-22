#ifndef LOG_HEADER
#include "kc_config.h"

#ifdef DEBUG

#define LOG(x)  _Pragma ("GCC diagnostic push") \
                _Pragma ("GCC diagnostic ignored \"-Wunused-value\"") \
                KC_PL_DEBUG_MODE && x; \
                _Pragma ("GCC diagnostic pop")
#define HLOG(x) _Pragma ("GCC diagnostic push") \
                _Pragma ("GCC diagnostic ignored \"-Wunused-value\"") \
                KC_PL_HDEBUG_MODE && x; \
                _Pragma ("GCC diagnostic pop")
#define kfree(x)    _Pragma ("GCC diagnostic push") \
                    _Pragma ("GCC diagnostic ignored \"-Wunused-value\"") \
                    HLOG(printf("freeing %p\n", x)); free(x); \
                    _Pragma ("GCC diagnostic pop")

#else

#define LOG(x)
#define HLOG(x)
#define kfree(x) free(x)

#endif

#endif