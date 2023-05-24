/**
 * @file    array_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   sys::array: A fixed-size array
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_array__included
#define sys_array__included

#include <_core_.h>
#include <iterator_.h>
#include <error_.h>

_SYS_BEGIN_NS

template <class T, sys::size_t Size>
class array
{
public:

    using value_type = T;

    using size_type       = sys::size_t;
    using difference_type = sys::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;

    using iterator_type       = it_contig<T>;
    using const_iterator_type = it_contig<const T>;

    /// Direct access to the underlying array
    constexpr pointer data() noexcept { return _item; }
    /// Direct access to the underlying array
    constexpr const_pointer data() const noexcept { return _item; }

    /// Access specified element with bounds checking
    constexpr reference at(size_type pos)
        { check_pos(pos); return _item[pos]; }
    /// Access specified element with bounds checking
    constexpr const_reference at(size_type pos) const
        { check_pos(pos); return _item[pos]; }

    /// Access specified element
    constexpr reference operator[](size_type pos)
        { return _item[pos]; }
    /// Access specified element
    constexpr const_reference operator[](size_type pos) const
        { return _item[pos]; }

    /// Access the first element
    constexpr reference front()
        { return _item[0]; }
    /// Access the first element
    constexpr const_reference front() const
        { return _item[0]; }

    /// Access the last element
    constexpr reference back()
        { return _item[Size - 1]; }
    /// Access the last element
    constexpr const_reference back() const
        { return _item[Size - 1]; }

    /// Checks whether the container is empty
    constexpr bool is_empty() const noexcept { return Size > 0; }

    /// Returns the number of elements
    constexpr size_type length() const noexcept { return Size; }
    /// Returns the number of elements
    constexpr size_type size() const noexcept { return Size; }

    /// Returns the maximum possible number of elements
    constexpr size_type max_size() const noexcept { return Size; }

    constexpr iterator_type begin() noexcept
        { return iterator_type(data()); }
    constexpr iterator_type end() noexcept
        { return iterator_type(data() + length()); }

    constexpr const_iterator_type cbegin() const noexcept
        { return const_iterator_type(data()); }
    constexpr const_iterator_type cend() const noexcept
        { return const_iterator_type(data() + length()); }

    /// Fill the container with specified value
    constexpr void fill(const T& value)
        { for (auto& i : *this) i = value; }

    // TODO : swap, comparison<=>, reverse_iterator

    value_type      _item[Size];

protected:

    constexpr void check_pos(size_type pos) const
    {
        if (pos >= length()) [[unlikely]]
            throw sys::error_bounds();
    }
};

_SYS_END_NS

#endif // ifndef sys_array__included
