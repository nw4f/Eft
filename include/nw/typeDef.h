#ifdef INCLUDE_CAFE
#include <cafe.h>
#else
#include <types.h>
#endif // INCLUDE_CAFE

#ifndef TYPE_DEF_H_
#define TYPE_DEF_H_

// https://stackoverflow.com/a/1597129
#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#define static_assert(condition, ...) typedef int TOKENPASTE2(static_assert_, __LINE__)[(condition) ? 1 : -1]

#include <cstddef>

template<typename T>
struct __alignment_of
{
    char c;
    T a;
};

#define alignof(Type) offsetof(__alignment_of<Type>, a)

#endif // TYPE_DEF_H_
