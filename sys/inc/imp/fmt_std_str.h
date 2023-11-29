/**
 * @file    fmt_std_str.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Standard formatter implementation for string types
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <string_view_.h>
#include "fmt_std.h"

_SYS_BEGIN_NS

template<string_view_convertible T>
struct formatter<T> : formatter_std
{
    // Note: Ultimately, UTF-8 will be the assumed encoding here. As of now,
    //       we're assuming plain old ASCII.

    constexpr formatter()
    {
        get_format_spec().type_chars = "s?";
        get_format_spec().type       = 's';

        supports_precision = true;
    }

    template <class ParseCtx>
    constexpr auto parse(ParseCtx& p_ctx) -> ParseCtx::iterator
    {
        parse_std(p_ctx);
        return p_ctx.begin();
    }

    template <class OutputIt>
    constexpr OutputIt sink_string(OutputIt it, string_view sv) const
    {
        for (const auto c : sv)
            *it++ = c;

        return it;
    }

    template <class OutputIt>
    constexpr OutputIt sink_escaped_output(OutputIt it, string_view sv) const
    {
        OutputIt it_out = move(it);

        *it_out++ = '"';

        // TEMP: For now. Assuming ASCII encoding here.
        for (const auto c : sv) {
            switch(c) {
                case '\t': it = sink_string(move(it), "\\t");  break;
                case '\n': it = sink_string(move(it), "\\n");  break;
                case '\r': it = sink_string(move(it), "\\r");  break;
                case '"' : it = sink_string(move(it), "\\\""); break;
                case '\\': it = sink_string(move(it), "\\\\"); break;
                default:   *it_out++ = c;                      break;
            }
        }

        *it_out++ = '"';

        return it_out;
    }

    size_t get_escaped_output_length(string_view sv) const
    {
        // This is probably totally wrong for utf-8 code. Here,
        // we're counting chars, but the use below is to count
        // the width.

        count_insert_iterator<string_view::char_t> counter;
        return sink_escaped_output(move(counter), sv).get_count();
    }

    template <class FormatCtx>
    constexpr auto format(T val_raw, FormatCtx& fmt_ctx) -> FormatCtx::iterator
    {
        // The format argument value
        sys::string_view val{val_raw};

        // Resolve output type
        const auto& fs = get_format_spec();
        bool escaped_output = false;
        switch (fs.type) {
            case 's': break;
            case '?': escaped_output = true; break;
            default: throw error_format("Bad format type");
        }

        // Resolve field width/precision arguments
        size_t width = fs.width_in_arg
            ? get_width_from_arg(fs.width, fmt_ctx) : fs.width;
        size_t precision = fs.have_precision ? fs.prec_in_arg
            ? get_precision_from_arg(fs.precision, fmt_ctx) : fs.precision : 0;

        auto it_out = fmt_ctx.out();

        // TODO UTF-8 : This will eventually be assuming utf-8 encoding.
        //              For now, it's assuming plain old ASCII.

        // If we don't have a minimum field width or maximum precision then
        // just dump string and go.
        if (!width && !fs.have_precision) {
            if (escaped_output)
                return sink_escaped_output(fmt_ctx.out(), val);
            else
                return sink_string(fmt_ctx.out(), val);
        }

        // -- ASCII
        size_t fld_len = val.length();
        if (fs.have_precision && (fld_len > precision))
            val.truncate(fld_len = precision);

        if (escaped_output) {
            // Get the length of the escaped output so we can properly align it
            fld_len = get_escaped_output_length(val);
        }

        // Figure out alignment and fill
        size_t pre_fill = 0, post_fill = 0;
        if (width && (fld_len < width)) {
            size_t fill = width - fld_len;
            switch (fs.align) {
                default : post_fill = fill; break;
                case '^': pre_fill = fill >> 1; post_fill = fill - pre_fill; break;
                case '>': pre_fill = fill;
            }
        }

        char fill_char = fs.fill ? fs.fill : ' ';

        // [pre-fill]
        while (pre_fill--)
            *it_out++ = fill_char;

        // value
        if (escaped_output)
            it_out = sink_escaped_output(move(it_out), val);
        else
            it_out = sink_string(move(it_out), val);

        // [post-fill]
        while (post_fill--)
            *it_out++ = fill_char;

        return it_out;
    }
};

_SYS_END_NS
