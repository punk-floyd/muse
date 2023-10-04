/**
 * @file    type_traits_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Type traits for metaprogramming
 *
 * Be forewarned. Some of these lines run a little long.
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_TypeTraits__included
#define sys_TypeTraits__included

#include <_core_.h>

_SYS_BEGIN_NS

/// Wrapper for an integral constant; base class for many type traits
template <class T, T v>
struct integral_constant {

    using value_type = T;
    using type       = integral_constant;

    static constexpr T value = v;

    constexpr operator value_type()   const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};

/// Base for many of the is_* types
template <bool B> using bool_constant = integral_constant<bool, B>;
using true_type  = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

namespace imp {
    template <bool B, class T, class F> struct conditional              { using type = T; };
    template <        class T, class F> struct conditional<false, T, F> { using type = F; };
    template <bool B, class T = void>   struct enable_if {};
    template <        class T>          struct enable_if<true, T> { using type = T; };
}

// Conditionally removes a function overload or template specialization from overload resolution
template <bool B, class T = void> using enable_if_t = typename imp::enable_if<B,T>::type;
/// Choose one type or another based on compile-time boolean
template <bool B, class T, class F> using conditional_t = typename imp::conditional<B,T,F>::type;

// v----- These guys come from the compiler itself -----v
// https://gcc.gnu.org/onlinedocs/gcc-4.7.4/gcc/Type-Traits.html

namespace imp {
    template <class T> struct is_union               : bool_constant<__is_union(T)> {};
    template <class T> struct is_trivial             : bool_constant<__is_trivial(T)> {};
    template <class T> struct is_standard_layout     : bool_constant<__is_standard_layout(T)> {};
    template <class T> struct is_polymorphic         : bool_constant<__is_polymorphic(T)> {};
    template <class T> struct is_pod                 : bool_constant<__is_pod(T)> {};
    template <class T> struct is_final               : bool_constant<__is_final(T)> {};
    template <class T> struct is_enum                : bool_constant<__is_enum(T)> {};
    template <class T> struct is_empty               : bool_constant<__is_empty(T)> {};
    template <class T> struct is_class               : bool_constant<__is_class(T)> {};
    template <class T> struct is_aggregate           : bool_constant<__is_aggregate(T)> {};
    template <class T> struct is_abstract            : bool_constant<__is_abstract(T)> {};
    template <class T> struct has_virtual_destructor : bool_constant<__has_virtual_destructor(T)> {};
    template <class T> struct is_trivially_copyable  : bool_constant<__has_trivial_copy(T)> {};

    template <class T>
        requires is_enum<T>::value
    struct underlying_type { using type = __underlying_type(T); };

    template <class Base, class Derived> struct is_base_of :
        bool_constant<is_class<Base>::value && is_class<Derived>::value && __is_base_of(Base, Derived)> {};

    /// Checks if type has a trivial destructor
    template <class T> struct has_trivial_destructor : bool_constant<__has_trivial_destructor(T)> {};
    template <class T> inline constexpr bool has_trivial_destructor_v = has_trivial_destructor<T>::value;

    template <class T, class... Args> struct is_constructible__imp           : bool_constant<__is_constructible(T, Args...)> {};
    template <class T, class U>       struct is_assignable__imp              : bool_constant<__is_assignable(T, U)> {};
    template <class T, class... Args> struct is_trivially_constructible__imp : bool_constant<__is_trivially_constructible(T, Args...)> {};
    template <class T, class U>       struct is_trivially_assignable__imp    : bool_constant<__is_trivially_assignable(T, U)> {};
    template <class T, class... Args> struct is_nothrow_constructible__imp   : bool_constant<__is_nothrow_constructible(T, Args...)> {};
    template <class T, class U>       struct is_nothrow_assignable__imp      : bool_constant<__is_nothrow_assignable(T, U)> {};
}

// ^----- These guys come from the compiler itself -----^

/// Checks if type is a union
template <class T> inline constexpr bool is_union_v = imp::is_union<T>::value;
/// Checks if type is trivial
template <class T> inline constexpr bool is_trivial_v = imp::is_trivial<T>::value;
/// Checks if type has standard layout
template <class T> inline constexpr bool is_standard_layout_v = imp::is_standard_layout<T>::value;
/// Checks if type is polymorphic class type
template <class T> inline constexpr bool is_polymorphic_v = imp::is_polymorphic<T>::value;
/// Checks if type is a plain old data type
template <class T> inline constexpr bool is_pod_v = imp::is_pod<T>::value;
/// Checks if type is final
template <class T> inline constexpr bool is_final_v = imp::is_final<T>::value;
/// Checks if type is an enumeration
template <class T> inline constexpr bool is_enum_v = imp::is_enum<T>::value;
/// Returns underlying type of an enumeration
template <class T> using underlying_type_t = typename imp::underlying_type<T>::type;
/// Checks if type is empty
template <class T> inline constexpr bool is_empty_v = imp::is_empty<T>::value;
/// Checks if type is a class or struct
template <class T> inline constexpr bool is_class_v = imp::is_class<T>::value;
/// Checks if Base is a base class of Derived
template <class Base, class Derived> inline constexpr bool is_base_of_v = imp::is_base_of<Base, Derived>::value;
/// Checks if type is aggregate
template <class T> inline constexpr bool is_aggregate_v = imp::is_aggregate<T>::value;
/// Checks if a type is abstract
template <class T> inline constexpr bool is_abstract_v = imp::is_abstract<T>::value;
/// Checks if a class has virtual destructor
template <class T> inline constexpr bool has_virtual_destructor_v = imp::has_virtual_destructor<T>::value;
/// Checks if a type is trivially copyable
template <class T> inline constexpr bool is_trivially_copyable_v = imp::is_trivially_copyable<T>::value;

namespace imp {
    template <class T> struct type_identity { using type = T; };
    template <class T> struct remove_cv                         { using type = T; };
    template <class T> struct remove_cv<const T>                { using type = T; };
    template <class T> struct remove_cv<const volatile T>       { using type = T; };
    template <class T> struct remove_cv<volatile T>             { using type = T; };
    template <class T> struct remove_const                      { using type = T; };
    template <class T> struct remove_const<const T>             { using type = T; };
    template <class T> struct remove_volatile                   { using type = T; };
    template <class T> struct remove_volatile<volatile T>       { using type = T; };
    template <class T> struct remove_pointer                    { using type = T; };
    template <class T> struct remove_pointer<T*>                { using type = T; };
    template <class T> struct remove_pointer<T* const>          { using type = T; };
    template <class T> struct remove_pointer<T* volatile>       { using type = T; };
    template <class T> struct remove_pointer<T* const volatile> { using type = T; };
    template <class T> struct remove_reference                  { using type = T; };
    template <class T> struct remove_reference<T&>              { using type = T; };
    template <class T> struct remove_reference<T&&>             { using type = T; };
}

/// Returns the type argument unchanged
template <class T> using  type_identity_t = typename imp::type_identity<T>::type;
/// Removes const and/or volatile specifier from given type
template <class T> using  remove_cv_t = typename imp::remove_cv<T>::type;
/// Removes const specifier from given type
template <class T> using  remove_const_t = typename imp::remove_const<T>::type;
/// Removes volatile specifier from given type
template <class T> using  remove_volatile_t = typename imp::remove_volatile<T>::type;
// Removes a pointer from the given type
template <class T> using  remove_pointer_t = typename imp::remove_pointer<T>::type;
// Removes a reference from the given type
template <class T> using  remove_reference_t = typename imp::remove_reference<T>::type;

namespace imp {
    template <class T>                     struct remove_cvref { using type = remove_cv_t<remove_reference_t<T>>; };
    template <class T>                     struct remove_extent            { using type = T; };
    template <class T>                     struct remove_extent<T[]>       { using type = T; };
    template <class T, size_t N>           struct remove_extent<T[N]>      { using type = T; };
    template <class T>                     struct remove_all_extents       { using type = T; };
    template <class T>                     struct remove_all_extents<T[]>  { using type = remove_all_extents<T>::type; };
    template <class T, size_t N>           struct remove_all_extents<T[N]> { using type = remove_all_extents<T>::type; };
    template <class T>                     struct rank       : integral_constant<size_t, 0> {};
    template <class T>                     struct rank<T[]>  : integral_constant<size_t, 1 + rank<T>::value> {};
    template <class T, size_t N>           struct rank<T[N]> : integral_constant<size_t, 1 + rank<T>::value> {};
    template <class T, size_t N = 0>       struct extent          : integral_constant<size_t, 0> {};
    template <class T>                     struct extent<T[], 0>  : integral_constant<size_t, 0> {};
    template <class T, size_t N>           struct extent<T[], N>  : extent<T, N-1> {};
    template <class T, size_t I>           struct extent<T[I], 0> : integral_constant<size_t, I> {};
    template <class T, size_t I, size_t N> struct extent<T[I], N> : extent<T, N-1> {};
}
/// remove_cv + remove_reference
template <class T> using  remove_cvref_t = typename imp::remove_cvref<T>::type;
/// Removes one extent from the given array type
template <class T> using remove_extent_t = typename imp::remove_extent<T>::type;
/// Removes all extents from the given array type
template <class T> using remove_all_extents_t = typename imp::remove_all_extents<T>::type;
/// Obtains the number of dimensions of an array type
template <class T> inline constexpr size_t rank_v = imp::rank<T>::value;
/// Obtains the size of an array type along a specified dimension
template <class T, size_t N = 0> inline constexpr size_t extent_v = imp::extent<T, N>::value;

namespace imp {
    // Helper for add_lvalue_reference
    template <class T> auto try_add_lval_ref(int) -> type_identity<T&>;
    template <class T> auto try_add_lval_ref(...) -> type_identity<T>;
    // Helper for add_rvalue_reference
    template <class T> auto try_add_rval_ref(int) -> type_identity<T&&>;
    template <class T> auto try_add_rval_ref(...) -> type_identity<T>;
    // Helper for add_pointer
    template <class T> auto try_add_ptr(int) -> type_identity<remove_reference_t<T>*>;
    template <class T> auto try_add_ptr(...) -> type_identity<T>;

    template <class T> struct add_pointer           : decltype(try_add_ptr<T>(0)) {};
    template <class T> struct add_lvalue_reference  : decltype(try_add_lval_ref<T>(0)) {};
    template <class T> struct add_rvalue_reference  : decltype(try_add_rval_ref<T>(0)) {};
    template <class T> struct add_cv { using type = const volatile T; };
    template <class T> struct add_const { using type = const T; };
    template <class T> struct add_volatile { using type = volatile T; };
}

/// Add pointer to a given type
template <class T> using  add_pointer_t = typename imp::add_pointer<T>::type;
/// Add lvalue reference to the given type
template <class T> using  add_lvalue_reference_t = typename imp::add_lvalue_reference<T>::type;
/// Add rvalue reference to the given type
template <class T> using  add_rvalue_reference_t = typename imp::add_rvalue_reference<T>::type;
/// Add const and volatile to the type
template <class T> using  add_cv_t = typename imp::add_cv<T>::type;
/// Add const to the type
template <class T> using  add_const_t = typename imp::add_const<T>::type;
/// Add volatile to the type
template <class T> using  add_volatile_t = typename imp::add_volatile<T>::type;

namespace imp {
    /// Copy top level cv-qualifiers from another type
    template <class From, class To> struct copy_cv                          { using type = To; };
    template <class From, class To> struct copy_cv<const volatile From, To> { using type = add_cv_t<To>; };
    template <class From, class To> struct copy_cv<      volatile From, To> { using type = add_volatile_t<To>; };
    template <class From, class To> struct copy_cv<const          From, To> { using type = add_const_t<To>; };
}

/// Copy top level cv-qualifiers from another type
template <class From, class To> using copy_cv_t = typename imp::copy_cv<remove_reference_t<From>, To>::type;

/// Obtains an rvalue reference
template <class T>
[[nodiscard]] constexpr remove_reference_t<T>&& move(T&& t) noexcept
    { return static_cast<remove_reference_t<T>&&>(t); }

/// Forwards a function argument
template <class T>
[[nodiscard]] constexpr T&& forward(remove_reference_t<T>& t) noexcept
    { return static_cast<T&&>(t); }
template <class T>
[[nodiscard]] constexpr T&& forward(remove_reference_t<T>&& t) noexcept
    { return static_cast<T&&>(t); }

/// Obtains a reference to its argument for use in unevaluated context
template <class T> add_rvalue_reference_t<T> declval() noexcept;

namespace imp {
    template <class T, class U> struct is_same       : false_type {};
    template <class T>          struct is_same<T, T> : true_type  {};
    template <class T> struct is_const          : false_type {};
    template <class T> struct is_const<const T> : true_type  {};
    template <class T> struct is_volatile             : false_type {};
    template <class T> struct is_volatile<volatile T> : true_type  {};
    template <class T>           struct is_array       : false_type {};
    template <class T>           struct is_array<T[]>  : true_type  {};
    template <class T, size_t N> struct is_array<T[N]> : true_type  {};
    template <class T> struct is_unbounded_array      : false_type {};
    template <class T> struct is_unbounded_array<T[]> : true_type  {};
    template <class T>           struct is_bounded_array       : false_type {};
    template <class T, size_t N> struct is_bounded_array<T[N]> : true_type  {};
    template <class T> struct is_pointer                    : false_type {};
    template <class T> struct is_pointer<T*>                : true_type  {};
    template <class T> struct is_pointer<T* const>          : true_type  {};
    template <class T> struct is_pointer<T* volatile>       : true_type  {};
    template <class T> struct is_pointer<T* const volatile> : true_type  {};
    template <class T> struct is_reference      : false_type {};
    template <class T> struct is_reference<T&>  : true_type  {};
    template <class T> struct is_reference<T&&> : true_type  {};
    template <class T> struct is_lvalue_reference     : false_type {};
    template <class T> struct is_lvalue_reference<T&> : true_type  {};
    template <class T> struct is_rvalue_reference      : false_type {};
    template <class T> struct is_rvalue_reference<T&&> : true_type  {};

    // Only function types and reference types can't be const qualified
    template <class T> struct is_function : bool_constant<!is_const<const T>::value && !is_reference<T>::value> {};

    template <class T> struct is_nullptr : is_same<nullptr_t, remove_cv_t<T>> {};
}

/// Checks if two types are the same
template <class T, class U> inline constexpr bool is_same_v = imp::is_same<T,U>::value;
/// Checks if a type is const-qualified
template <class T> inline constexpr bool is_const_v = imp::is_const<T>::value;
/// Checks if a type is volatile-qualified
template <class T> inline constexpr bool is_volatile_v = imp::is_volatile<T>::value;
/// Checks if type is an array
template <class T> inline constexpr bool is_array_v = imp::is_array<T>::value;
/// Checks if type is an unbound array
template <class T> inline constexpr bool is_unbounded_array_v = imp::is_unbounded_array<T>::value;
/// Checks if type is a bound array
template <class T> inline constexpr bool is_bounded_array_v = imp::is_bounded_array<T>::value;
/// Checks if type is a pointer
template <class T> inline constexpr bool is_pointer_v = imp::is_pointer<T>::value;
/// Checks if type is a reference
template <class T> inline constexpr bool is_reference_v = imp::is_reference<T>::value;
/// Checks if type is an lvalue reference
template <class T> inline constexpr bool is_lvalue_reference_v = imp::is_lvalue_reference<T>::value;
/// Checks if type is an rvalue reference
template <class T> inline constexpr bool is_rvalue_reference_v = imp::is_rvalue_reference<T>::value;
/// Checks if type is a function
template <class T> inline constexpr bool is_function_v = imp::is_function<T>::value;
/// Checks if type is a nullptr_t
template <class T> inline constexpr bool is_nullptr_v = imp::is_nullptr<T>::value;

namespace imp {
    template <class T>          struct is_member_fun_ptr_imp         : false_type {};
    template <class T, class U> struct is_member_fun_ptr_imp<T U::*> : is_function<T> {};
    template <class T> inline constexpr bool is_member_fun_ptr_imp_v = is_member_fun_ptr_imp<T>::value;

    template <class T>          struct is_member_ptr_imp         : false_type {};
    template <class T, class U> struct is_member_ptr_imp<T U::*> : true_type {};
    template <class T> inline constexpr bool is_member_ptr_imp_v = is_member_ptr_imp<T>::value;

    template <class T> struct is_member_pointer          : is_member_ptr_imp<remove_cv_t<T>> {};
    template <class T> struct is_member_function_pointer : is_member_fun_ptr_imp<remove_cv_t<T>> {};
    template <class T> struct is_member_object_pointer :
        bool_constant<is_member_pointer<T>::value && !is_member_function_pointer<T>::value> {};
}

/// Checks if type is a member pointer (object or function)
template <class T> inline constexpr bool is_member_pointer_v = imp::is_member_pointer<T>::value;
/// Checks if type is a member function pointer
template <class T> inline constexpr bool is_member_function_pointer_v = imp::is_member_function_pointer<T>::value;
/// Checks if type is a member object pointer
template <class T> inline constexpr bool is_member_object_pointer_v = imp::is_member_object_pointer<T>::value;

// Credit for disjunction and conjunction implementations (and theory):
//  https://www.fluentcpp.com/2021/04/30/how-to-implement-stdconjunction-and-stddisjunction-in-c11/

namespace imp {
    template <bool...> struct bool_pack {};
    template <bool... Bs> using conjunction = is_same<bool_pack<true,Bs...>, bool_pack<Bs...,true>>;
    //   - If at least one is true, then it is false that all of them are false
    template <bool... Bs> struct disjunction : bool_constant<!conjunction<!Bs...>::value> {};
    template <class T> struct negation : bool_constant<!bool(T::value)> {};
}

/// Variadic logical 'and' metafunction
template <bool... Bs> inline constexpr bool conjunction_v = imp::conjunction<Bs...>::value;
// Variadic logical 'or' metafunction
template <bool... Bs> inline constexpr bool disjunction_v = imp::disjunction<Bs...>::value;
/// Logical NOT metafunction
template <class T> inline constexpr bool negation_v = imp::negation<T>::value;

/// Checks if type T is any type in Types
template <class T, class... Types>
inline constexpr bool type_is_any_of_v = disjunction_v<is_same_v<T, Types>...>;

namespace imp {
    template <class T> struct is_void : is_same<void, remove_cv_t<T>> {};
    template <class T> struct is_floating_point : bool_constant<type_is_any_of_v<remove_cv_t<T>,
        float, double, long double>> {};
    template <class T> struct is_integral : bool_constant<type_is_any_of_v<remove_cv_t<T>,
        bool, char32_t, char16_t, char8_t, wchar_t, char, signed char, unsigned char,
                 short,          int,          long,          long long,
        unsigned short, unsigned int, unsigned long, unsigned long long,
        __int128, unsigned __int128, __intptr_t, unsigned __intptr_t>> {};
}

/// Checks if type is void
template <class T> inline constexpr bool is_void_v = imp::is_void<T>::value;
/// Checks if type is floating-point (float, double, long double)
template <class T> inline constexpr bool is_floating_point_v = imp::is_floating_point<T>::value;
// Checks if type is an integral type
template <class T> inline constexpr bool is_integral_v = imp::is_integral<T>::value;

namespace imp {
    template <class T> struct is_arithmetic  : bool_constant<is_integral_v<T> || is_floating_point_v<T>> {};
    template <class T> struct is_fundamental : bool_constant<is_arithmetic<T>::value || is_void_v<T> || is_nullptr_v<T>> {};
    template <class T> struct is_compound    : bool_constant<!is_fundamental<T>::value> {};
    template <class T> struct is_scalar :
        bool_constant<is_arithmetic<T>::value || is_pointer_v<T> ||
                    is_member_pointer_v<T> || is_enum_v<T> || is_nullptr_v<T>> {};
    template <class T> struct is_object :
        bool_constant<!is_function_v<T> && !is_reference_v<T> && !is_void_v<T>> {};
}

/// Checks if type is an arithmetic (integral or floating-point) type
template <class T> inline constexpr bool is_arithmetic_v = imp::is_arithmetic<T>::value;
/// Checks if type is fundamental (arithmetic, void, or nullptr_t)
template <class T> inline constexpr bool is_fundamental_v = imp::is_fundamental<T>::value;
/// Checks if type is compound (complement to fundamental)
template <class T> inline constexpr bool is_compound_v = imp::is_compound<T>::value;
/// Checks if type is scalar (arithmetic, enum, pointers, pointer-to-members, nullptr_t)
template <class T> inline constexpr bool is_scalar_v = imp::is_scalar<T>::value;
/// Checks if type is an object (not a function, reference, or void)
template <class T> inline constexpr bool is_object_v = imp::is_object<T>::value;

namespace imp {
    template <class T, bool = is_arithmetic<T>::value> struct is_signed__imp           : bool_constant<T(-1) < T(0)> {};
    template <class T>                                 struct is_signed__imp<T, false> : false_type {};
    template <class T> using is_signed__imp_t = is_signed__imp<T>::type;

    template <class T, bool = is_arithmetic<T>::value> struct is_unsigned__imp           : bool_constant<T(0) < T(-1)> {};
    template <class T>                                 struct is_unsigned__imp<T, false> : false_type {};
    template <class T> using is_unsigned__imp_t = is_unsigned__imp<T>::type;

    template <class T> struct is_signed   : imp::is_signed__imp_t<T> {};
    template <class T> struct is_unsigned : imp::is_unsigned__imp_t<T> {};
}

/// Checks if type is signed
template <class T> inline constexpr bool is_signed_v = imp::is_signed<T>::value;
/// Checks if type is unsigned
template <class T> inline constexpr bool is_unsigned_v = imp::is_unsigned<T>::value;

namespace imp {
    // Check that T can be a return value
    template <class T> auto test_returnable(int) -> decltype(void(static_cast<T(*)()>(nullptr)), true_type{});
    template <class T> auto test_returnable(...) -> false_type;
    template <class T> inline constexpr bool is_returnable_v = decltype(test_returnable<T>(0))::value;
    // Check that From can be implicitly converted to To
    template <class From, class To> auto test_implicitly(int) -> decltype(void(declval<void(&)(To)>()(declval<From>())), true_type{});
    template <class From, class To> auto test_implicitly(...) -> false_type;
    template <class From, class To> inline constexpr bool test_implicitly_v = decltype(test_implicitly<From, To>(0))::value;
    // Combine the above
    template <class From, class To> struct test_ret_impl : bool_constant<is_returnable_v<To> && test_implicitly_v<From, To>> {};
    template <class From, class To> inline constexpr bool test_ret_impl_v = test_ret_impl<From, To>::value;

    template <class From, class To>
    struct is_convertible : bool_constant<test_ret_impl_v<From, To> || (is_void_v<From> && is_void_v<To>)> {};
}

/// Checks if a type can be converted to the other type
template <class From, class To> inline constexpr bool is_convertible_v = imp::is_convertible<From, To>::value;

namespace imp {

    template <class T>
    struct decay {
    private:
        using U = remove_reference_t<T>;
    public:
        using type =
            // If an array, decay into a pointer
            conditional_t<is_array_v<U>, add_pointer_t<remove_extent_t<U>>,
                // Else, if a function, add a pointer, else remove CV
                conditional_t<is_function_v<U>, add_pointer_t<U>, remove_cv_t<U>>>;
    };
}
/// Applies type transformations as when passing a function argument by value
template <class T> using decay_t = imp::decay<T>::type;

namespace imp {
    // Is object destructible? Must be object and not a reference
    template <class T> auto test_destructible(int) -> decltype(declval<T>().~T(), true_type{});
    template <class T> auto test_destructible(...) -> false_type;
    template <class T> struct is_dx_defined : decltype(test_destructible<T>(0)) {};
    template <class T> inline constexpr bool is_dx_defined_v = is_dx_defined<T>::value;

    // Helper: Is type void, function, or unbound array?
    template <class T> struct is_vfua : bool_constant<is_void_v<T> || is_function_v<T> || is_unbounded_array_v<T>> {};
    template <class T> inline constexpr bool is_vfua_v = is_vfua<T>::value;

    /*
        is_destructible implementation:
            is_dx_impl : If T is a reference then we're done: true
            is_dx_p2   : If T is void, function, or unbounded array then we're done: false
            is_dx_p3   : Remove all array extents and check for existence of destructor
    */
    template <class T>                           struct is_dx_p3             : is_dx_defined<remove_all_extents_t<T>> {};
    template <class T, bool = is_vfua_v<T>>      struct is_dx_p2             : false_type {};
    template <class T>                           struct is_dx_p2<T, false>   : is_dx_p3<T> {};
    template <class T, bool = is_reference_v<T>> struct is_dx_impl           : true_type {};
    template <class T>                           struct is_dx_impl<T, false> : is_dx_p2<T> {};
    template <class T> inline constexpr bool is_dx_impl_v = is_dx_impl<T>::value;

    /*
        is_nothrow_destructible implementation:
            is_nt_dx_impl   : If T is a reference then we're done: true
            is_nt_dx_p2     : If T is void, function, or unbounded array then we're done: false
            is_nt_dx_p3     : Remove all array extents
            is_nt_dx_p4     : If don't have a destructor then we're done: false
                              else, we're done via evaluation of noexcept on destructor
    */
    template <class T, bool = is_dx_defined_v<T>> struct is_nt_dx_p4             : bool_constant<noexcept(declval<T>().~T())> {};
    template <class T>                            struct is_nt_dx_p4<T, false>   : false_type     {}; // No destructor: false
    template <class T>                            struct is_nt_dx_p3             : is_nt_dx_p4<remove_all_extents_t<T>> {};
    template <class T, bool = is_vfua_v<T>>       struct is_nt_dx_p2             : false_type     {}; // void, func, []: false
    template <class T>                            struct is_nt_dx_p2<T, false>   : is_nt_dx_p3<T> {};
    template <class T, bool = is_reference_v<T>>  struct is_nt_dx_impl           : true_type      {}; // Reference: true
    template <class T>                            struct is_nt_dx_impl<T, false> : is_nt_dx_p2<T> {};
    template <class T> inline constexpr bool is_nt_dx_impl_v = is_nt_dx_impl<T>::value;

    template <class T> struct is_destructible           : is_dx_impl<T> {};
    template <class T> struct is_trivially_destructible : bool_constant<is_destructible<T>::value && imp::has_trivial_destructor_v<T>> {};
    template <class T> struct is_nothrow_destructible   : is_nt_dx_impl<T> {};

    template <class T, class... Args> struct is_constructible : is_constructible__imp<T, Args...> {};
    template <class T> struct is_default_constructible        : is_constructible__imp<T> {};
    template <class T> struct is_copy_constructible           : is_constructible__imp<T, add_const_t<add_lvalue_reference_t<T>>> {};
    template <class T> struct is_move_constructible           : is_constructible__imp<T, add_rvalue_reference_t<T>> {};

    template <class T, class U> struct is_assignable : is_assignable__imp<T, U> {};
    template <class T> struct is_copy_assignable     : is_assignable__imp<add_lvalue_reference_t<T>, add_const_t<add_lvalue_reference_t<T>>> {};
    template <class T> struct is_move_assignable     : is_assignable__imp<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};

    template <class T, class... Args> struct is_trivially_constructible : is_trivially_constructible__imp<T, Args...> {};
    template <class T> struct is_trivially_default_constructible        : is_trivially_constructible__imp<T> {};
    template <class T> struct is_trivially_copy_constructible           : is_trivially_constructible__imp<T, add_const_t<add_lvalue_reference_t<T>>> {};
    template <class T> struct is_trivially_move_constructible           : is_trivially_constructible__imp<T, add_rvalue_reference_t<T>> {};

    template <class T, class Arg> struct is_trivially_assignable : is_trivially_assignable__imp<T, Arg> {};
    template <class T> struct is_trivially_copy_assignable       : is_trivially_assignable__imp<add_lvalue_reference_t<T>, add_const_t<add_lvalue_reference_t<T>>> {};
    template <class T> struct is_trivially_move_assignable       : is_trivially_assignable__imp<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};

    template <class T, class... Args> struct is_nothrow_constructible : is_nothrow_constructible__imp<T, Args...> {};
    template <class T> struct is_nothrow_default_constructible        : is_nothrow_constructible__imp<T> {};
    template <class T> struct is_nothrow_copy_constructible           : is_nothrow_constructible__imp<T, add_const_t<add_lvalue_reference_t<T>>> {};
    template <class T> struct is_nothrow_move_constructible           : is_nothrow_constructible__imp<T, add_rvalue_reference_t<T>> {};

    template <class T, class U> struct is_nothrow_assignable : is_nothrow_assignable__imp<T, U> {};
    template <class T> struct is_nothrow_copy_assignable     : is_nothrow_assignable__imp<add_lvalue_reference_t<T>, add_const_t<add_lvalue_reference_t<T>>> {};
    template <class T> struct is_nothrow_move_assignable     : is_nothrow_assignable__imp<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};
}

