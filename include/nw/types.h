#ifndef NW_TYPES_H_
#define NW_TYPES_H_

#include <cstddef>

#define NW_PLATFORM_CAFE

#if !defined(NW_DEBUG) && !defined(NW_DEVELOP) && !defined(NW_RELEASE)
    #error "Build target is not defined."
#endif

#define NW_INLINE inline
#if defined(NW_DEBUG)
    #define NW_FORCE_INLINE     inline
#else
    #define NW_FORCE_INLINE     __attribute__((always_inline)) inline
#endif

#define NW_CONFIG_USE_STATIC
#define NW_TABLE_FIELD static const

#endif // NW_TYPES_H_
