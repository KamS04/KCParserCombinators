#include<stdlib.h>
#include<string.h>

// specify nullable alloc
// specific null alloc
// normal alloc

#define MUTARR(x) _##x##_arr
#define NULX(x) __##x##nul
#define ARR_SIZE(x) _##x##_as
#define CARR_SIZE(x) _##x##_cal
#define T_ARR(x) _##x##_tarr
#define SSIZE(x) _##x##ss
#define NSIZE(x) _##x##ns
#define ATYPE(x) x##_type
#define TINT(x) _##x##t_int
#define SNULL(x) _##x##set_null
#define NULLIFY(x) NULLIFY_S(x, 0)
#define NULLIFY_S(x, st) if (SNULL(x)) { \
                            for (TINT(x) = st; TINT(x) < ARR_SIZE(x); TINT(x)++) { \
                                MUTARR(x)[TINT(x)] = NULX(x); \
                            } \
                        }
// identifier, startsize, setnull, null
#define ALLOCATE(x, ss) ALLOCATE_SN(x, ss, 0, 0)
#define ALLOCATE_S(x, ss, sn) ALLOCATE_SN(x, ss, sn, NULL)
#define ALLOCATE_SN(x, ss, sn, n) int ARR_SIZE(x) = ss; \
                        int SSIZE(x) = ss; \
                        int CARR_SIZE(x) = 0; \
                        int NSIZE(x); \
                        int TINT(x); \
                        int SNULL(x) = sn; \
                        ATYPE(x) NULX(x) = n; \
                        ATYPE(x)* T_ARR(x); \
                        ATYPE(x)* MUTARR(x) = malloc( ss * sizeof(ATYPE(x))); \
                        NULLIFY(x);
#define CHECK_REALLOC_NEEDED(x) CARR_SIZE(x) == ARR_SIZE(x)
#define CHECK_SP_REALLOC_NEEDED(x, n) CARR_SIZE(x) + n >= ARR_SIZE(x)
#define REALLOCATE(x, nsi) NSIZE(x) = ARR_SIZE(x) + nsi; \
                        ATYPE(x)* T_ARR(x) = malloc( NSIZE(x) * sizeof(ATYPE(x))); \
                        memcpy(T_ARR(x), MUTARR(x), CARR_SIZE(x) * sizeof(ATYPE(x))); \
                        free(MUTARR(x)); \
                        MUTARR(x) = T_ARR(x); \
                        ARR_SIZE(x) = NSIZE(x); \
                        T_ARR(x) = NULL; \
                        NULLIFY_S(x, CARR_SIZE(x)+1);
#define REALLOCATE_NMIN(x, nmsi) T_INT(x) = (CARR_SIZE(x) + nmsi) - ARR_SIZE; \
                                T_INT(x) += SSIZE(x) - (T_INT % SSIZE(x)); \
                                REALLOCATE(x, T_INT(x))
#define APPEND(x, val) if (CHECK_REALLOC_NEEDED(x)) {\
                            REALLOCATE(x, SSIZE(x)); \
                        } \
                        MUTARR(x)[CARR_SIZE(x)] = val; \
                        CARR_SIZE(x) += 1;
#define SHRINK_TO_NEEDED(x) if (CARR_SIZE(x) < ARR_SIZE(x)) { \
                                T_ARR(x) = malloc(CARR_SIZE(x) * sizeof(ATYPE(x))); \
                                memcpy(T_ARR(x), MUTARR(x), CARR_SIZE(x) * sizeof(ATYPE(x))); \
                                free(MUTARR(x)); \
                                MUTARR(x) = T_ARR(x); \
                            }
#define CLEAR(x) NULLIFY(x); \
                CARR_SIZE(x) = 0;
#define CLEAR_SHRINK(x) T_ARR(x) = malloc( SSIZE(x) * sizeof(ATYPE(x)*) ); \
                        free(MUTARR(x)); \
                        MUTARR(x) = T_ARR(x); \
                        T_ARR(x) = NULL; \
                        ARR_SIZE(x) = SSIZE(x); \
                        CARR_SIZE(x) = 0; \
                        NULLIFY(x);
#define SIZEOF(x) CARR_SIZE(x)
