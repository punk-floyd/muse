/**
 * @file    format_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Formatted output: sys::fmt*
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <_core_.h>
#include "imp/fmt_core.h"
#include "imp/fmt_std.h"
#include "imp/fmt_std_int.h"
#include "imp/fmt_std_str.h"
#include "imp/fmt_buf.h"

_SYS_BEGIN_NS

/// Non-template variant of format using type-erased argument representation
string vformat(string_view fmt, const format_args& args)
{
    imp::fmt_buf buf;
    format_context f_ctx{args, back_insert_iterator(buf)};
    parse_context  p_ctx{fmt, args.count()};
    imp::do_format(p_ctx, f_ctx);
    return buf.release_string();
}

/// Non-template variant of format_to using type-erased argument representation
template <class OutputIt>
OutputIt vformat_to(OutputIt out, string_view fmt,
    const basic_format_args<basic_format_context<OutputIt>>& args)
{
    basic_format_context<OutputIt> f_ctx(args, move(out));
    parse_context p_ctx(fmt, args.count());
    imp::do_format(p_ctx, f_ctx);
    return f_ctx.out();
}

/// Stores formatted representation of the arguments in a new string
template <class... FmtArgs>
inline string format(format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    return vformat(fmt.get_view(), make_format_args(args...));
}

/// Writes out formatted representation of its arguments through an output iterator
template <class OutputIt, class... FmtArgs>
inline OutputIt format_to(OutputIt out, format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    return vformat_to(move(out), fmt.get_view(),
        make_format_args<basic_format_context<OutputIt>, FmtArgs...>(args...));
}

/// Determines the number of characters necessary to store formatted output
template <class... FmtArgs>
inline size_t formatted_size(format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    return format_to(count_insert_iterator<char>{}, fmt, forward<FmtArgs>(args)...).get_count();
}

_SYS_END_NS