/// Checks if type is destructible
template <class T> inline constexpr bool is_destructible_v = imp::is_destructible<T>::value;
/// Checks if type has a trivial destructor
template <class T> inline constexpr bool is_trivially_destructible_v = imp::is_trivially_destructible<T>::value;
/// Checks if type has nothrow destructor
template <class T> inline constexpr bool is_nothrow_destructible_v = imp::is_nothrow_destructible<T>::value;
/// Checks if type is constructible from given arguments
template <class T, class... Args> inline constexpr bool is_constructible_v = imp::is_constructible<T, Args...>::value;
/// Checks if type is default constructible
template <class T> inline constexpr bool is_default_constructible_v = imp::is_default_constructible<T>::value;
/// Checks if type is copy constructible
template <class T> inline constexpr bool is_copy_constructible_v = imp::is_copy_constructible<T>::value;
/// Checks if type is move constructible
template <class T> inline constexpr bool is_move_constructible_v = imp::is_move_constructible<T>::value;
/// Checks if type is assignable from given argument
template <class T, class U> inline constexpr bool is_assignable_v = imp::is_assignable<T, U>::value;
/// Checks if type is copy assignable
template <class T> inline constexpr bool is_copy_assignable_v = imp::is_copy_assignable<T>::value;
/// Checks if type is move assignable
template <class T> inline constexpr bool is_move_assignable_v = imp::is_move_assignable<T>::value;
/// Checks if type is trivially constructible from given arguments
template <class T, class... Args> inline constexpr bool is_trivially_constructible_v = imp::is_trivially_constructible<T, Args...>::value;
/// Checks if type is trivially default constructible
template <class T> inline constexpr bool is_trivially_default_constructible_v = imp::is_trivially_default_constructible<T>::value;
/// Checks if type has a trivial copy constructor
template <class T> inline constexpr bool is_trivially_copy_constructible_v = imp::is_trivially_copy_constructible<T>::value;
/// Checks if type has a trivial move constructor
template <class T> inline constexpr bool is_trivially_move_constructible_v = imp::is_trivially_move_constructible<T>::value;
/// Checks if type is trivially assignable from given argument
template <class T, class Arg> inline constexpr bool is_trivially_assignable_v = imp::is_trivially_assignable<T, Arg>::value;
/// Checks if type is trivially copy assignable
template <class T> inline constexpr bool is_trivially_copy_assignable_v = imp::is_trivially_copy_assignable<T>::value;
/// Checks if type is trivially move assignable
template <class T> inline constexpr bool is_trivially_move_assignable_v = imp::is_trivially_move_assignable<T>::value;
/// Checks if type is nothrow constructible
template <class T, class... Args> inline constexpr bool is_nothrow_constructible_v = imp::is_nothrow_constructible<T, Args...>::value;
/// Checks if type is nothrow default constructible
template <class T> inline constexpr bool is_nothrow_default_constructible_v = imp::is_nothrow_default_constructible<T>::value;
/// Checks if type is nothrow copy constructible
template <class T> inline constexpr bool is_nothrow_copy_constructible_v = imp::is_nothrow_copy_constructible<T>::value;
/// Checks if type is nothrow move constructible
template <class T> inline constexpr bool is_nothrow_move_constructible_v = imp::is_nothrow_move_constructible<T>::value;
/// Checks if type is nothrow assignable
template <class T, class U> inline constexpr bool is_nothrow_assignable_v = imp::is_nothrow_assignable<T, U>::value;
/// Checks if type is nothrow copy assignable
template <class T> inline constexpr bool is_nothrow_copy_assignable_v = imp::is_nothrow_copy_assignable<T>::value;
/// Checks if type is nothrow copy assignable
template <class T> inline constexpr bool is_nothrow_move_assignable_v = imp::is_nothrow_move_assignable<T>::value;

