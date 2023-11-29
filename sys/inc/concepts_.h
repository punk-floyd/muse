/**
 * @file    concepts_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Core concepts
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_concepts__included
#define sys_concepts__included

#include <_core_.h>
#include <type_traits_.h>

_SYS_BEGIN_NS

/// Specifies that a type is the same as another type
template <class T, class U>
concept same_as = sys::is_same_v<T,U> && sys::is_same_v<U,T>;

/// Specifies that a type is derived from another type
template <class Derived, class Base>
concept derived_from =
    sys::is_base_of_v<Base, Derived> &&
    sys::is_convertible_v<const volatile Derived*, const volatile Base*>;

/// Specifies that a type is implicitly convertible to another type
template <class From, class To>
concept convertible_to =
    sys::is_convertible_v<From, To> &&
    requires { static_cast<To>(sys::declval<From>()); };

// MISSING: common_reference_with, common_with

/// Specified that a type is an arithmetic type
template <class T> concept arithmetic = sys::is_arithmetic_v<T>;
/// Specifies that a type is an integral type
template <class T> concept integral = sys::is_integral_v<T>;
/// Specifies that a type is a signed integral type
template <class T> concept signed_integral = sys::integral<T> && sys::is_signed_v<T>;
/// Specified that a type is an unsigned integral type
template <class T> concept unsigned_integral = sys::integral<T> && sys::is_unsigned_v<T>;
/// Specifies that a type is a floating-point type
template <class T> concept floating_point = sys::is_floating_point_v<T>;
/// Specifies that a type is a scalar type
template <class T> concept scalar = sys::is_scalar_v<T>;

// MISSING: assignable_from, swappable, swappable_with

/// Specifies that an object of the type can be destroyed
template <class T> concept destructible = sys::is_nothrow_destructible_v<T>;

/// Specifies that a variable of the type can be constructed from or bound to a set of argument types
template <class T, class... Args> concept constructible_from =
    sys::destructible<T> && sys::is_constructible_v<T, Args...>;

/// Specifies that an object of a type can be default constructed
template <class T> concept default_initializible =
    sys::constructible_from<T> &&                           // value-initialized
    requires { T{}; } &&                                    // direct-list-initialized
    requires { ::new (static_cast<void*>(nullptr)) T; };    // default-initialized

/// Specifies that an object of a type can be move constructed
template <class T> concept move_constructible =
    sys::constructible_from<T, T> && sys::convertible_to<T, T>;

/// Specifies that an object of a type can be copy constructed and move constructed
template <class T> concept copy_constructible =
    sys::move_constructible<T> &&
    sys::constructible_from<T, T&>       && sys::convertible_to<T&, T> &&
    sys::constructible_from<T, const T&> && sys::convertible_to<const T&, T> &&
    sys::constructible_from<T, const T>  && sys::convertible_to<const T, T>;

/// True if given parameter pack is not empty
template <class... Things>
concept non_empty_pack = sizeof...(Things) > 0;

/// True if given parameter pack is empty
template <class... Things>
concept empty_pack = sizeof...(Things) == 0;

/// Specifies that a type can be used in Boolean contexts
template <class B>
concept boolean_testable =
    convertible_to<B, bool> &&
    requires (B&& b) { { !sys::forward<B>(b) } -> convertible_to<bool>; };

/// Specifies that type T is comparable to type U
template <class T, class U>
concept half_equality_comparable =
    requires (const remove_reference_t<T>& t, const remove_reference_t<U>& u) {
        { (t == u) } -> boolean_testable;
        { (t != u) } -> boolean_testable;
    };

template <class T, class U>
concept equality_comparable =
    half_equality_comparable<T,U> &&
    half_equality_comparable<U,T>;

template <class T, class U>
concept partially_ordered_with =
    requires (const remove_reference_t<T>& t, const remove_reference_t<U>& u) {
        { (t <  u) } -> boolean_testable;
        { (u <  t) } -> boolean_testable;
        { (t <= u) } -> boolean_testable;
        { (u <= t) } -> boolean_testable;
        { (t >  u) } -> boolean_testable;
        { (u >  t) } -> boolean_testable;
        { (t >= u) } -> boolean_testable;
        { (u >= t) } -> boolean_testable;
    };

_SYS_END_NS

#endif // ifndef sys_concepts__included
