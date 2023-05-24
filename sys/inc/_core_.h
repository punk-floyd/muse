/**
 * @file    _core_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Top level internal include for system classes
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys__core__included
#define sys__core__included

// Stuff the compiler gives us
#include <bits/types.h>
#include <bits/pthreadtypes-arch.h>

#define _SYS_BEGIN_NS       namespace sys {
#define _SYS_END_NS         }

#define SYS_INLINE          inline __attribute__((always_inline))

_SYS_BEGIN_NS

/// The type of the null pointer literal nullptr
using nullptr_t = decltype(nullptr);

/// Can store the maximum size of a theoretically possible object of any type
using size_t = __SIZE_TYPE__;
/// Signed size_t
using ssize_t = long int;

/// The result of subtracting two poiners
using ptrdiff_t = __PTRDIFF_TYPE__;

/// Size of a native mutex object
inline constexpr size_t __sizeof_native_mutex = __SIZEOF_PTHREAD_MUTEX_T;

/// Type of a native file/device
using __file_type = int;
/// Value of an invalid native file
inline constexpr __file_type __invalid_file = -1;

/// Returns true only when called during constant evaluation
constexpr inline bool is_constant_evaluated() noexcept
    { return __builtin_is_constant_evaluated(); }

/// Type of a system error code
using error_t = long int;

_SYS_END_NS

#endif // ifndef sys__core__included