namespace imp {
    template <class T> struct dependent_false : sys::false_type {};
}

/// Utility class for using static_assert to flag errors in constexpr code
template <class T> inline constexpr bool dependent_false_v = imp::dependent_false<T>::value;

/// Checks if a type is a specialization of a given template
/// Eequires template type; cannot be a template value
template <class T, template <class...> class Template>
struct is_specialization : false_type {};
template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : true_type {};

// TODO : is_specialization_v

template <class T, template <size_t> class Template>
struct is_specialization_size_t : false_type {};
template <template <size_t> class Template, size_t Arg>
struct is_specialization_size_t<Template<Arg>, Template> : true_type {};

template <sys::size_t N>
struct string_literal
{
    using char_t = char;

    constexpr string_literal(const char_t (&s)[N]) {
        for (sys::size_t i=0; i<N; ++i)
            value[i] = s[i];
    }

    constexpr const char_t* data() const noexcept
        { return value; }
    constexpr sys::size_t length() const noexcept
        { return N; }

    char_t value[N];        // Must be public
};

// -- common_type<...> implementation --------------------------------------

/// Determines the common type among all types, if any
template <class... T> struct common_type;

//  - common_type<T1> ------------------------------------------------------
template <class T1>   struct common_type<T1> {
    using type = common_type<T1,T1>::type;
};

