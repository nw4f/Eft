#ifndef TYPE_DEF_H_
#define TYPE_DEF_H_

#include <nw/types.h>

#ifdef _WIN32
    #include <GL/glew.h>
#endif // _WIN32

#if defined(_WIN32) || defined(__WUT__)
    #include <cafe/gfd.h>
    #include <cafe.h>
#else
    #include <types.h>
    #include <cafe/demo.h>
    #include <cafe/gx2.h>
#endif

#ifdef _WIN32
    #define EFT_IS_WIN                  1
    #define EFT_IS_CAFE                 0
    #define EFT_IS_CAFE_WUT             0

    #define EFT_MEMUTIL_CAFE_DCBZ_OFFSET(addr, offset)	((void)offset)
#else
    #define EFT_IS_WIN                  0
    #define EFT_IS_CAFE                 1

    #ifdef __WUT__
        #define EFT_IS_CAFE_WUT         1
    #else
        #define EFT_IS_CAFE_WUT         0
    #endif

    #define EFT_MEMUTIL_CAFE_DCBZ_OFFSET(addr, offset)	asm("	dcbz	%0,%1" : "+g"(addr), "+g"(offset) )
#endif

#if EFT_IS_WIN
    #if (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
        #error "Windows host expected to be little-endian"
    #endif
#endif // EFT_IS_WIN

#if (defined(__cplusplus) && __cplusplus < 201103L) || (!defined(__cplusplus) && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L))
    #ifndef static_assert
        // https://stackoverflow.com/a/1597129
        #define TOKENPASTE(x, y) x ## y
        #define TOKENPASTE2(x, y) TOKENPASTE(x, y)
        #define static_assert(condition, ...) typedef int TOKENPASTE2(static_assert_, __LINE__)[(condition) ? 1 : -1]
    #endif // static_assert
#else
    #include <assert.h>
#endif

#include <nw/math.h>
#include <nw/ut/ut_Color.h>

#endif // TYPE_DEF_H_
