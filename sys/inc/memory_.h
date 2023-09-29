/**
 * @file    memory_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Classes related to memory management
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_memory__included
#define sys_memory__included

#include <_core_.h>
#include <types_.h>
#include <utility_.h>
#include <type_traits_.h>

_SYS_BEGIN_NS

/// Smart pointer with unique object ownership semantic
template <class T>
class unique_ptr
{
public:

    using pointer = T*;
    using element_type = T;

    /// True if unique_ptr<U> is convertible to unique_ptr<T>
    template <class U>
    static constexpr bool is_element_convertible =
        !is_array_v<U> && is_convertible_v<typename unique_ptr<U>::pointer, pointer>;

    // -- Construction

    /// Construct an object that owns nothing
    constexpr unique_ptr() noexcept = default;
    /// Construct an object that owns nothing
    constexpr unique_ptr(nullptr_t) noexcept {}
    /// Constructs from explicit pointer
    constexpr explicit unique_ptr(pointer p) noexcept
        : _ptr(p) {}
    /// Construct by transferring ownership from another instance
    constexpr unique_ptr(unique_ptr&& that) noexcept
        : _ptr(that.release()) {}
    /// Converting move constructor
    template <class U> requires (is_element_convertible<U>)
    constexpr unique_ptr(unique_ptr<U>&& u) noexcept
        : _ptr(u.release()) {}
    // No copy constructor
    unique_ptr(const unique_ptr&) = delete;

    // -- Attributes

    /// Returns the underlying managed pointer
    constexpr pointer get() const noexcept { return _ptr; }

    /// Returns true if we have a managed object
    constexpr explicit operator bool() const noexcept { return get() != nullptr; }

    // -- Operations

    /// Returns a pointer to the managed object and releases the ownership
    [[nodiscard]] constexpr pointer release() noexcept
    {
        auto ret = get();
        _ptr = nullptr;
        return ret;
    }

    /// Replaces the managed object
    constexpr void reset(pointer p = pointer()) noexcept
    {
        auto old = get();
        _ptr = p;

        static_assert(sizeof(T) > 0);   // Type must be complete
        delete old;
    }

    /// Swaps the managed objects
    void swap(unique_ptr& other) noexcept
    {
        sys::swap(_ptr, other._ptr);
    }

    // -- Implementation

    ~unique_ptr()
    {
        static_assert(sizeof(T) > 0);   // Type must be complete
        delete _ptr;
    }

    // No copy assignment
    unique_ptr& operator=(const unique_ptr&) = delete;

    /// Move assignment
    unique_ptr& operator=(unique_ptr&& that) noexcept
    {
        reset(that.release());
        return *this;
    }

    /// Converting assign operator
    template <class U> requires (is_element_convertible<U>)
    unique_ptr& operator=(unique_ptr<U>&& that) noexcept
    {
        reset(that.release());
        return *this;
    }

    constexpr pointer operator->() const noexcept { return _ptr; }

private:

    pointer         _ptr{nullptr};
};

/// Specialization of exclusive_ptr for arrays
template <class T>
class unique_ptr<T[]>
{
public:

    using pointer = T*;
    using element_type = T;

    // -- Construction

    /// Construct an object that owns nothing
    constexpr unique_ptr() noexcept = default;
    /// Construct an object that owns nothing
    constexpr unique_ptr(nullptr_t) noexcept {}
    /// Construct from a raw pointer
    constexpr explicit unique_ptr(pointer p) noexcept
        : _ptr(p) {}
    /// Construct by transferring ownership from another instance
    constexpr unique_ptr(unique_ptr&& that) noexcept
        : _ptr(that.release()) {}
    // No copy constructor
    unique_ptr(const unique_ptr&) = delete;

    // -- Attributes

    /// Returns the underlying managed pointer
    constexpr pointer get() const noexcept { return _ptr; }

    /// Returns true if we have a managed object
    constexpr explicit operator bool() const noexcept { return get() != nullptr; }

    // -- Operations

    /// Returns a pointer to the managed object and releases the ownership
    [[nodiscard]] constexpr pointer release() noexcept
    {
        auto ret = get();
        _ptr = nullptr;
        return ret;
    }

    /// Replaces the managed object
    constexpr void reset(pointer p) noexcept
    {
        auto old = get();
        _ptr = p;

        static_assert(sizeof(T) > 0);   // Type must be complete
        delete[] old;
    }

    /// Swaps the managed objects
    void swap(unique_ptr& other) noexcept
    {
        sys::swap(_ptr, other._ptr);
    }

    // -- Implementation

    ~unique_ptr()
    {
        static_assert(sizeof(T) > 0);   // Type must be complete
        delete[] _ptr;
    }

    // No copy assignment
    unique_ptr& operator=(const unique_ptr&) = delete;

    /// Move assignment
    unique_ptr& operator=(unique_ptr&& that) noexcept
    {
        reset(that.release());
        return *this;
    }

    /// Provides indexed access to the managed array
    constexpr T& operator[](size_t i) const
    {
        return get()[i];
    }

private:

    pointer         _ptr{nullptr};
};

/// Make a unique pointer, forwarding arguments to constructor
template <class T, class... Args> requires (!is_array_v<T>)
[[nodiscard]] constexpr unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(forward<Args>(args)...));
}

/// Make a unique pointer with default constructed object
template <class T, class... Args> requires (!is_array_v<T>)
[[nodiscard]] constexpr unique_ptr<T> make_unique_for_overwrite()
{
    return unique_ptr<T>(new T);
}

template <class T> requires(is_unbounded_array_v<T>)
[[nodiscard]] constexpr unique_ptr<T> make_unique(size_t size)
{
    return unique_ptr<T>(new remove_extent_t<T>[size]());
}

template <class T> requires(is_unbounded_array_v<T>)
[[nodiscard]] constexpr unique_ptr<T> make_unique_for_overwrite(size_t size)
{
    return unique_ptr<T>(new remove_extent_t<T>[size]);
}

template <class T>
constexpr T* addressof(T& arg) noexcept
{
    return __builtin_addressof(arg);
}

template<class T>
constexpr T* addressof(const T&&) = delete;

namespace mem {

/// Copy bytes of elements from src to dst; memory areas may not overlap
template <class T>
    requires (is_trivially_copyable_v<T>)
static constexpr inline T* copy_bytes(T* dst, const T* src, size_t count)
{
    if (is_constant_evaluated()) {
        T* ret = dst;
        while (count--) *dst++ = *src++;
        return ret;
    }

    __builtin_memcpy(dst, src, count * sizeof(T));
    return dst;
}

namespace imp {

// Copy bytes in reverse from src to dst; memory areas may not overlap
static inline void* copy_bytes_reverse_imp(char* dst, const char* src, size_t count)
{
    // Copy byte wise until count is 32-bit word aligned
    if (size_t unaligned = count & 3) {

        auto dstb = dst + count - 1;
        auto srcb = src  + count - 1;

        for (size_t i = 0; i < unaligned; ++i)
            *dstb-- = *srcb--;

        count -= unaligned;
    }

    // 32-bit aligned addresses?
    if (count && (0 == (reinterpret_cast<uintptr_t>(dst) & 3)) && (0 == (reinterpret_cast<uintptr_t>(src) & 3))) {
        if (size_t word_count = count >> 2) {
            auto dstw = reinterpret_cast<      uint32_t*>(dst) + word_count - 1;
            auto srcw = reinterpret_cast<const uint32_t*>(src)  + word_count - 1;
            for (size_t i = 0; i < word_count; ++i)
                *dstw-- = *srcw--;

            count -= (word_count << 2);
        }
    }

    // 16-bit aligned addresses?
    else if (count && (0 == (reinterpret_cast<uintptr_t>(dst) & 1)) && (0 == (reinterpret_cast<uintptr_t>(src) & 1))) {
        if (size_t word_count = count >> 1) {
            auto dstw = reinterpret_cast<      uint16_t*>(dst) + word_count - 1;
            auto srcw = reinterpret_cast<const uint16_t*>(src)  + word_count - 1;
            for (size_t i = 0; i < word_count; ++i)
                *dstw-- = *srcw--;

            count -= (word_count << 1);
        }
    }

    // Copy whatever else remains byte-wise
    if (count) {

        auto dstb = dst + count - 1;
        auto srcb = src  + count - 1;

        while (count--)
            *dstb-- = *srcb--;
    }

    return dst;
}

}

/// Copy bytes of elements in reverse from src to dst; memory areas may not overlap
template <class T>
    requires (is_trivially_copyable_v<T>)
static constexpr T* copy_bytes_reverse(T* dst, const T* src, size_t count)
{
    if (is_constant_evaluated()) {
        auto dstr = dst + count - 1;
        auto srcr = src + count - 1;
        while (count--)
            *dstr-- = *srcr--;
        return dst;
    }

    auto dstc = reinterpret_cast<char*>(dst);
    auto srcc = reinterpret_cast<const char*>(src);
    imp::copy_bytes_reverse_imp(dstc, srcc, count * sizeof(T));
    return dst;
}

/// Copy bytes of elements from src to dst; memory areas may overlap
template <class T>
    requires (is_trivially_copyable_v<T>)
static constexpr inline T* move_bytes(T* dst, const T* src, size_t count)
{
    if (dst > src)
        return copy_bytes_reverse(dst, src, count);
    if (dst < src)
        return copy_bytes(dst, src, count);

    return dst;
}

/// Compare memory bytes
static inline int compare_bytes(const void* s1, const void* s2, size_t count)
{
    return __builtin_memcmp(s1, s2, count);
}

template <class T>
static constexpr inline T* fill(T* dest, const T& fill_val, size_t count)
{
    if (is_constant_evaluated()) {
        auto ret = dest;
        while (count--)
            *dest++ = fill_val;
        return ret;
    }

    if constexpr (is_trivially_copyable_v<T> && sizeof(T) == 1) {
        __builtin_memset(dest, fill_val, count);
        return dest;
    }
    else {
        auto ret = dest;
        while (count--)
            *dest++ = fill_val;
        return ret;
    }
}

}

_SYS_END_NS

namespace std {
    // construct_at is another special case that requires std. GCC only
    // allows std::construct_at to use placement new in a constexpr
    // context. No other namespace is allowed.

    template <class T, class... Args>
    constexpr T* construct_at(T* p, Args&&... args)
        noexcept(sys::is_nothrow_constructible_v<T, Args...>)
    {
        return ::new(static_cast<void*>(p)) T(sys::forward<Args>(args)...);
    }
}

_SYS_BEGIN_NS

using std::construct_at;

template <class T>
constexpr void destruct_at(T* doomed) noexcept(is_nothrow_destructible_v<T>)
{
    if constexpr (is_array_v<T>) {
        for (auto& e : *doomed)
            destruct_at(&e);
    }
    else
        doomed->~T();
}

_SYS_END_NS

#endif // ifndef sys_memory__included