//  - common_type<T1,T2> ---------------------------------------------------
namespace imp {

    template <class...> using void_t = void;

    // This is the heart of common_type
    template <class T1, class T2>
    using conditional_result_t = decltype(false ? declval<T1>() : declval<T2>());

    template <class, class, class = void>
    struct decay_conditional_result {};
    template <class T1, class T2>
    struct decay_conditional_result<T1, T2, void_t<conditional_result_t<T1, T2>>> {
        using type = decay_t<conditional_result_t<T1, T2>>;
    };

    // - common_type<T1,T2>
    template <class T1, class T2, class = void>
    struct common_type2 {
        using type = decay_conditional_result<const T1&, const T2&>::type;
    };
    template <class T1, class T2>
    struct common_type2<T1, T2, void_t<conditional_result_t<T1, T2>>> {
        using type = decay_conditional_result<T1, T2>::type;
    };
}

template <class T1, class T2>
struct common_type<T1,T2> {
    using type = conditional_t<
        is_same_v<T1, decay_t<T1>> && is_same_v<T2, decay_t<T2>>,
            imp::common_type2<T1, T2>, common_type<decay_t<T1>, decay_t<T2>>>::type;
};

//  - common_type<T1,T2,T3...> ---------------------------------------------
namespace imp {
    template <class VoidPlaceholder, class T1, class T2, class... Ts>
    struct common_type3p {};
    template <class T1, class T2, class... Ts>
    struct common_type3p<void_t< typename common_type<T1,T2>::type>, T1, T2, Ts...> {
        using type = common_type<typename common_type<T1,T2>::type, Ts...>::type;
    };
}

