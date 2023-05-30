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
    constexpr explicit vector_buf(size_t cap)
        : _buf(allocator().allocate(cap)), _cap(cap)
    { }

    /// Construct with a given capacity, copying/moving items from other
    constexpr vector_buf(size_t cap, vector_buf<T>&& other)
        : _buf(allocator().allocate(cap)), _cap(cap)
        , _len(sys::min(cap, other.length()))
    {
        copy_or_move_elements(sys::forward<vector_buf<T>&&>(other));
    }

    constexpr ~vector_buf()
    {
        release();
    }

    /// Copy constructor
    constexpr vector_buf(const vector_buf& other)
        : vector_buf(other.capacity(),  other)
    { }

    /// Move constructor
    constexpr vector_buf(vector_buf&& other) noexcept(is_nothrow_move_constructible_v<T>)
        : vector_buf(other.capacity(), sys::move(other))
    { }

    /// Copy assignment
    constexpr vector_buf& operator=(const vector_buf& cpy)
    {
        release();

        _buf = allocator().allocate(cpy.capacity());
        _cap = cpy.capacity();
        _len = cpy.length();

        copy_or_move_elements(cpy);

        return *this;
    }

    /// Returns the current length of the buffer
    [[nodiscard]] constexpr size_type length() const noexcept
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

    constexpr void swap(vector_buf& other) noexcept
    {
        sys::swap(_buf, other._buf);
        sys::swap(_cap, other._cap);
        sys::swap(_len, other._len);
    }

    constexpr void set_length(size_type n)
    {
        _len = n;
    }

protected:

    constexpr void copy_or_move_elements(vector_buf<T>&& other)
    {
        auto dst = data();
        auto src = other.data();
        for (size_type i=0; i<length(); ++i)
            construct_at(dst++, sys::forward<T>(*src++));
    }

    constexpr void release()
    {
        if (_buf) {
            // Destruct elements and free memory
            destruct_elements();
            allocator().deallocate(_buf);

            _buf = nullptr;
            _cap = _len = 0;
        }
    }

    constexpr void destruct_elements()
    {
        // Destroy in reverse order
        auto doomed = _buf + _len - 1;
        while (_len--)
            destruct_at(doomed--);
    }

private:

    pointer     _buf{nullptr};
    size_type   _cap{0};
    size_type   _len{0};
};

}
_SYS_END_NS

#endif // ifndef sys_vector_buf_included
