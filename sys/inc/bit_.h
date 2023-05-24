/**
 * @file    bit_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Bit manipulation
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_bit__included
#define sys_bit__included

#include <_core_.h>

#include <type_traits_.h>

_SYS_BEGIN_NS

template <class To, class From>
    requires (sizeof(To) == sizeof(From))
        && is_trivially_copyable_v<To>
        && is_trivially_copyable_v<From>
constexpr To bit_cast(const From& from)
{
    return __builtin_bit_cast(To, from);
}

_SYS_END_NS

#endif // ifndef sys_bit__included