template <class T1, class T2, class... Ts>
struct common_type<T1, T2, Ts...> {
    using type = imp::common_type3p<void, T1, T2, Ts...>::type;
};

template <class... T> using common_type_t = common_type<T...>::type;

// -- common_reference<...> implementation ---------------------------------

/// Customization point that allows users to influence the result of common_reference for user-defined types
template <class T, class U, template<class> class TQual, template<class> class UQual>
struct basic_common_reference {};

namespace imp {

    // -- common_ref - common reference between two types

    template <class T, class U>
    using cr_cond_res_t = decltype(false ? declval<T>() : declval<U>());

    // Primary template
    template <class A, class B>
    struct common_ref {};

    // - Both l-value references

    template<class A, class B,
        class X = remove_reference_t<A>, class Y = remove_reference_t<B>>
    concept common_ref_ll =
        is_lvalue_reference_v<A> && is_lvalue_reference_v<B> &&
        requires { typename cr_cond_res_t<copy_cv_t<X, Y> &, copy_cv_t<Y, X> &>; };

    template <class A, class B>
        requires common_ref_ll<A,B>
    struct common_ref<A,B> {
        using X = remove_reference_t<A>;
        using Y = remove_reference_t<B>;
        using type = cr_cond_res_t<copy_cv_t<X, Y> &, copy_cv_t<Y, X> &>;
    };

