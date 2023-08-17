/**
 * @file    vector_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   vector - Dynamic contiguous array
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

/// Dynamic contiguous array
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

    /// Construct a vector with initial capacity \p cap
    explicit vector(size_type cap)
        : _buf(cap)
    {}

    // -- Element access

    /// Accesses the specified character with bounds checking
    constexpr reference at(size_type pos)
        { check_pos_ref(pos); return data()[pos]; }
    /// Accesses the specified character with bounds checking
    constexpr const_reference at(size_type pos) const
        { check_pos_ref(pos); return data()[pos]; }

    /// Accesses the specified character
    constexpr reference operator[](size_type pos)
        { return data()[pos]; }
    /// Accesses the specified character
    constexpr const_reference operator[](size_type pos) const
        { return data()[pos]; }

    /// Accesses the first character
    constexpr reference front()
        { return data()[0]; }
    /// Accesses the first character
    constexpr const_reference front() const
        { return data()[0]; }

    /// Accesses the last character
    constexpr reference back()
        { return data()[size() - 1]; }
    /// Accesses the last character
    constexpr const_reference back() const
        { return data()[size() - 1]; }

    /// Direct access to the underlying data
    [[nodiscard]] constexpr pointer data() noexcept
        { return _buf.data(); }
    /// Direct access to the underlying data
    [[nodiscard]] constexpr const_pointer data() const noexcept
        { return _buf.data(); }

    // -- Iterators

    [[nodiscard]] constexpr iterator begin() noexcept
        { return iterator{data()}; }
    [[nodiscard]] constexpr iterator end() noexcept
        { return iterator{data() + size()}; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
        { return const_iterator{data()}; }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
        { return const_iterator{data() + size()}; }

    // -- Capacity

    /// Returns true if container is empty
    [[nodiscard]] constexpr bool is_empty() const noexcept
        { return 0 == _buf.size(); }
    /// Returns the number of elements
    [[nodiscard]] constexpr size_type size() const noexcept
        { return _buf.size(); }
    // Returns maximum possible number of elements
    [[nodiscard]] constexpr size_type max_size() const noexcept
        { return numeric_limits<size_type>::max; }
    /// Returns the current capacity of the buffer
    [[nodiscard]] constexpr size_type capacity() const noexcept
        { return _buf.capacity(); }

    constexpr void reserve(size_type new_cap)
    {
        if (new_cap > capacity()) {
            if constexpr (is_nothrow_move_constructible_v<value_type>) {
                // Create a bigger buffer and move our content in
                auto new_data = buffer_type{new_cap, sys::move(_buf)};
                _buf.swap(new_data);
            }
            else {
                // Create a bigger buffer and copy our content in. Need to
                // copy because moving might throw. If this throws, our
                // original vector is still intact and unaffected.
                auto new_data = buffer_type{new_cap, _buf};
                _buf.swap(new_data);
            }
        }
    }

    // -- Modifiers

    /// Clears the contents (capacity unaffected)
    constexpr void clear() noexcept(is_nothrow_destructible_v<value_type>)
    {
        _buf.clear_elements();
    }

    /// Insert \p value before \p pos
    template <class E>
        requires same_as<remove_cvref_t<E>, value_type>
    constexpr iterator insert (const_iterator pos, E&& value)
    {
        return insert_work(pos - cbegin(), 1, sys::forward<E>(value));
    }

    /// Insert \p count instances of \p value before \p pos
    template <class E>
        requires same_as<remove_cvref_t<E>, value_type>
    constexpr iterator insert (const_iterator pos, size_type count, E&& value)
    {
        return insert_work(pos - cbegin(), count, sys::forward<E>(value));
    }

    /// Constructs element in-place
    template <class... Args>
        requires is_constructible_v<value_type, Args...>
    constexpr iterator emplace(const_iterator pos, Args&&... args)
    {
        return insert_work(pos - cbegin(), 1, sys::forward<Args>(args)...);
    }

    /// Remove the element at \p pos
    constexpr void erase(const_iterator pos)
        noexcept(is_nothrow_move_assignable_v<value_type>)
    {
        erase_work(pos, 1);
    }

    /// Remove elements in the range [ \p first, \p last )
    constexpr void erase(const_iterator first, const_iterator last)
        noexcept(is_nothrow_move_assignable_v<value_type>)
    {
        auto d = sys::distance(first, last);
        if (d > 0)
            erase_work(first, static_cast<size_type>(d));
    }

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
    constexpr iterator emplace_back(Args&&... args)
    {
        return append_work(sys::forward<Args>(args)...);
    }

    /// Removes the last element of the container; assumes non empty vector
    constexpr void pop_back() noexcept(is_nothrow_destructible_v<value_type>)
    {
        destruct_at(data() + size() - 1);
        _buf.dec_size();
    }

    /// Changes the number of elements stored
    constexpr void resize(size_type count)
    {
        if (count < size())
            erase(cbegin() + (size() - count), cend());
        else if (count > size())
            insert(cend(), count - size(), value_type{});
    }

    /// Changes the number of elements stored
    constexpr void resize(size_type count, const value_type& value)
    {
        if (count < size())
            erase(cbegin() + (size() - count), cend());
        else if (count > size())
            insert(cend(), count - size(), value);
    }

    /// Swaps the contents of two vectors
    constexpr void swap(vector<value_type>& other) noexcept
    {
        _buf.swap(other._buf);
    }

    // -- Comparison

    constexpr bool operator==(const vector<T>& rhs) const
    {
        if (size() != rhs.size())
            return false;

        auto il = cbegin();
        for (auto ir = rhs.cbegin(); il!=cend() && (*il == *ir); ++il,++ir);
        return il == cend();
    }

    constexpr compare_three_way_result_t<T> operator<=>(const vector<T>& rhs) const
        requires three_way_comparable<T>
    {
        // First check size
        auto sc = size() <=> rhs.size();
        if (is_neq(sc))
            return sc;

        // Next check the elements
        auto il = cbegin();
        for (auto ir = rhs.cbegin(); il!=cend(); ++il,++ir) {
            auto ic = *il <=> *ir;
            if (is_neq(ic))
                return ic;
        }

        return strong_ordering::equal;
    }

protected:

    constexpr void erase_work(const_iterator pos, size_type count)
        noexcept(is_nothrow_move_assignable_v<value_type>)
    {
        size_type offset = static_cast<size_type>(sys::distance(cbegin(), pos));

        // Move the remaining elements down
        if (size_type to_move = size() - (offset + count)) {
            auto dst = &data()[offset];
            auto src = dst + count;

            for (size_type i = 0; i<to_move; ++i)
                *dst++ = sys::move(*src++);
        }

        // Destroy the elements
        auto doomed = &data()[size() - count];
        for (size_type i = 0; i<count; ++i)
            destruct_at(doomed++);

        // Finally update size
        _buf.dec_size(count);
    }

    template <class... Args>
        requires is_constructible_v<T, Args...>
    constexpr iterator insert_work(size_type offset, size_type count, Args&&... args)
    {
        if (count + size() > capacity()) {
            // grow_capacity_for_insert is optimized such that if we need
            // to grow, the underlying buffer object will construct the new
            // buffer with a "hole" where the inserted items will be. That
            // allows us to just construct the new items in place and not
            // have to move existing items to make room like we do in the
            // next case. This will set length to size() + count.
            grow_capacity_for_insert(calc_new_capacity(count), offset, count);
        }
        else {
            // Move all items in the insert region up. The 'old' and 'new'
            // regions can overlap so we need to iterate in reverse order.
            const auto move_count = size() - offset;            // Number of items to move
            const auto src_offset = offset + move_count - 1;    // Last element
            auto src = &data()[src_offset];
            auto dst = &data()[src_offset + count];

            for (size_type i = 0; i<move_count; ++i) {
                construct_at(dst--, sys::forward<T>(*src));     // Copy or move constructor
                destruct_at (src--);
            }

            // Adjust length
            _buf.inc_size(count);
        }

        constexpr bool no_try_block =
            is_trivially_destructible_v<value_type> ||
            is_nothrow_constructible_v <value_type, Args...>;

        // Insert the new items
        if constexpr (no_try_block) {
            auto dst = &data()[offset];
            for (size_type i = 0; i<count; ++i,++dst)
                construct_at(dst, sys::forward<Args>(args)...);
        }
        else {
            // In this case, the constructor could throw and we have a non
            // trivial destructor. If any constructor throws then we are
            // guaranteed to have at least one "hole" where no item has been
            // constructed. This can cause Bad Things™ when vector_buf tries
            // to destruct the object that doesn't exist in said hole.
            size_type i = 0;
            try {
                auto dst = &data()[offset];
                for (; i<count; ++i,++dst)
                    construct_at(dst, sys::forward<Args>(args)...);
            }
            catch(...) {

                //idx:0,...     w,...      x,...         y,...       z
                // { [pre-hole] [hole-obj] [hole-noinit] [post-hole] [excess-capacity] }
                //       ^           ^          ^             ^
                //       |           |          |             \-- Elements post hole
                //       |           |          \-- Uninitialized memory
                //       |           \-- Elements construced in try block above
                //       \-- Elements prior to hole
                //
                // w == offset, x == offset + i, y == offset + count, z == size()
                //

                // Destruct the hole objects we just constructed above [hole-obj]
                auto doomed = &data()[offset];
                for (size_type j=0; j<i; ++j,++doomed)
                    destruct_at(doomed);

                // Destruct previously constructed objects after the hole [post-hole]
                doomed = &data()[offset + count];
                auto post_hole_count = size() - (offset + count);
                for (size_type j=0; j<post_hole_count; ++j,++doomed)
                    destruct_at(doomed);

                // Truncate length to insertion point and re-throw. Normal
                // destruction will handle the rest.
                _buf.set_size(offset);
                throw;
            }
        }

        return iterator(data() + offset);
    }

    template <class... Args>
        requires is_constructible_v<T, Args...>
    constexpr iterator append_work(Args&&... args)
    {
        if (1 + size() > capacity())
            ensure_capacity(calc_new_capacity(1));
        auto ret = construct_at(data() + size(), sys::forward<Args>(args)...);
        _buf.inc_size();
        return iterator(ret);
    }

    /** Calculate new capacity for adding \p count new items
     *
     * This function is called when we've determined that adding one or more
     * elements will exceed the current capacity. It figures out what the
     * new capacity should be.
     *
     * This function does not adjust the capacity!
     *
     * @param add_count     The number of new items that we want to add
     *
     * @return Returns the size that we should increase the capacity to.
    */
    constexpr size_type calc_new_capacity(size_type add_count) const
    {
        // Make sure new minimum capacity isn't too large
        size_type cap_need{};
        if (sys::add_overflow(size(), add_count, cap_need) || (cap_need > max_size()))
            throw sys::error_length();
        if (cap_need <= capacity())
            return capacity();

        // Double capacity until we have enough to store cap_need
        size_type cap_try = capacity() * 2;
        if (0 == cap_try)
            cap_try = 1;
        while (cap_try < cap_need) {
            if (sys::multiply_overflow(cap_try, 2, cap_try))
                cap_try = cap_need;
        }

        return cap_try;
    }

    /// Ensure that our capacity is at least \p size items (append)
    constexpr void ensure_capacity(size_type new_cap)
    {
        if (new_cap > capacity()) {
            if constexpr (sys::is_nothrow_move_constructible_v<value_type>) {
                // Create a bigger buffer and move our content in.
                auto new_data = buffer_type{new_cap, sys::move(_buf)};
                _buf.swap(new_data);
            }
            else {
                // Create a bigger buffer and copy our content in. Need to
                // copy because moving might throw. If this throws, our
                // original vector is still intact and unaffected.
                auto new_data = buffer_type{new_cap, _buf};
                _buf.swap(new_data);
            }
        }
    }

    /** @brief Grow our capacity for an insertion operation
     *
     * This method is called to when our capacity needs to grow because we
     * are inserting one or more new items. It is optimized such that the
     * newly allocated buffer will contain a "hole" where the to-be-inserted
     * elements will go. This hole is just default initialized data, which
     * is to say no elements have been constructed there. This allows the
     * caller to just construct the inserted elements in place.
     *
     * @param new_cap   The new capacity to set
     * @param offset    The offset at which the to-be-inserted element(s)
     *  will be inserted.
     * @param count     The number of elements that will be inserted
     *
    */
    constexpr void grow_capacity_for_insert(size_type new_cap,
        size_type offset, size_type count)
    {
        if constexpr (sys::is_nothrow_move_constructible_v<value_type>) {
            // Create a bigger buffer and move our content in, but leave a
            // hole for the items we'll be inserting.
            auto new_data = buffer_type{new_cap, sys::move(_buf), offset, count};
            _buf.swap(new_data);
        }
        else {
            // Create a bigger buffer and copy our content in, but leave a
            // hole for the items we'll be inserting. Need to copy because
            // moving might throw. If this throws, our original vector is
            // still intact and unaffected.
            auto new_data = buffer_type{new_cap, _buf, offset, count};
            _buf.swap(new_data);
        }
    }

    /// Check position for a reference: [begin, end)
    constexpr void check_pos_ref(size_type pos) const
    {
        if (pos >= size())
            throw sys::error_bounds();
    }

private:

    buffer_type _buf;
};

_SYS_END_NS

#endif // ifndef sys_vector__included
