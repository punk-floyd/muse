/**
 * @file    format_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Formatted output: sys::fmt*
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_format__included
#define sys_format__included

#include <string_view_.h>
#include <iterator_.h>
#include <charconv_.h>
#include <variant_.h>
#include <string_.h>
#include <array_.h>

#include "imp/fmt_base.h"
#include "imp/fmt_std.h"
#include "imp/fmt_args.h"
#include "imp/fmt_parse.h"
#include "imp/fmt_buf.h"

_SYS_BEGIN_NS

/// Non-template variant of sys::format using type-erased argument representation
string vformat(string_view fmt, basic_format_args args)
{
    imp::fmt_buf buf;
    back_insert_iterator bit(buf);

    imp::parse_context pctx(args, bit);
    imp::format_parse(fmt, pctx);
    return buf.release_string();
}

/// Stores formatted representation of the arguments in a new string
template <class... FmtArgs>
inline string format(format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    format_arg_store<FmtArgs...> arg_store(forward<FmtArgs>(args)...);
    return vformat(fmt.get_view(), basic_format_args(arg_store));
}

template <class OutputIt, class... FmtArgs>
inline OutputIt format_to(OutputIt it, sys::format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    format_arg_store<FmtArgs...> arg_store(forward<FmtArgs>(args)...);
    basic_format_args fmt_args(arg_store);

    imp::parse_context pctx(fmt_args, it);
    imp::format_parse(fmt, pctx);

    return it;
}

/// Determines the number of characters necessary to store formatted output
template <class... FmtArgs>
inline size_t formatted_size(format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    format_arg_store<FmtArgs...> arg_store(forward<FmtArgs>(args)...);
    basic_format_args fmt_args(arg_store);

    count_insert_iterator<char> counter;
    imp::parse_context pctx(fmt_args, counter);

    imp::format_parse(fmt, pctx);
    return counter.get_count();
}

_SYS_END_NS

#endif // ifndef sys_format__included