    // Both r-value references

    template <class X, class Y>
    using common_ref_rr_c =
        add_rvalue_reference_t<remove_reference_t<typename common_ref<X&, Y&>::type>>;

    template<class A, class B,
        class X = remove_reference_t<A>, class Y = remove_reference_t<B>>
    concept common_ref_rr =
        is_rvalue_reference_v<A> && is_rvalue_reference_v<B> &&
        requires { typename common_ref_rr_c<X, Y>; } &&
        is_convertible_v<A, common_ref_rr_c<X, Y>> &&
        is_convertible_v<B, common_ref_rr_c<X, Y>>;

    template <class A, class B>
        requires common_ref_rr<A, B>
    struct common_ref<A,B> {
        using X = remove_reference_t<A>;
        using Y = remove_reference_t<B>;
        using type = common_ref_rr_c<X, Y>;
    };

    // A is an rvalue reference and B is an lvalue reference

    template <class X, class Y>
    using common_ref_rl_d = typename common_ref<const X&, Y&>::type;

    template<class A, class B,
        class X = remove_reference_t<A>, class Y = remove_reference_t<B>>
    concept common_ref_rl =
        is_rvalue_reference_v<A> && is_lvalue_reference_v<B> &&
        requires { typename common_ref_rl_d<X, Y>; } &&
        is_convertible_v<A, common_ref_rl_d<X, Y>>;

