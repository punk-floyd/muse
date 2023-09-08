/**
 * @file    functional_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Function objects, function invocations, bind operations and reference wrappers
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_functional__included
#define sys_functional__included

#include <_core_.h>
#include <type_traits_.h>
#include <memory_.h>

_SYS_BEGIN_NS

template <class T> class ref_wrap;

// Details for sys::ref_wrap
namespace imp {

    template <class T> constexpr T& FUN(T& t) noexcept { return t; }
    template <class T> void FUN(T&&) = delete;

    template <class T>
    concept funnable = requires { { FUN(declval<T>()) }; };

    template <class T> struct is_ref_wrap               : false_type {};
    template <class T> struct is_ref_wrap<ref_wrap<T>>  : true_type {};
}

// Checks if type is a specialization of sys::ref_wrap
template <class T> inline constexpr bool is_ref_wrap_v = imp::is_ref_wrap<T>::value;

// Details for sys::invoke_result_t
namespace imp {

    // General case; function object
    template <class T>
    struct invoke_work {
        template <class F, class... Args>
        static auto call(F&& f, Args&&... args)
            -> decltype(forward<F>(f)(forward<Args>(args)...));
    };

    // Member functions and data
    template <class Base, class MT>
    struct invoke_work<MT Base::*>
    {
        template <class T>
            requires is_base_of_v<Base, decay_t<T>>
        static auto get(T&& t) -> T&&;

        template <class T>
            requires is_ref_wrap_v<decay_t<T>>
        static auto get(T&& t) -> decltype(t.get());

        template <class T>
            requires (!is_base_of_v<Base, decay_t<T>> && !is_ref_wrap_v<decay_t<T>>)
        static auto get(T&& t) -> decltype(*forward<T>(t));

        // Member function
        template <class T, class... Args, class MT1>
            requires is_function_v<MT1>
        static auto call(MT1 Base::*pmf, T&& t, Args&&... args)
            -> decltype((invoke_work::get(forward<T>(t)).*pmf)(forward<Args>(args)...));

        // Member data
        template <class T>
        static auto call(MT Base::*pmd, T&& t)
            -> decltype(invoke_work::get(forward<T>(t)).*pmd);
    };

    template <class F, class... Args>
    auto INVOKE(F&& f, Args&&... args)
        -> decltype(invoke_work<decay_t<F>>::call(forward<F>(f), forward<Args>(args)...));

    template <class Void, class, class...>
    struct invoke_result {};

    template <class F, class... Args>
    struct invoke_result<decltype(void(INVOKE(declval<F>(), declval<Args>()...))), F, Args...>
    {
        using type = decltype(INVOKE(declval<F>(), declval<Args>()...));
    };
}

template <class F, class... Args>
concept invocable =
    requires (F&& f, Args&&... args)
    {
        imp::INVOKE(forward<F>(f), forward<Args>(args)...);
    };

template <class F, class... Args>
concept nothrow_invocable =
    requires (F&& f, Args&&... args) {
        requires noexcept (imp::INVOKE(forward<F>(f), forward<Args>(args)...));
    };

namespace imp {
    template <class F, class... Args>
    struct is_invocable : false_type {};

    template <class F, class... Args>
        requires invocable<F, Args...>
    struct is_invocable<F, Args...> : true_type {};

    template <class F, class... Args>
    struct is_nothrow_invocable : false_type {};

    template <class F, class... Args>
        requires nothrow_invocable<F, Args...>
    struct is_nothrow_invocable<F, Args...> : true_type {};
}

template <class F, class... Args>
inline constexpr bool is_invocable_v = imp::is_invocable<F, Args...>::value;

template <class F, class... Args>
inline constexpr bool is_nothrow_invocable_v = imp::is_nothrow_invocable<F, Args...>::value;

template <class F, class... Args>
using invoke_result_t = imp::invoke_result<void, F, Args...>::type;

/// A reference wrapper for anything that is copy constructible and copy assignable
template <class T>
class ref_wrap          // == std::reference_wrapper wannabe
{
public:

    using type = T;

    // -- Construction

    /// Construct from target object
    template <class U>
        requires (!is_same_v<decay_t<U>, ref_wrap> && imp::funnable<U>)
    constexpr ref_wrap(U&& x) noexcept(noexcept(imp::FUN(declval<U>())))
        : _ptr(addressof(forward<U>(x)))
    {}

    /// Copy constructor
    constexpr ref_wrap(const ref_wrap& other) noexcept
        : _ptr(other._ptr)
    {}

    /// Access the stored reference
    constexpr operator T&() const noexcept { return *_ptr; }

    /// Access the stored reference
    constexpr T& get() const noexcept { return *_ptr; }

    // -- Implementation

    // Copy assignment
    constexpr ref_wrap& operator= (const ref_wrap& other) noexcept
    {
        _ptr = other._ptr;
        return *this;
    }

    // Operator() : Call the stored function
    template <class... Args>
    constexpr invoke_result_t<T&, Args...> operator()(Args&&... args) const
        //noexcept(is_nothrow_invocable_v<T&, Args...>) MOOMOO Not working. I_AM_HERE
    {
        return (*_ptr)(forward<Args>(args)...);
    }

private:

    type*       _ptr;
};

// Deduction guide - Deduction of the sole class template parameter
template <class T> ref_wrap(T&) -> ref_wrap<T>;

_SYS_END_NS

#endif // ifndef sys_functional__included
