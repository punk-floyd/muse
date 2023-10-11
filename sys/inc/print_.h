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

_SYS_BEGIN_NS

/// Prints to standard output using formatted representation of the arguments
template <class... FmtArgs>
inline void print(sys::format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    io::ostream_iterator osi(*io::stout.get());
    format_to(osi, fmt, forward<FmtArgs>(args)...);
}

/// Prints to standard output using formatted representation of the arguments with a terminal newline
template <class... FmtArgs>
inline void println(sys::format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    print(fmt, forward<FmtArgs>(args)...);
    io::stout->out('\n');
}

_SYS_END_NS

#endif // ifndef sys_print__included
