/**
 * @file    utility_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   General utility classes
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_utility__included
#define sys_utility__included

#include <_core_.h>
#include <type_traits_.h>
#include <concepts_.h>

_SYS_BEGIN_NS

/// Swap the values of two objects
template <class T>
constexpr void swap(T& a, T& b) noexcept(is_nothrow_move_constructible_v<T> && is_nothrow_move_assignable_v<T>)
{
    T temp = sys::move(a);
    a      = sys::move(b);
    b      = sys::move(temp);
}

template <class T, class U>
concept is_swappable_with = requires (T& t, U& u) {
    sys::swap(t, u);
    sys::swap(u, t);
};

template <class T>
concept is_swappable = is_swappable_with<T, T>;

template <class T, class U>
concept is_nothrow_swappable_with = requires (T& t, U& u) {
    { sys::swap(t, u) } noexcept;
    { sys::swap(u, t) } noexcept;
};

template <class T>
concept is_nothrow_swappable = is_nothrow_swappable_with<T, T>;

/// Specialization for all pointers
template <>
constexpr void swap<void*> (void*& a, void*& b) noexcept
{
    void* temp = sys::move(a);
    a          = sys::move(b);
    b          = sys::move(temp);
}

/// Returns the minimum of two values
template <arithmetic T>     // Probably need to loosen this
constexpr const T& min(const T& a, const T& b)
{
    return a < b ? a : b;
}

/// Returns the maximum of two values
template <arithmetic T>
constexpr const T& max(const T& a, const T& b)
{
    return a > b ? a : b;
}

/// Returns the minimum of one or more homogeneous values
template <arithmetic T>
constexpr T min_v(T a)
{
    return a;
}
/// Returns the minimum of one or more homogeneous values
template <arithmetic T>
constexpr T min_v(T a, T b)
{
    return a < b ? a : b;
}
/// Returns the minimum of one or more homogeneous values
template <arithmetic T, class... Ts>
constexpr T min_v(T a, T b, Ts ... ts)
{
    return min_v(min_v(a,b), ts...);
}

/// Returns the maximum of one or more homogeneous values
template <arithmetic T>
constexpr T max_v(T a)
{
    return a;
}
/// Returns the maximum of one or more homogeneous values
template <arithmetic T>
constexpr T max_v(T a, T b)
{
    return a > b ? a : b;
}
/// Returns the maximum of one or more homogeneous values
template <arithmetic T, class... Ts>
constexpr T max_v(T a, T b, Ts ... ts)
{
    return max_v(max_v(a,b), ts...);
}

// - Helper types used to select different constructors in variant
template <class T> struct in_place_type_t  { explicit in_place_type_t() = default; };
template <class T> inline constexpr in_place_type_t<T> in_place_type{};

template <size_t T> struct in_place_index_t { explicit in_place_index_t() = default; };
template <size_t T> inline constexpr in_place_index_t<T> in_place_index{};

_SYS_END_NS

#endif // ifndef sys_Utility__included
