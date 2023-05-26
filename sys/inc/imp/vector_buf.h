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

_SYS_BEGIN_NS
namespace imp {

template <class T>
class buffer {
public:

    using value_type = T;

    using size_type       = sys::size_t;
    using difference_type = sys::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;

    /// Construct default, empty buffer
    constexpr buffer() = default;

    /// Construct *uninitialized* buffer of given size
    constexpr explicit buffer(size_t n)
        : _buf(allocate(n)), _cap(n)
    { }

    constexpr ~buffer()
    {
        if (_buf) {
            // Destruct elements
            auto doomed = _buf + _len - 1;
            while (_len) {
                destruct_at(*doomed--);
                --_len;
            }
            // Free the memory
            deallocate(_buf);
        }
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

private:
    pointer     _buf{nullptr};
    size_type   _cap{0};
    size_type   _len{0};
};

}
_SYS_END_NS

#endif // ifndef sys_vector_buf_included
