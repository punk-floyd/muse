/**
 * @file    type_list_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Generic type list support
 *
 * This implementation is based on the typelist example from
 *  https://www.packtpub.com/product/template-metaprogramming-with-c/9781803243450
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_type_list__included
#define sys_type_list__included

#include <_core_.h>
#include <type_traits_.h>
#include <utility_.h>

_SYS_BEGIN_NS

/// A list of arbitrary types
template <class... Ts>
struct type_list {};

// Special type used to represent "no type" or "empty typelist". This type
// will only show up when querying an empty type list
struct null_type_list {};

// The type list namespace
namespace tl {

    constexpr size_t npos = size_t(-1);

    namespace imp {

        // sys::tl::size implementation
        template <class TL> struct size;
        template <template <class...> class TL, class... Ts>
        struct size<TL<Ts...>> {
            using type = integral_constant<size_t, sizeof...(Ts)>;
        };

        // sys::tl::sizeof_largest implementation
        template <class TL> struct sizeof_largest;
        template <template <class...> class TL, class T, class... Ts>
        struct sizeof_largest<TL<T, Ts...>> {
            using type = integral_constant<size_t, max_v(sizeof(T), sizeof(Ts)...)>;
        };
        template <template <class...> class TL>
        struct sizeof_largest<TL<>> {
            using type = integral_constant<size_t, 0>;
        };

        // sys::tl::sizeof_smallest implementation
        template <class TL> struct sizeof_smallest;
        template <template <class...> class TL, class T, class... Ts>
        struct sizeof_smallest<TL<T, Ts...>> {
            using type = integral_constant<size_t, min_v(sizeof(T), sizeof(Ts)...)>;
        };
        template <template <class...> class TL>
        struct sizeof_smallest<TL<>> {
            using type = integral_constant<size_t, 0>;
        };

        // sys::tl::contains and sys::tl::unique implementation
        template <class TL, class T> struct count_of;
        template <template <class...> class TL, class T, class T0, class... Ts>
        struct count_of<TL<T0, Ts...>, T> {
            using type = integral_constant<size_t, (is_same_v<T, T0> ? 1 : 0) + count_of<TL<Ts...>, T>::type::value>;
        };
        template <template <class...> class TL, class T>
        struct count_of<TL<>, T> {
            using type = integral_constant<size_t, 0>;
        };

        // sys::tl::find_first implementation
        template <class TL, class T, size_t I> struct find_first;
        template <template <class...> class TL, class T, size_t I, class T0, class... Ts>
        struct find_first<TL<T0, Ts...>, T, I> {
            using type = integral_constant<size_t, is_same_v<T, T0> ? I : find_first<TL<Ts...>, T, I + 1>::type::value>;
        };
        template <template <class...> class TL, class T, size_t I>
        struct find_first<TL<>, T, I> {
            using type = integral_constant<size_t, npos>;
        };

        // sys::tl::first_type implementation
        template <class TL> struct first_type;
        template <template <class...> class TL, class T, class... Ts>
        struct first_type<TL<T, Ts...>> {
            using type = T;
        };
        template <template <class...> class TL>
        struct first_type<TL<>> {
            using type = null_type_list;
        };

        // sys::tl::last_type implementation
        template <class TL> struct last_type;
        template <template <class...> class TL, class T, class... Ts>
        struct last_type<TL<T, Ts...>> {
            using type = last_type<TL<Ts...>>::type;
        };
        template <template <class...> class TL, class T>
        struct last_type<TL<T>> {
            using type = T;
        };
        template <template <class...> class TL>
        struct last_type<TL<>> {
            using type = null_type_list;
        };

        // sys::tl::type_at implementation
        template <size_t I, size_t N, class TL> struct type_at;
        template <size_t I, size_t N, template <class...> class TL, class T, class... Ts>
        struct type_at<I, N, TL<T, Ts...>> {
            using type = conditional_t<I==N, T, typename type_at<I, N+1, TL<Ts...>>::type>;
        };
        template <size_t I, size_t N>
        struct type_at<I, N, type_list<>> {
            using type = null_type_list;
        };

        // sys::tl::push_back implementation
        template <class TL, class T> struct push_back;
        template <template <class...> class TL, class T, class... Ts>
        struct push_back<TL<Ts...>, T> {
            using type = TL<Ts..., T>;
        };

        // sys::tl::push_front implementation
        template <class TL, class T> struct push_front;
        template <template <class...> class TL, class T, class... Ts>
        struct push_front<TL<Ts...>, T> {
            using type = TL<T, Ts...>;
        };
    }

    /// Obtain the number of types in the type list
    template <class TL>
    constexpr size_t size = imp::size<TL>::type::value;

    /// Determine if type list contains given type
    template <class TL, class T>
    constexpr bool contains = imp::count_of<TL, T>::type::value > 0;
    /// Determines if type list contains given type once and only once
    template <class TL, class T>
    constexpr bool unique = imp::count_of<TL, T>::type::value == 1;

    /// Evaluates to index of given type or tl::npos
    template <class TL, class T>
    constexpr size_t find_first = imp::find_first<TL, T, 0>::type::value;

    /// Obtain the size of the largest type (as determined by sizeof())
    template <class TL>
    constexpr size_t sizeof_largest = imp::sizeof_largest<TL>::type::value;
    /// Obtain the size of the smallest type (as determined by sizeof())
    template <class TL>
    constexpr size_t sizeof_smallest = imp::sizeof_smallest<TL>::type::value;

    /// Obtain the first type in a type list
    template <class TL>
    using first_type = imp::first_type<TL>::type;
    /// Obtain the last type in a type list
    template <class TL>
    using last_type = imp::last_type<TL>::type;
    /// Obtain the type at the specified index
    template <class TL, size_t I>
    using type_at = imp::type_at<I, 0, TL>::type;

    /// Append a type to a typelist
    template <class TL, class T>
    using push_back = imp::push_back<TL, T>::type;
    /// Prepend a type to a typelist
    template <class TL, class T>
    using push_front = imp::push_front<TL, T>::type;

}

_SYS_END_NS

#endif // ifndef sys_type_list__included
