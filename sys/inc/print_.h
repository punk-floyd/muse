/**
 * @file    print_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Implements sys::print and sys::println
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_print__included
#define sys_print__included

#include <_core_.h>
#include <format_.h>
#include <io_ostream_.h>
#include <string_view_.h>

_SYS_BEGIN_NS

/// Print an formatted string to standard output
template <class... FmtArgs>
inline void print(format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    io::ostream_iterator osi(*io::stout.get());
    format_to(osi, fmt, forward<FmtArgs>(args)...);
}

/// Print a formatted string to standard output with trailing newline
template <class... FmtArgs>
inline void println(format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    print(fmt, forward<FmtArgs>(args)...);
    io::stout->out('\n');
}

/// Print an unformatted string to standard output
template <string_view_convertible Viewable>
inline void print_str(Viewable view)
{
    io::stout->out(view);
}

/// Print an unformatted string to standard output with trailing newline
template <string_view_convertible Viewable>
inline void println_str(Viewable view)
{
    io::stout->out(view);
    io::stout->out('\n');
}

_SYS_END_NS

#endif // ifndef sys_print__included
