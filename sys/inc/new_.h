/**
 * @file    new_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Dynamic memory management
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_new__included
#define sys_new__included

#include <_core_.h>
#include <error_.h>

_SYS_BEGIN_NS

struct no_throw_t {
    no_throw_t() noexcept = default;
};

_SYS_END_NS

// - Main allocation routines
[[nodiscard]] void* operator new   (sys::size_t count);
[[nodiscard]] void* operator new[] (sys::size_t count);

// - Non-throwing allocation routines
[[nodiscard]] void* operator new   (sys::size_t count, const sys::no_throw_t& tag) noexcept;
[[nodiscard]] void* operator new[] (sys::size_t count, const sys::no_throw_t& tag) noexcept;

// - Non-allocating placement allocation routines
[[nodiscard]] inline void* operator new   (sys::size_t count, void* dst) noexcept
{
    return dst;
}
[[nodiscard]] inline void* operator new[] (sys::size_t count, void* dst) noexcept
{
    return dst;
}

/// Pointer optimization barrier
template <class T>
[[nodiscard]] constexpr T* launder(T* p) noexcept
    { return __builtin_launder(p); }

// Program is ill-formed if T is a function type or (possibly cv-qualified) void

template <class Ret, class... Args>
void launder(Ret(*)(Args...)) = delete;

void launder(void*) = delete;
void launder(const void*) = delete;
void launder(volatile void*) = delete;
void launder(const volatile void*) = delete;

#endif // ifndef sys_new__included
