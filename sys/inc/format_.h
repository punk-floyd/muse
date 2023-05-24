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

_SYS_BEGIN_NS

string vformat(string_view fmt, basic_format_args args)
{
    // TODO : Use a growable buffer here, then allocate string
    string ret;
    back_insert_iterator bit(ret);

    imp::parse_context pctx(args, bit);
    imp::format_parse(fmt, pctx);
    return ret;
}

template <class... FmtArgs>
inline string format(format_string<FmtArgs...> fmt, FmtArgs&&... args)
{
    format_arg_store<FmtArgs...> arg_store(forward<FmtArgs>(args)...);
    return vformat(fmt.get_view(), basic_format_args(arg_store));
}

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
