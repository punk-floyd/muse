/**
 * @file    allocator_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   The standard allocator
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_allocator__included
#define sys_allocator__included

#include <_core_.h>
#include <bit_.h>

_SYS_BEGIN_NS

template <class T>
class allocator
{
public:

    using value_type = T;

    constexpr allocator() = default;

    /// Allocate N items *without* initialization
    [[nodiscard]] constexpr value_type* allocate(sys::size_t n)
    {
        auto p = ::operator new(n * sizeof(value_type));
        return sys::bit_cast<value_type*>(p);
    }

    constexpr void deallocate(value_type* p)
    {
        ::operator delete(p);
    }
};

_SYS_END_NS

#endif // ifndef sys_allocator__included
