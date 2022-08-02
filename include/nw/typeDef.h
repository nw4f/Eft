#ifndef TYPE_DEF_H_
#define TYPE_DEF_H_

#include <types.h>
#include <cafe/demo.h>
#include <cafe/gx2.h>

#include <cstddef>

#define EFT_MEMUTIL_CAFE_DCBZ_OFFSET(addr, offset)	asm("	dcbz	%0,%1" : "+g"(addr), "+g"(offset) )

#ifndef static_assert
    // https://stackoverflow.com/a/1597129
    #define TOKENPASTE(x, y) x ## y
    #define TOKENPASTE2(x, y) TOKENPASTE(x, y)

    #define static_assert(condition, ...) typedef int TOKENPASTE2(static_assert_, __LINE__)[(condition) ? 1 : -1]
#endif // static_assert

#ifndef alignof
    template<typename T>
    struct __alignment_of
    {
        char c;
        T a;
    };

    #define alignof(Type) offsetof(__alignment_of<Type>, a)
#endif // alignof

#include <nw/math.h>
#include <nw/ut/ut_Color.h>

#endif // TYPE_DEF_H_
