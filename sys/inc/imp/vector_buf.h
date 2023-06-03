/**
 * @file    vector_buf.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Implements buffering for sys::vector
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_vector_buf_included
#define sys_vector_buf_included

#include <_core_.h>
#include <allocator_.h>
#include <memory_.h>

_SYS_BEGIN_NS
namespace imp {

template <class T>
class vector_buf {
public:

    using value_type = T;

    using size_type       = sys::size_t;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using allocator       = sys::allocator<value_type>;

    /// Construct default, empty buffer
    constexpr vector_buf() = default;

    /// Construct *uninitialized* buffer of given size
    constexpr explicit vector_buf(size_type cap)
        : _buf(allocator().allocate(cap)), _cap(cap)
    { }

    /// Construct with a given capacity, copying/moving items from other
    template <class V>
        requires is_same_v<remove_cvref_t<V>, vector_buf<T>>
    constexpr vector_buf(size_type cap, V&& other)
        : _buf(allocator().allocate(cap)), _cap(cap)
        , _len(sys::min(cap, other.size()))
    {
        forward_elements(sys::forward<V>(other));
    }

    /// Construct with a given capacity, forwarding items from other, but
    /// leaving a "hole" for to-be-inserted items. It is assumed that the
    /// new

    /** @brief Construct with \p cap capacity, forwarding items from other
     *   with "hole"
     *
     * This constructor is used when an insertion of one or more items into
     * the parent container will result in a reallocation. It's optimized
     * such that when we're copying/moving items from the source buffer,
     * the memory where the to-be-inserted items will be (which is defined
     * by the \p ins_offset and \p ins_count parameters) will be left in a
     * default initialized state so the new item(s) can be constructed in
     * place directly.
     *
     * @param cap           The capacity to use for this buffer
     * @param other         The source buffer from which to copy/move items
     * @param ins_offset    The offset at which to leave the hole
     * @param ins_count     The size of the hole in elements
    */
    template <class V>
        requires is_same_v<remove_cvref_t<V>, vector_buf<T>>
    constexpr vector_buf(size_type cap, V&& other, size_type ins_offset, size_type ins_count)
        : _buf(allocator().allocate(cap)), _cap(cap)
        , _len(sys::min(cap, other.size()))
    {
        forward_elements_with_hole(sys::forward<V>(other),
            ins_offset, ins_count);
    }

    /// Copy constructor
    constexpr vector_buf(const vector_buf& other)
        : vector_buf(other.capacity(), other)
    { }

    /// Move constructor
    constexpr vector_buf(vector_buf&& other) noexcept(is_nothrow_move_constructible_v<T>)
        : vector_buf(other.capacity(), sys::move(other))
    { }

    constexpr ~vector_buf() noexcept(is_nothrow_destructible_v<value_type>)
    {
        release();
    }

    /// Copy assignment
    constexpr vector_buf& operator=(const vector_buf& cpy)
    {
        release();

        _buf = allocator().allocate(cpy.capacity());
        _cap = cpy.capacity();
        _len = cpy.size();

        forward_elements(cpy);

        return *this;
    }

    /// Move assignment
    constexpr vector_buf& operator=(vector_buf&& cpy)
    {
        release();

        _buf = cpy._buf;
        _cap = cpy._cap;
        _len = cpy._len;

        cpy._buf = nullptr;
        cpy._cap = 0;
        cpy._len = 0;

        return *this;
    }

    /// Returns the current size of the buffer
    [[nodiscard]] constexpr size_type size() const noexcept
        { return _len; }
    /// Returns the current capacity of the buffer
    [[nodiscard]] constexpr size_type capacity() const noexcept
        { return _cap; }
    /// Direct access to the underlying data
    [[nodiscard]] constexpr pointer data() noexcept
        { return _buf; }
    /// Direct access to the underlying data
    [[nodiscard]] constexpr const_pointer data() const noexcept
        { return _buf; }

    /// Clear out all elements, leaving capacity unaffected
    constexpr void clear_elements() noexcept(is_nothrow_destructible_v<value_type>)
    {
        if (_buf)
            destruct_elements();
    }

    constexpr void swap(vector_buf& other) noexcept
    {
        sys::swap(_buf, other._buf);
        sys::swap(_cap, other._cap);
        sys::swap(_len, other._len);
    }

    constexpr void set_size(size_type n)     noexcept { _len  = n; }
    constexpr void inc_size(size_type n = 1) noexcept { _len += n; }
    constexpr void dec_size(size_type n = 1) noexcept { _len -= n; }

protected:

    template <class V>
        requires is_same_v<remove_cvref_t<V>, vector_buf<T>>
    constexpr void forward_elements(V&& other)
    {
        auto dst = data();
        auto src = other.data();
        for (size_type i=0; i<size(); ++i)
            construct_at(dst++, sys::forward<T>(*src++));
    }

    template <class V>
        requires is_same_v<remove_cvref_t<V>, vector_buf<T>>
    constexpr void forward_elements_with_hole(V&& other,
        size_type ins_offset, size_type ins_count)
    {
        auto dst = data();
        auto src = other.data();
        size_type i = 0;

        for (i=0; (i < size()) && (i != ins_offset); ++i)
            construct_at(dst++, sys::forward<T>(*src++));
        // I_AM_HERE

    }

    constexpr void release() noexcept(is_nothrow_destructible_v<value_type>)
    {
        if (_buf) {
            // Destruct elements and free memory
            destruct_elements();
            allocator().deallocate(_buf);

            _buf = nullptr;
            _cap = _len = 0;
        }
    }

    constexpr void destruct_elements() noexcept(is_nothrow_destructible_v<value_type>)
    {
        // Destroy in reverse order
        auto doomed = _buf + _len - 1;
        while (_len--)
            destruct_at(doomed--);
        _len = 0;
    }

private:

    pointer     _buf{nullptr};
    size_type   _cap{0};
    size_type   _len{0};
};

}
_SYS_END_NS

#endif // ifndef sys_vector_buf_included