    template <class A, class B>
        requires common_ref_rl<A, B>
    struct common_ref<A,B> {
        using X = remove_reference_t<A>;
        using Y = remove_reference_t<B>;
        using type = common_ref_rl_d<X, Y>;
    };

    // A is an lvalue reference and B is an rvalue reference

    template<class A, class B,
        class X = remove_reference_t<A>, class Y = remove_reference_t<B>>
    concept common_ref_lr =
        is_lvalue_reference_v<A> && is_rvalue_reference_v<B>;

    template <class A, class B>
        requires common_ref_lr<A, B>
    struct common_ref<A,B> : common_ref<B,A> {};

    // -- common_reference - common reference between N types

    // Primary template
    template <class... Ts>
    struct common_reference{};

    // - One type

    template <class T>
    struct common_reference<T>
    {
        using type = T;
    };

    // - Two types

    // Two types: Case 1: a simple common reference
    template <class T1, class T2>
    concept cr2_case1 =
        is_reference_v<T1> && is_reference_v<T2> &&
        requires { typename common_ref<T1, T2>::type; };

    template <class T1, class T2>
        requires cr2_case1<T1, T2>
    struct common_reference<T1,T2>
    {
        using type = common_ref<T1, T2>::type;
    };

    // Two types: Case 2: Hook for basic_common_reference

