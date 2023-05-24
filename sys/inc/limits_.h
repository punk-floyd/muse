/**
 * @file    limits_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Numeric limits
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_limits__included
#define sys_limits__included

#include <_core_.h>
#include <concepts_.h>

_SYS_BEGIN_NS

template <class T>
struct numeric_limits
{
    static constexpr bool   is_signed = false;
    static constexpr bool   is_unsigned = false;
    static constexpr bool   is_integral = false;
    static constexpr bool   is_exact = false;
    static constexpr int    radix= 0;
    static constexpr size_t bits = 0;

    static constexpr T min = T();
    static constexpr T max = T();
};

/// Returns value with MSB set
template <integral T>
constexpr inline remove_cv_t<T> msb()
{
    return T(T{1} << ((sizeof(T) << 3) - 1));
}

template <sys::integral T>
struct numeric_limits<T>
{
    static constexpr bool is_signed     = is_signed_v<T>;
    static constexpr bool is_unsigned   = is_unsigned_v<T>;
    static constexpr bool is_integral   = true;
    static constexpr bool is_exact      = false;
    static constexpr int  radix         = 2;
    static constexpr size_t bits        = sizeof(T) * 8;

    // Note: This implementation is assuming:
    //  - All bits of the type are used. This assumption might not hold for
    //    types like int_least* or int_fast* but I don't use those.
    //  - signed values use two's complement (c++20 requires this)

    static constexpr T min = is_unsigned_v<T> ? 0 : msb<T>();
    static constexpr T max = is_unsigned_v<T> ? T(-1) : (T(-1) & ~msb<T>());
};

/**
 * @brief   Multiply two values and store result, detecting overflow
 *
 * @param a         The multiplier
 * @param b         The multiplicand
 * @param result    The product (a * b)
 *
 * @return Returns true if operation overflowed/underflowed or false if not
 */
template <integral T, integral U, integral V>
constexpr bool multiply_overflow(T a, U b, V& result)
{
    return __builtin_mul_overflow(a, b, &result);
}

/**
 * @brief Add two values and store result, detecting overflow
 *
 * @param a         The first addend
 * @param b         The second addend
 * @param result    The sum (a + b)
 *
 * @return Returns true if operation overflowed/underflowed or false if not
 */
template <integral T, integral U, integral V>
constexpr bool add_overflow(T a, U b, V& result)
{
    return __builtin_add_overflow(a, b, &result);
}

/**
 * @brief Subtract two values and store result, detecting overflow
 *
 * @param a         The minuend
 * @param b         The subtrahend
 * @param result    The difference (a - b)
 *
 * @return Returns true if operation overflowed/underflowed or false if not
 */
template <integral T, integral U, integral V>
constexpr bool subtract_overflow(T a, U b, V& result)
{
    return __builtin_sub_overflow(a, b, &result);
}

_SYS_END_NS

#endif // ifndef sys_limits__included
