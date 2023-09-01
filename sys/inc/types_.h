/**
 * @file    types_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Basic system types
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_SysTypes__included
#define sys_SysTypes__included

#include <_core_.h>

_SYS_BEGIN_NS

using uint8_t  = __uint8_t;
using sint8_t  = __int8_t;

using uint16_t = __uint16_t;
using sint16_t = __int16_t;

using uint32_t = __uint32_t;
using sint32_t = __int32_t;

using uint64_t = __uint64_t;
using sint64_t = __int64_t;

using uint128_t = unsigned __int128;
using sint128_t = __int128;

using uintmax_t = __uintmax_t;
using sintmax_t = __intmax_t;

using uintptr_t = unsigned long;
using sintptr_t = long;

using time_t = __time_t;

_SYS_END_NS

#endif // ifndef sys_SysTypes__included