    template <class T>
    struct xref      { template<class U> using type = copy_cv_t<T, U>; };
    template <class T>
    struct xref<T&>  { template<class U> using type = add_lvalue_reference_t<copy_cv_t<T, U>>; };
    template <class T>
    struct xref<T&&> { template<class U> using type = add_rvalue_reference_t<copy_cv_t<T, U>>; };

    template <class T1, class T2>
    concept cr2_case2 = requires
        { typename basic_common_reference<remove_cvref_t<T1>, remove_cvref_t<T2>,
            xref<T1>::template type, xref<T2>::template type>::type; };

    template <class T1, class T2>
        requires (!cr2_case1<T1,T2> && cr2_case2<T1,T2>)
    struct common_reference<T1,T2>
    {
        using type = basic_common_reference<remove_cvref_t<T1>, remove_cvref_t<T2>,
            xref<T1>::template type, xref<T2>::template type>::type;
    };

    // Two types: Case 3: cr_cond_res_t
    template <class T1, class T2>
    concept cr2_case3 = requires { typename cr_cond_res_t<T1,T2>; };

    template <class T1, class T2>
        requires (!cr2_case1<T1,T2> && !cr2_case2<T1,T2> && cr2_case3<T1,T2>)
    struct common_reference<T1,T2>
    {
        using type = cr_cond_res_t<T1,T2>;
    };

    // Two types: Case 4: common_type_t
    template <class T1, class T2>
    concept cr2_case4 = requires { typename common_type_t<T1,T2>; };

    template <class T1, class T2>
        requires (!cr2_case1<T1,T2> && !cr2_case2<T1,T2> && !cr2_case3<T1,T2> && cr2_case4<T1,T2>)
    struct common_reference<T1,T2>
    {
        using type = common_type_t<T1,T2>;
    };

    // - Three or more types

    template <class T1, class T2, class... TN>
        requires (sizeof...(TN) > 0) &&
            requires { typename common_reference<T1,T2>::type; }
    struct common_reference<T1,T2,TN...>
    {
        using C = typename common_reference<T1, T2>::type;
        using type = typename common_reference<C, TN...>::type;
    };
}

/// The common reference type over all \tparam Ts
template <class... Ts>
using common_reference_t = imp::common_reference<Ts...>::type;

_SYS_END_NS

#endif // ifndef sys_TypeTraits__included
