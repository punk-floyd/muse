/**
 * @file    atomic_imp.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Implements atomic operations
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_atomic_imp__included
#define sys_atomic_imp__included

_SYS_BEGIN_NS
namespace imp {

template <class T>
class atomic_imp {

public:

    // -- Construction

    constexpr atomic_imp() = default;

protected:

    /// Returns true if objects of size bytes always generate lock-free atomic instructions for the target architecture
    static constexpr bool imp_is_always_lock_free(const void* ptr = nullptr)
        { return __atomic_always_lock_free(sizeof(T), ptr); }

    /// Returns true if objects of size bytes always generate lock-free atomic instructions for the target architecture
    static bool imp_is_lock_free(const void* ptr = nullptr) noexcept
        { return __atomic_is_lock_free(sizeof(T), ptr); }

    /// Performs an atomic store operation (mem_order: [relaxed, seq_cst, release])
    static void imp_store(T* ptr, T val, int mem_order) noexcept
        { __atomic_store_n(ptr, val, mem_order); }
    /// Performs an atomic store operation (mem_order: [relaxed, seq_cst, release])
    static void imp_store_v(volatile T* ptr, T val, int mem_order) noexcept
        { __atomic_store_n(ptr, val, mem_order); }

    /// Performs an atomic load operation (mem_order: [relaxed, seq_cst, acquire, consume])
    static T imp_load_c(const T* ptr, int mem_order) noexcept
        { return __atomic_load_n(ptr, mem_order); }
    /// Performs an atomic load operation (mem_order: [relaxed, seq_cst, acquire, consume])
    static T imp_load_cv(const volatile T* ptr, int mem_order) noexcept
        { return __atomic_load_n(ptr, mem_order); }

    /// Performs an atomic exchange operation
    static T imp_exchange_c(const T* ptr, T val, int mem_order) noexcept
        { return __atomic_exchange_n(ptr, val, mem_order); }
    /// Performs an atomic exchange operation
    static T imp_exchange_cv(const volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_exchange_n(ptr, val, mem_order); }

    /// Performs an atomic compare and exchange operation
    static bool imp_compare_exchange(T* ptr, T* expected, T desired, bool weak, int mo_success, int mo_fail) noexcept
        { return __atomic_compare_exchange_n(ptr, expected, desired, weak, mo_success, mo_fail); }
    /// Performs an atomic compare and exchange operation
    static bool imp_compare_exchange_v(volatile T* ptr, T* expected, T desired, bool weak, int mo_success, int mo_fail) noexcept
        { return __atomic_compare_exchange_n(ptr, expected, desired, weak, mo_success, mo_fail); }

    /// Atomic increment; stores and returns the new sum
    static T imp_add_fetch(T* ptr, T val, int mem_order) noexcept
        { return __atomic_add_fetch(ptr, val, mem_order); }
    /// Atomic increment; stores and returns the new sum
    static T imp_add_fetch_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_add_fetch(ptr, val, mem_order); }

    /// Atomic increment; store sum and return original value
    static T imp_fetch_add(T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_add(ptr, val, mem_order); }
    /// Atomic increment; store sum and return original value
    static T imp_fetch_add_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_add(ptr, val, mem_order); }

    /// Atomic decrement; stores and returns the new difference
    static T imp_sub_fetch(T* ptr, T val, int mem_order) noexcept
        { return __atomic_sub_fetch(ptr, val, mem_order); }
    /// Atomic decrement; stores and returns the new difference
    static T imp_sum_fetch_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_sub_fetch(ptr, val, mem_order); }

    /// Atomic decrement; store difference and return original value
    static T imp_fetch_sub(T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_sub(ptr, val, mem_order); }
    /// Atomic decrement; store difference and return original value
    static T imp_fetch_sub_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_sub(ptr, val, mem_order); }

    /// Atomic bitwise AND; stores and returns the new value
    static T imp_and_fetch(T* ptr, T val, int mem_order) noexcept
        { return __atomic_and_fetch(ptr, val, mem_order); }
    /// Atomic bitwise AND; stores and returns the new value
    static T imp_and_fetch_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_and_fetch(ptr, val, mem_order); }

    /// Atomic bitwise AND; store new value and return original value
    static T imp_fetch_and(T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_and(ptr, val, mem_order); }
    /// Atomic bitwise AND; store new value and return original value
    static T imp_fetch_and_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_and(ptr, val, mem_order); }

    /// Atomic bitwise NAND; stores and returns the new value
    static T imp_nand_fetch(T* ptr, T val, int mem_order) noexcept
        { return __atomic_nand_fetch(ptr, val, mem_order); }
    /// Atomic bitwise NAND; stores and returns the new value
    static T imp_nand_fetch_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_nand_fetch(ptr, val, mem_order); }

    /// Atomic bitwise NAND; store new value and return original value
    static T imp_fetch_nand(T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_nand(ptr, val, mem_order); }
    /// Atomic bitwise NAND; store new value and return original value
    static T imp_fetch_nand_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_nand(ptr, val, mem_order); }

    /// Atomic bitwise OR; stores and returns the new value
    static T imp_or_fetch(T* ptr, T val, int mem_order) noexcept
        { return __atomic_or_fetch(ptr, val, mem_order); }
    /// Atomic bitwise OR; stores and returns the new value
    static T imp_or_fetch_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_or_fetch(ptr, val, mem_order); }

    /// Atomic bitwise OR; store new value and return original value
    static T imp_fetch_or(T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_or(ptr, val, mem_order); }
    /// Atomic bitwise OR; store new value and return original value
    static T imp_fetch_or_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_or(ptr, val, mem_order); }

    /// Atomic bitwise XOR; stores and returns the new value
    static T imp_xor_fetch(T* ptr, T val, int mem_order) noexcept
        { return __atomic_xor_fetch(ptr, val, mem_order); }
    /// Atomic bitwise XOR; stores and returns the new value
    static T imp_xor_fetch_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_xor_fetch(ptr, val, mem_order); }

    /// Atomic bitwise XOR; store new value and return original value
    static T imp_fetch_xor(T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_xor(ptr, val, mem_order); }
    /// Atomic bitwise XOR; store new value and return original value
    static T imp_fetch_xor_v(volatile T* ptr, T val, int mem_order) noexcept
        { return __atomic_fetch_xor(ptr, val, mem_order); }
};

}   // end namespace imp
_SYS_END_NS

#endif // ifndef sys_atomic_imp__included
