
#ifndef UTIL_H_
#define UTIL_H_

#include <assert.h>

typedef unsigned char u8_t;
typedef unsigned u32_t;

template <typename _Tp>
inline const _Tp &
min(const _Tp &__a, const _Tp &__b)
{
    return (__a < __b) ? __a : __b;
}

template <typename _Tp>
inline const _Tp &
max(const _Tp &__a, const _Tp &__b)
{
    return (__a > __b) ? __a : __b;
}
// min(1, 2);
#endif