/**
 * @file    new.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Dynamic memory management
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <new_.h>

[[nodiscard]] void* operator new (sys::size_t count)
{
    auto p = __builtin_malloc(count);
    if (nullptr == p)
        throw sys::error_malloc();

    return p;
}

[[nodiscard]] void* operator new[] (sys::size_t count)
{
    return operator new(count);
}

[[nodiscard]] void* operator new   (sys::size_t count, const sys::no_throw_t& tag) noexcept
{
    (void)tag;
    return __builtin_malloc(count);
}

[[nodiscard]] void* operator new[] (sys::size_t count, const sys::no_throw_t& tag) noexcept
{
    return operator new(count, tag);
}
