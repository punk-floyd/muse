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
        sys::string_view sv(sys::move(t));
        sink(sv.data(), sv.length());   // MOOMOO failure
        return *this;
    }

    // -- Implementation

    virtual ~ostream() {}

protected:

    // - Virtual overrides

    /// Output given buffer to sink
    virtual bool sink(const char_t* data, size_t length) = 0;

};

_SYS_IO_END_NS

#endif // ifndef sys_io_ostream_included
