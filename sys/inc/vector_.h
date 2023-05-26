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
#if 0
#include "imp/vector_buf.h"
#endif
#include <type_traits_.h>
#include <iterator_.h>
#include <limits_.h>
#include <bit_.h>

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

    // -- Construction

    /// Construct an empty vector
    constexpr vector() noexcept = default;

    /// Returns true if container is empty
    [[nodiscard]] constexpr bool is_empty() const noexcept
        { return 0 == length(); }
    /// Returns the current length of the buffer
    [[nodiscard]] constexpr size_type length() const noexcept
        { return _len; }
    /// Returns the current capacity of the buffer
    [[nodiscard]] constexpr size_type capacity() const noexcept
        { return _cap; }
    // Returns maximum buffer length
    [[nodiscard]] constexpr size_type max_size() const noexcept
        { return numeric_limits<size_type>::max; }

    /// Direct access to the underlying data
    [[nodiscard]] constexpr pointer data() noexcept
        { return _buf; }
    /// Direct access to the underlying data
    [[nodiscard]] constexpr const_pointer data() const noexcept
        { return _buf; }

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
    constexpr void push_back (Ty&& t)
    {
        if (1 + length() > capacity())
            grow(1);
        construct_at(data() + _len, sys::forward<Ty>(t));
        _len += 1;
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

    /// Allocate N items *without* initialization
    [[nodiscard]] static constexpr value_type* allocate(sys::size_t n)
    {
        auto p = ::operator new(n * sizeof(value_type));
        return sys::bit_cast<value_type*>(p);
    }
    static constexpr void deallocate(value_type* p)
    {
        ::operator delete(p);
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

        ensure_buf(cap_try, false);
    }

    /// Ensure that our capacity is at least count items
    constexpr pointer ensure_buf(size_type count, bool set_length = true)
    {
        if (count > capacity()) {

            auto new_data = allocate(count);
            // MOOMOO : FIXME. If we throw, that's going to leak

            auto src = cbegin();
            auto dst = new_data;

            if constexpr (sys::is_nothrow_move_constructible_v<value_type>) {
                for (size_t i = 0; i<length(); ++i)
                    construct_at(dst++, sys::move(*src++));     // Move data over
            }
            else {
                for (size_t i = 0; i<length(); ++i)
                    construct_at(dst++, *src++);                // Copy data over
            }

            // Do the old switcheroo
            deallocate(_buf);       // MOOMOO: Fixme: No one destructing yet
            _buf = new_data;
        }

        if (set_length)
            _len = count;

        return data();
    }

    /// Check position for a reference: [begin, end)
    constexpr void check_pos_ref(size_type pos) const
    {
        if (pos >= length())
            throw sys::error_bounds();
    }

private:

    pointer     _buf{nullptr};
    size_type   _cap{0};
    size_type   _len{0};
};

_SYS_END_NS

#endif // ifndef sys_vector__included
