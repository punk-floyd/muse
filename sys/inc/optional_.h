/**
 * @file    optional_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   optional - a wrapper that may or may not hold an object
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_optional__included
#define sys_optional__included

#include <_core_.h>
#include <type_traits_.h>
#include <utility_.h>
#include <memory_.h>
#include <error_.h>

_SYS_BEGIN_NS

/// Indicator of optional type with uninitialized state
struct nullopt_t {
    constexpr explicit nullopt_t(int) {}
};

/// The nullopt_t instance
inline constexpr nullopt_t nullopt{1975};

/// Bad optional access: trying to access value when object has no value
class error_optional_access : public error_logic
{
public:

    error_optional_access() = default;

    template <string_view_convertible T>
    explicit error_optional_access(T svl) : error_logic(svl) {}

protected:

    string_view get_def_msg() const noexcept override
        { return "Bad optional access"; }
};

/// A wrapper that may or may not hold an object
template <class T>
    requires (
        !is_reference_v<T> &&
        !is_array_v<T> &&
        !is_same_v<remove_cv_t<T>, nullopt_t> &&
        !is_same_v<remove_cv_t<T>, in_place_t>
    )
class optional
{
public:

    using value_type = T;

    // -- Construction

    /// Constructs an object that does not contain a value
    constexpr optional() noexcept {}

    /// Constructs an object that does not contain a value
    constexpr optional(nullopt_t) noexcept {}

    /// Copy constructor (deleted)
    optional (const optional&)
        requires (!is_copy_constructible_v<value_type>)
    = delete;

    /// Copy constructor (trivial)
    constexpr optional (const optional& cpy)
        requires is_trivially_copy_constructible_v<value_type>
    = default;

    /// Copy constructor (non-trivial)
    constexpr optional (const optional& cpy)
        requires (
            is_copy_constructible_v<value_type> &&
            !is_trivially_copy_constructible_v<value_type>
        )
    {
        if (cpy) {
            _u.val_oui = cpy.value();
            _have_value = true;
        }
        else
            _u.val_non = no_value_type{};
    }

    /// Move constructor (deleted)
    optional (optional&&)
        requires (!is_move_constructible_v<value_type>)
    = delete;

    /// Move constructor (trivial)
    constexpr optional (optional&&) noexcept(is_nothrow_move_constructible_v<value_type>)
        requires is_trivially_move_constructible_v<value_type>
    = default;

    /// Move constructor (non-trivial)
    constexpr optional (optional&& rhs) noexcept(is_nothrow_move_constructible_v<value_type>)
        requires (
            is_move_constructible_v<value_type> &&
            !is_trivially_move_constructible_v<value_type>
        )
    {
        if (rhs) {
            _u.val_oui  = sys::move(*rhs);
            _have_value = true;
        }
        else
            _u.val_non = no_value_type{};
    }

protected:

    template <class... Args>
        requires is_constructible_v<value_type, Args...>
    void construct_value(Args&&... args)
    {
        construct_at(&_u.val_oui, forward<Args>(args)...);
        _have_value = true;
    }

public:

    /// Converting copy constructor
    template <class U>
        requires (
            is_constructible_v<value_type, const U&> && (
            is_same_v<remove_cv_t<value_type>, bool> || (
                !is_constructible_v<value_type,       optional<U>&> &&
                !is_constructible_v<value_type, const optional<U>&> &&
                !is_constructible_v<value_type,       optional<U>&&> &&
                !is_constructible_v<value_type, const optional<U>&&> &&
                !is_constructible_v<      optional<U>&,  value_type> &&
                !is_constructible_v<const optional<U>&,  value_type> &&
                !is_constructible_v<      optional<U>&&, value_type> &&
                !is_constructible_v<const optional<U>&&, value_type>
                )
            )
        )
    constexpr explicit(!is_convertible_v<const U&, value_type>)
        optional (const optional<U>& rhs)
    {
        if (rhs)
            construct_value(*rhs);
    }

    /// Converting move constructor
    template <class U>
        requires (
            is_constructible_v<value_type, const U&&> && (
            is_same_v<remove_cv_t<value_type>, bool> || (
                !is_constructible_v<value_type,       optional<U>&> &&
                !is_constructible_v<value_type, const optional<U>&> &&
                !is_constructible_v<value_type,       optional<U>&&> &&
                !is_constructible_v<value_type, const optional<U>&&> &&
                !is_constructible_v<      optional<U>&,  value_type> &&
                !is_constructible_v<const optional<U>&,  value_type> &&
                !is_constructible_v<      optional<U>&&, value_type> &&
                !is_constructible_v<const optional<U>&&, value_type>
                )
            )
        )
    constexpr explicit(!is_convertible_v<const U&&, value_type>)
        optional (const optional<U>&& rhs)
    {
        if (rhs)
            construct_value(sys::move(*rhs));
    }

    // Construct an optional with a value that's constructed from the given arguments
    template <class... Args>
        requires is_constructible_v<value_type, Args...>
    constexpr optional(sys::in_place_t, Args&&... args)
    {
        construct_value(sys::forward<Args>(args)...);
    }

    // Construct an optional with a value that's constructed from an initializer list
    template <class U, class... Args>
        requires is_constructible_v<value_type, sys::initializer_list<U>&, Args...>
    constexpr optional(sys::in_place_t, sys::initializer_list<U> ilist, Args&&... args)
    {
        construct_value(ilist, sys::forward<Args>(args)...);
    }

    /// Construct an optional with a value that's direct initialized from the given value
    template <class U = T>
        requires (
            is_constructible_v<T, U&&> &&
            !is_same_v<remove_cvref_t<U>, in_place_t> &&
            !is_same_v<remove_cvref_t<U>, optional> &&
            (!is_same_v<remove_cvref_t<U>, bool> ||
                is_specialization<remove_cvref_t<U>, optional>::value)
        )
    constexpr explicit(!is_convertible_v<U&&, T>) optional(U&& u)
    {
        construct_value(sys::forward<U>(u));
    }

    // -- Observers

    // - Accesses the contained value; undefined if we don't have a value
    constexpr const T* operator->() const   noexcept { return &_u.val_oui; }
    constexpr       T* operator->()         noexcept { return &_u.val_oui; }
    constexpr const T& operator*()  const&  noexcept { return _u.val_oui; }
    constexpr       T& operator*()       &  noexcept { return _u.val_oui; }
    constexpr const T&& operator*() const&& noexcept { return sys::move(_u.val_oui); }
    constexpr       T&& operator*()      && noexcept { return sys::move(_u.val_oui); }

protected:

    constexpr void check_value() const
    {
        if (!has_value()) [[unlikely]]
            throw error_optional_access{};
    }

    constexpr void release(bool invalidate = true)
    {
        if constexpr (!is_trivially_destructible_v<value_type>) {
            if (has_value())
                destruct_at(&_u.val_oui);
        }

        if (invalidate)
            _have_value = false;
    }

public:

    // - Access the contained value; throws error_optional_access if value-less
    constexpr const T&  value() const  & { check_value(); return _u.val_oui; }
    constexpr       T&  value()        & { check_value(); return _u.val_oui; }
    constexpr const T&& value() const && { check_value(); return sys::move(_u.val_oui); }
    constexpr       T&& value()       && { check_value(); return sys::move(_u.val_oui); }

    /// Returns the contained value if available, another value otherwise
    template <class U>
        requires (is_copy_constructible_v<value_type> && is_convertible_v<U&&, value_type>)
    constexpr value_type value_or(U&& default_value) const &
    {
        return has_value() ?
            **this : static_cast<value_type>(sys::forward<U>(default_value));
    }

    /// Returns the contained value if available, another value otherwise
    template <class U>
        requires (is_move_constructible_v<value_type> && is_convertible_v<U&&, value_type>)
    constexpr value_type value_or(U&& default_value) &&
    {
        return has_value() ?
            sys::move(**this) : static_cast<value_type>(sys::forward<U>(default_value));
    }

    /// Returns true if we have a value
    constexpr bool has_value() const noexcept { return _have_value; }
    /// Returns true if we have a value
    constexpr explicit operator bool() const noexcept { return has_value(); }

    // -- Modifiers

    /// Exchanges the contents
    constexpr void swap(optional& other)
        noexcept (
            is_nothrow_move_constructible_v<value_type> &&
            is_nothrow_swappable<value_type>
        )
    {
        if (has_value() && other.has_value())
            sys::swap(**this, *other);
        else if (has_value()) {
            // We have value, other does not.
            other.construct_value(sys::move(**this));
            release();
        }
        else if (other.has_value()) {
            // Other has value, we do not.
            construct_value(sys::move(*other));
            other.release();
        }
    }

    /// Destroys any contained value
    constexpr void reset() noexcept
    {
        if (has_value())
            release();
    }

    /// Constructs the contained value in-place
    template <class... Args>
        requires is_constructible_v<value_type, Args...>
    T& emplace(Args&&... args)
    {
        reset();
        construct_value(sys::forward<Args>(args)...);

        return **this;
    }

    /// Constructs the contained value in-place from an initialize list
    template <class U, class... Args>
        requires is_constructible_v<value_type, sys::initializer_list<U>&, Args...>
    T& emplace(sys::initializer_list<U> ilist, Args&&... args)
    {
        reset();
        construct_value(ilist, sys::forward<Args>(args)...);

        return **this;
    }

    // -- Implementation

    /// Assignment from a nullopt_t (set to no value)
    constexpr optional& operator= (sys::nullopt_t) noexcept
    {
        release();
        return *this;
    }

    /// Copy assignment (deleted)
    optional& operator= (const optional& rhs)
        requires (
            !is_copy_constructible_v<value_type> ||
            !is_copy_assignable_v<value_type>
        )
    = delete;

    /// Copy assignment (trivial)
    constexpr optional& operator= (const optional& rhs)
        requires (
            is_copy_constructible_v<value_type> &&
            is_copy_assignable_v<value_type> &&
            is_trivially_copy_assignable_v<value_type>
        )
    = default;

    /// Copy assignment (non-trivial)
    constexpr optional& operator= (const optional& rhs)
        requires (
            is_copy_constructible_v<value_type> &&
            is_copy_assignable_v<value_type> &&
            !is_trivially_copy_assignable_v<value_type>
        )
    {
        if (!rhs) {
            if (has_value())
                release();
        }
        else {
            if (has_value())
                _u.val_oui = rhs.value();
            else
                construct_value(*rhs);
        }

        return *this;
    }

    /// Move assignment (deleted)
    optional& operator= (optional&&)
        requires (
            !is_move_constructible_v<value_type> ||
            !is_move_assignable_v<value_type>
        )
    = delete;

    /// Move assignment (trivial)
    constexpr optional& operator= (optional&&)
        noexcept(
            is_nothrow_move_assignable_v<value_type> &&
            is_nothrow_move_constructible_v<value_type>
        )
        requires (
            is_move_constructible_v<value_type> &&
            is_move_assignable_v<value_type> &&
            is_trivially_move_assignable_v<value_type>
        )
    = default;

    /// Move assignment (non-trivial)
    constexpr optional& operator= (optional&& rhs)
        noexcept(
            is_nothrow_move_assignable_v<value_type> &&
            is_nothrow_move_constructible_v<value_type>
        )
        requires (
            is_move_constructible_v<value_type> &&
            is_move_assignable_v<value_type> &&
            !is_trivially_move_assignable_v<value_type>
        )
    {
        if (!rhs) {
            if (has_value())
                release();
        }
        else {
            if (has_value())
                _u.val_oui = sys::move(*rhs);
            else
                construct_value(sys::move(*rhs));
        }

        return *this;
    }

    /// Perfect forward assigment
    template <class U = T>
        requires (
            !is_same_v<remove_cvref_t<U>, optional> &&
            is_constructible_v<T, U> &&
            is_assignable_v<T&, U> &&
            (!is_scalar_v<T> || !is_same_v<decay_t<U>, T>)
        )
    constexpr optional& operator=(U&& value)
    {
        if (has_value())
            _u.val_oui = forward<U>(value);
        else
            construct_value(sys::forward<U>(value));

        return *this;
    }

    /// Converting copy assignment
    template <class U>
        requires (
            is_constructible_v<value_type, const U&> &&
            is_assignable_v<value_type&, const U&> &&
           !is_constructible_v<value_type,       optional<U>&> &&
           !is_constructible_v<value_type, const optional<U>&> &&
           !is_constructible_v<value_type,       optional<U>&&> &&
           !is_constructible_v<value_type, const optional<U>&&> &&
           !is_convertible_v<      optional<U>&, T> &&
           !is_convertible_v<const optional<U>&, T> &&
           !is_convertible_v<      optional<U>&&, T> &&
           !is_convertible_v<const optional<U>&&, T> &&
           !is_assignable_v<value_type&,       optional<U>&> &&
           !is_assignable_v<value_type&, const optional<U>&> &&
           !is_assignable_v<value_type&,       optional<U>&&> &&
           !is_assignable_v<value_type&, const optional<U>&&>
        )
    constexpr optional& operator=(const optional<U>& rhs)
    {
        if (rhs.has_value()) {
            if (has_value())
                **this = *rhs;
            else
                construct_value(*rhs);
        }
        else {
            if (has_value())
                release();
        }

        return *this;
    }

    /// Converting move assignment
    template <class U>
        requires (
            is_constructible_v<value_type, U> &&
            is_assignable_v<value_type&, U> &&
           !is_constructible_v<value_type,       optional<U>&> &&
           !is_constructible_v<value_type, const optional<U>&> &&
           !is_constructible_v<value_type,       optional<U>&&> &&
           !is_constructible_v<value_type, const optional<U>&&> &&
           !is_convertible_v<      optional<U>&, T> &&
           !is_convertible_v<const optional<U>&, T> &&
           !is_convertible_v<      optional<U>&&, T> &&
           !is_convertible_v<const optional<U>&&, T> &&
           !is_assignable_v<value_type&,       optional<U>&> &&
           !is_assignable_v<value_type&, const optional<U>&> &&
           !is_assignable_v<value_type&,       optional<U>&&> &&
           !is_assignable_v<value_type&, const optional<U>&&>
        )
    constexpr optional& operator=(optional<U>&& rhs)
    {
        if (rhs.has_value()) {
            if (has_value())
                **this = sys::move(*rhs);
            else
                construct_value(sys::move(*rhs));
        }
        else {
            if (has_value())
                release();
        }

        return *this;
    }

    constexpr ~optional()
    {
        release(false);
    }

private:

    /// A dummy, empty type that we select when we have no value
    struct no_value_type {
        constexpr no_value_type() noexcept = default;
    };

    /// Storage for the value, or not
    union store
    {
        // Prefer default, trivial implementations for default constructor
        // and the gang of five if we can (i.e., if value_type supports it).
        // This will allow optional to also be trivial in these cases.

        constexpr store() noexcept
            requires sys::is_trivially_default_constructible_v<value_type>
        = default;

        constexpr store(const store&)
            requires sys::is_trivially_copy_constructible_v<value_type>
        = default;

        constexpr store(store&&) noexcept
            requires sys::is_trivially_move_constructible_v<value_type>
        = default;

        constexpr store& operator= (const store&)
            requires sys::is_trivially_copy_assignable_v<value_type>
        = default;

        constexpr store& operator= (store&&)
            requires sys::is_trivially_move_assignable_v<value_type>
        = default;

        constexpr ~store()
            requires sys::is_trivially_destructible_v<value_type>
        = default;

        // Minimal implementation for non-trivial value_type

        constexpr store() noexcept
            requires (!sys::is_trivially_default_constructible_v<value_type>)
        : val_non() {}

        constexpr ~store()
            requires (!sys::is_trivially_destructible_v<value_type>)
        {}

        no_value_type   val_non{};
        value_type      val_oui;
    };

    store   _u;                     ///< The value, or not
    bool    _have_value{false};     ///< Do we have a value?
};

// Deduction guide - Handles non-copyable arguments and array to pointer conversion
template <class T> optional(T) -> optional<T>;

// TODO : Comparison (<=>, ==)

_SYS_END_NS

#endif // ifndef sys_optional__included
