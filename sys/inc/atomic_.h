/**
 * @file    atomic_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Atomic operations
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_atomic__included
#define sys_atomic__included

#include <_core_.h>
#include <concepts_.h>
#include "imp/atomic_imp.h"

_SYS_BEGIN_NS

enum class memory_order {
    relaxed, consume, acquire, release, acq_rel, seq_cst
};

template <class T>
    requires is_trivially_copyable_v<T>
        && is_copy_constructible_v<T> && is_copy_assignable_v<T>
        && is_move_constructible_v<T> && is_move_assignable_v<T>
        && is_trivially_destructible_v<T>
class atomic : protected imp::atomic_imp<T>
{
    T       _val;       ///< The atomic value

protected:

    using my_base = imp::atomic_imp<T>;

public:

    using value_type = T;

    // -- Construction

    /// Non-atomic construct with value initialization
    constexpr atomic() noexcept(is_nothrow_default_constructible_v<T>)
        : _val(T()) {}

    /// Non-atomic construct with given value initialization
    constexpr atomic(T val) noexcept
        : _val(val) {}

    // -- Attributes

    static constexpr bool is_always_lock_free = my_base::imp_is_always_lock_free();

    /// Returns true if this object is lock free
    bool is_lock_free() const noexcept
    { return my_base::imp_is_lock_free(&_val); }
    /// Returns true if this object is lock free
    bool is_lock_free() const volatile noexcept
        requires is_always_lock_free
    { return my_base::imp_is_lock_free(&_val); }

    // -- Operations

    /// Performs and atomic store operation
    void store(T val, memory_order mo = memory_order::seq_cst) noexcept
    { my_base::imp_store(&_val, val, static_cast<int>(mo)); }
    /// Performs and atomic store operation
    void store(T val, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires is_always_lock_free
    { my_base::imp_store_v(&_val, val, static_cast<int>(mo)); }

    /// Performs an atomic load operation
    T load(memory_order mo = memory_order::seq_cst) const noexcept
    { return my_base::imp_load_c(&_val, static_cast<int>(mo)); }
    /// Performs an atomic load operation
    T load(memory_order mo = memory_order::seq_cst) const volatile noexcept
        requires is_always_lock_free
    { return my_base::imp_load_cv(&_val, static_cast<int>(mo)); }

    /// Atomically exchange (read-modify-write)
    T exchange(T val, memory_order mo = memory_order::seq_cst) noexcept
    { return my_base::imp_exchange_c(&_val, val, static_cast<int>(mo)); }
    /// Atomically exchange (read-modify-write)
    T exchange(T val, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires is_always_lock_free
    { return my_base::imp_exchange_cv(&_val, val, static_cast<int>(mo)); }

    /// Atomic bitwise compare and exchange
    bool compare_exchange_strong(T& expected, T desired,
        memory_order mo_success, memory_order mo_fail) noexcept
    {
        return my_base::imp_compare_exchange(&_val, &expected, desired,
            false, static_cast<int>(mo_success), static_cast<int>(mo_fail));
    }
    /// Atomic bitwise compare and exchange
    bool compare_exchange_strong(T& expected, T desired,
        memory_order mo_success, memory_order mo_fail) volatile noexcept
            requires is_always_lock_free
    {
        return my_base::imp_compare_exchange_v(&_val, &expected, desired,
            false, static_cast<int>(mo_success), static_cast<int>(mo_fail));
    }

    /// Atomic compare and exchange
    bool compare_exchange_strong(T& expected, T desired,
        memory_order mo = memory_order::seq_cst) noexcept
    {
        memory_order mo_fail;
        switch (mo) {
        case memory_order::acq_rel: mo_fail = memory_order::acquire; break;
        case memory_order::release: mo_fail = memory_order::relaxed; break;
        default:                    mo_fail = mo;
        }

        return my_base::imp_compare_exchange(&_val, &expected, desired,
            false, static_cast<int>(mo), static_cast<int>(mo_fail));
    }

    //  - Atomic increment/decrement operations (requires arithmetic T)

    /// Atomic increment; stores and returns the new sum
    T add_fetch(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires arithmetic<T>
    { return my_base::imp_add_fetch(&_val, arg, static_cast<int>(mo)); }
    /// Atomic increment; stores and returns the new sum
    T add_fetch(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires arithmetic<T> && is_always_lock_free
    { return my_base::imp_add_fetch_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic increment; store sum and return original value
    T fetch_add(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires arithmetic<T>
    { return my_base::imp_fetch_add(&_val, arg, static_cast<int>(mo)); }
    /// Atomic increment; store sum and return original value
    T fetch_add(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires arithmetic<T> && is_always_lock_free
    { return my_base::imp_fetch_add_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic decrement; stores and returns the new difference
    T sub_fetch(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires arithmetic<T>
    { return my_base::imp_sub_fetch(&_val, arg, static_cast<int>(mo)); }
    /// Atomic decrement; stores and returns the new difference
    T sub_fetch(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires arithmetic<T> && is_always_lock_free
    { return my_base::imp_sub_fetch_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic decrement; store difference and return original value
    T fetch_sub(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires arithmetic<T>
    { return my_base::imp_fetch_sub(&_val, arg, static_cast<int>(mo)); }
    /// Atomic decrement; store difference and return original value
    T fetch_sub(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires arithmetic<T> && is_always_lock_free
    { return my_base::imp_fetch_sub_v(&_val, arg, static_cast<int>(mo)); }

    //  - Atomic post bit operations (requires integral T)

    /// Atomic bitwise AND; stores and returns the new value
    T and_fetch(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_and_fetch(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise AND; stores and returns the new value
    T and_fetch(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_and_fetch_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic bitwise AND; store new value and return original value
    T fetch_and(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_fetch_and(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise AND; store new value and return original value
    T fetch_and(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_fetch_and_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic bitwise NAND; stores and returns the new value
    T nand_fetch(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_nand_fetch(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise NAND; stores and returns the new value
    T nand_fetch(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_nand_fetch_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic bitwise NAND; store new value and return original value
    T fetch_nand(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_fetch_nand(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise NAND; store new value and return original value
    T fetch_nand(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_fetch_nand_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic bitwise OR; stores and returns the new value
    T or_fetch(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_or_fetch(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise OR; stores and returns the new value
    T or_fetch(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_or_fetch_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic bitwise OR; store new value and return original value
    T fetch_or(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_fetch_or(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise OR; store new value and return original value
    T fetch_or(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_fetch_or_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic bitwise XOR; stores and returns the new value
    T xor_fetch(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_xor_fetch(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise XOR; stores and returns the new value
    T xor_fetch(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_xor_fetch_v(&_val, arg, static_cast<int>(mo)); }

    /// Atomic bitwise XOR; store new value and return original value
    T fetch_xor(T arg, memory_order mo = memory_order::seq_cst) noexcept
        requires integral<T>
    { return my_base::imp_fetch_xor(&_val, arg, static_cast<int>(mo)); }
    /// Atomic bitwise XOR; store new value and return original value
    T fetch_xor(T arg, memory_order mo = memory_order::seq_cst) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return my_base::imp_fetch_xor_v(&_val, arg, static_cast<int>(mo)); }

    // -- Operators

    /// Atomically loads and returns the current value of the atomic variable
    operator T() const noexcept
    { return load(); }
    /// Atomically loads and returns the current value of the atomic variable
    operator T() const volatile noexcept
        requires is_always_lock_free
    { return load(); }

    T operator++() noexcept
        requires integral<T>
    { return add_fetch(1); }
    T operator++() volatile noexcept
        requires integral<T> && is_always_lock_free
    { return add_fetch(1); }

    T operator++(int) noexcept
        requires integral<T>
    { return fetch_add(1); }
    T operator++(int) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return fetch_add(1); }

    T operator--() noexcept
        requires integral<T>
    { return sub_fetch(1); }
    T operator--() volatile noexcept
        requires integral<T> && is_always_lock_free
    { return sub_fetch(1); }

    T operator--(int) noexcept
        requires integral<T>
    { return fetch_sub(1); }
    T operator--(int) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return fetch_sub(1); }

    T operator+=(T arg) noexcept
        requires arithmetic<T>
    { return add_fetch(arg); }
    T operator+=(T arg) volatile noexcept
        requires arithmetic<T> && is_always_lock_free
    { return add_fetch(arg); }

    T operator-=(T arg) noexcept
        requires arithmetic<T>
    { return sub_fetch(arg); }
    T operator-=(T arg) volatile noexcept
        requires arithmetic<T> && is_always_lock_free
    { return sub_fetch(arg); }

    T operator&=(T arg) noexcept
        requires integral<T>
    { return and_fetch(arg); }
    T operator&=(T arg) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return and_fetch(arg); }

    T operator|=(T arg) noexcept
        requires integral<T>
    { return or_fetch(arg); }
    T operator|=(T arg) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return or_fetch(arg); }

    T operator^=(T arg) noexcept
        requires integral<T>
    { return xor_fetch(arg); }
    T operator^=(T arg) volatile noexcept
        requires integral<T> && is_always_lock_free
    { return xor_fetch(arg); }

    // -- Implementation

    ~atomic() = default;

    // Not copyable, not movable
    atomic(const atomic&) = delete;
    atomic& operator=(const atomic&) = delete;
    atomic(atomic&&) = delete;
    atomic& operator=(atomic&&) = delete;
};

_SYS_END_NS

#endif // ifndef sys_atomic__included
