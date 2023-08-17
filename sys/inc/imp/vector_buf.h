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

    /** @brief Construct with a given capacity, moving items from other
     *
     * @param cap   The capacity that we should allocate for
     * @param other Another vector_buf from which we will move its elements
     *  into our own buffer. We're taking this parameter as an r-value
     *  reference mainly as a 'tag' to indicate we want to move its items.
     *  If we take it as a non-const l-value reference then it would be
     *  messy trying to disambiguate between this and the next contstructor,
     *  which does the same thing as this, but copies elements instead of
     *  moving them.
    */
    constexpr vector_buf(size_type cap, vector_buf&& other)
        : _buf(allocator().allocate(cap)), _cap(cap)
    {
        // assert(cap >= other.size())
        move_elements(other);
    }

    /// Construct with a given capacity, copying items from other
    constexpr vector_buf(size_type cap, const vector_buf& other)
        : _buf(allocator().allocate(cap)), _cap(cap)
    {
        // assert(cap >= other.size())
        copy_elements(other);
    }

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
     * @param cap           The capacity to use for this buffer. This value
     *  must not be less than other.size() + ins_count.
     * @param other         The source buffer from which to copy/move items
     * @param ins_offset    The offset at which to leave the hole
     * @param ins_count     The size of the hole in elements
    */
    template <class V>
        requires is_same_v<remove_cvref_t<V>, vector_buf<T>>
    constexpr vector_buf(size_type cap, V&& other, size_type ins_offset, size_type ins_count)
        : _buf(allocator().allocate(cap)), _cap(cap)
    {
        // assert (cap >= other.size() + ins_count)
        forward_elements_with_hole(sys::forward<V>(other),
            ins_offset, ins_count);
    }

    /// Copy constructor
    constexpr vector_buf(const vector_buf& other)
        : vector_buf(other.capacity(), other)
    { }

    /// Move constructor
    constexpr vector_buf(vector_buf&& other) noexcept
        : _buf(other._buf), _cap(other._cap), _len(other._len)
    {
        other._buf = nullptr;
        other._cap = 0;
        other._len = 0;
    }

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

        copy_elements(cpy);      // Will set _len

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

    // - Buffer length adjustment - assumes caller constructs/destructs items

    constexpr void set_size(size_type n)     noexcept { _len  = n; }
    constexpr void inc_size(size_type n = 1) noexcept { _len += n; }
    constexpr void dec_size(size_type n = 1) noexcept { _len -= n; }

protected:

    constexpr void copy_elements(const vector_buf& other)
    {
        // assert(capacity() >= other.size());

        // Update _len as we successfully construct items. This will ensure
        // that we destruct the appropriate number of items in our cleanup
        // if something throws.
        _len = 0;

        auto dst = data();
        auto src = other.data();
        for (size_type i=0; i<other.size(); ++i) {
            construct_at(dst++, *src++);
            ++_len;
        }
    }

    constexpr void move_elements(vector_buf& other)
    {
        // assert(capacity() >= other.size());

        // Update _len as we successfully construct items. This will ensure
        // that we destruct the appropriate number of items in our cleanup
        // if something throws.
        _len = 0;

        auto dst = data();
        auto src = other.data();
        for (size_type i=0; i<other.size(); ++i) {
            construct_at(dst++, sys::move(*src++));
            ++_len;
        }
    }

    template <class V>
        requires is_same_v<remove_cvref_t<V>, vector_buf<T>>
    constexpr void forward_elements_with_hole(V&& other,
        size_type ins_offset, size_type ins_count)
    {
        // Update _len as we successfully construct items. This will ensure
        // that we destruct the appropriate number of items in our cleanup
        // if something goes wrong.
        _len = 0;

        auto dst = data();
        auto src = other.data();
        size_type i = 0;

        // Forward items from source up to the new hole
        for (i=0; (i < other.size()) && (i != ins_offset); ++i) {
            construct_at(dst++, sys::forward<T>(*src++));
            ++_len;
        }

        // Jump over the memory where the caller will be constructing the
        // to-be-inserted items.
        if (i == ins_offset) {
            dst  += ins_count;
            _len += ins_count;
        }

        // Forward the remaining items after the hole
        for (; i < other.size(); ++i) {
            construct_at(dst++, sys::forward<T>(*src++));
            _len++;
        }
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

    pointer     _buf{nullptr};      ///< Buffer address
    size_type   _cap{0};            ///< Buffer capacity in elements
    size_type   _len{0};            ///< Number of elements in buffer
};

}
_SYS_END_NS

#endif // ifndef sys_vector_buf_included
