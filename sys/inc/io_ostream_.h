/**
 * @file    io_ostream_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Defines sys::io::ostream
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_io_ostream_included
#define sys_io_ostream_included

#include <io_.h>
#include <string_view_.h>
#include <type_traits_.h>
#include <iterator_.h>

namespace sys { class string; };

_SYS_IO_BEGIN_NS

class ostream
{
public:

    using char_t = char;

    constexpr ostream() = default;

    template <class T>
        requires (sys::is_convertible_v<T, sys::string_view>)
    ostream& out(T&& t)
    {
        sys::string_view sv(forward<T>(t));
        sink(sv.data(), sv.length());   // MOOMOO failure
        return *this;
    }

    ostream& out(char_t ch)
    {
        sink(&ch, 1);
        return *this;
    }

    // -- Implementation

    virtual ~ostream() {}

protected:

    // - Virtual overrides

    /// Output given buffer to sink
    virtual bool sink(const char_t* data, size_t length) = 0;
};

/// Output iterator that appends elements into a stream
template <class Stream>
struct ostream_iterator
{
    using iterator_category = tag_iterator_output;
    using value_type        = typename Stream::char_t;
    using difference_type   = sys::ptrdiff_t;
    using pointer           = void;
    using reference         = void;
    using container_type    = void;

    constexpr explicit ostream_iterator(Stream& stream) noexcept
        : _stream(stream)
    {}

    constexpr ostream_iterator& operator=(const value_type& value)
        { _stream.get().out(value); return *this; }

    constexpr ostream_iterator& operator*() noexcept
        { return *this; }
    constexpr ostream_iterator& operator++() noexcept
        { return *this; }
    constexpr ostream_iterator operator++(int) noexcept
        { return *this; }

protected:

    ref_wrap<Stream>    _stream;
};

_SYS_IO_END_NS

#endif // ifndef sys_io_ostream_included
