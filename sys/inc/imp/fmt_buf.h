/**
 * @file    fmt_buf.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Declares sys::imp::format_buf; stack buffer that grows into heap buffer
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_fmt_buf_included
#define sys_imp_fmt_buf_included

#include <_core_.h>
#include <string_.h>
#include <limits_.h>

_SYS_BEGIN_NS
namespace imp {

template <size_t InitCapacity = 512>
class fmt_buf
{
public:

    using char_t         = string::char_t;
    using size_type      = string::size_type;
    using value_type     = string::value_type;

    fmt_buf() = default;

    constexpr size_type length()   const noexcept
        { return _have_str ? _str.length() : _auto_len; }

    constexpr void push_back(char_t ch)
    {
        if (!_have_str) {
            if (_auto_len < InitCapacity) {
                _auto_buf[_auto_len++] = ch;
                return;
            }

            // We generally double capacity when growing, so we'll do that
            // here, too.
            size_type new_cap;
            auto overflowed = add_overflow(InitCapacity, InitCapacity*2, new_cap);
            if (overflowed || (new_cap > string::max_size())) [[unlikely]]
                new_cap = string::max_size();
            _str.reserve(new_cap);
            _str.assign(_auto_buf, InitCapacity);
            _have_str = true;

            // Fall through
        }

        _str.push_back(ch);
    }

private:

    char_t      _auto_buf[InitCapacity];
    size_type   _auto_len{0};

    bool        _have_str{false};       // placeholder for optional bool
    string      _str;                   // TODO: optional<string>
};

} // end namespace imp
_SYS_END_NS

#endif // ifndef sys_imp_fmt_buf_included
