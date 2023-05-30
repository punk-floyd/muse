/**
 * @file    vector_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   vector
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_vector__included
#define sys_vector__included

#include <_core_.h>
#include "imp/vector_buf.h"
#include <type_traits_.h>
#include <iterator_.h>
#include <limits_.h>

_SYS_BEGIN_NS

template <class T>
class vector
{
public:

    using value_type = T;

    using size_type       = sys::size_t;
    using difference_type = sys::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;

    using iterator        = it_contig<T>;
    using const_iterator  = it_contig<const T>;
    using buffer_type     = imp::vector_buf<value_type>;

    // -- Construction

    /// Construct an empty vector
    constexpr vector() noexcept = default;

    /// Returns true if container is empty
    [[nodiscard]] constexpr bool is_empty() const noexcept
        { return 0 == _buf.length(); }
    /// Returns the current length of the buffer
    [[nodiscard]] constexpr size_type length() const noexcept
        { return _buf.length(); }
    /// Returns the current capacity of the buffer
    [[nodiscard]] constexpr size_type capacity() const noexcept
        { return _buf.capacity(); }
    // Returns maximum buffer length
    [[nodiscard]] constexpr size_type max_size() const noexcept
        { return numeric_limits<size_type>::max; }

    /// Direct access to the underlying data
    [[nodiscard]] constexpr pointer data() noexcept
        { return _buf.data(); }
    /// Direct access to the underlying data
    [[nodiscard]] constexpr const_pointer data() const noexcept
        { return _buf.data(); }

    // -- Element access

    /// Accesses the specified character with bounds checking
    constexpr reference at(size_type pos)
        { check_pos_ref(pos); return data()[pos]; }

    /// Accesses the specified character with bounds checking
    constexpr const_reference at(size_type pos) const
        { check_pos_ref(pos); return data()[pos]; }

    /// Accesses the first character
    constexpr reference front()
        { return data()[0]; }

    /// Accesses the first character
    constexpr const_reference front() const
        { return data()[0]; }

    /// Accesses the last character
    constexpr reference back()
        { return data()[length() - 1]; }

    /// Accesses the last character
    constexpr const_reference back() const
        { return data()[length() - 1]; }

    // -- Modifiers

    /// Adds an element to the end
    template <class Ty>
        requires same_as<remove_cvref_t<Ty>, value_type>
    constexpr iterator push_back (Ty&& t)
    {
        return append_work(sys::forward<Ty>(t));
    }

    /// Emplace an element at the end
    template <class... Args>
        requires is_constructible_v<T, Args...>
    constexpr iterator emplace(Args... args)
    {
        return append_work(sys::forward<Args>(args)...);
    }

    // -- Iterators

    constexpr iterator begin() noexcept
        { return iterator(data()); }
    constexpr iterator end() noexcept
        { return iterator(data() + length()); }

    constexpr const_iterator cbegin() const noexcept
        { return const_iterator(data()); }
    constexpr const_iterator cend() const noexcept
        { return const_iterator(data() + length()); }

protected:

    template <class... Args>
        requires is_constructible_v<T, Args...>
    constexpr iterator append_work(Args... args)
    {
        if (1 + length() > capacity())
            grow(1);
        auto ret = construct_at(data() + length(), sys::forward<Args>(args)...);
        _buf.set_length(length() + 1);
        return iterator(ret);
    }

    /// Adjust capacity by at least n items
    constexpr void grow(size_type n)
    {
        // Make sure new minimum capacity isn't too large
        size_type cap_need{};
        if (sys::add_overflow(capacity(), n, cap_need))
            throw sys::error_length();

        size_type cap_try = capacity() * 2;
        if (0 == cap_try)
            cap_try = 1;

        // Double capacity until we have enough to store cap_need
        while (cap_try < cap_need) {
            if (sys::multiply_overflow(cap_try, 2, cap_try))
                cap_try = cap_need;
        }

        ensure_buf(cap_try);
    }

    /// Ensure that our capacity is at least count items
    constexpr pointer ensure_buf(size_type count)
    {
        if (count > capacity()) {
            if constexpr (sys::is_nothrow_move_constructible_v<value_type>) {
                // Create a bigger buffer and move our content in.
                auto new_data = buffer_type{count, sys::move(_buf)};
                _buf.swap(new_data);
            }
            else {
                // Create a bigger buffer and copy our content in. Need to
                // copy because moving might throw.
                auto new_data = buffer_type{count, _buf};
                _buf.swap(new_data);
            }
        }

        return data();
    }

    /// Check position for a reference: [begin, end)
    constexpr void check_pos_ref(size_type pos) const
    {
        if (pos >= length())
            throw sys::error_bounds();
    }

private:

    buffer_type _buf;
};

_SYS_END_NS

#endif // ifndef sys_vector__included
