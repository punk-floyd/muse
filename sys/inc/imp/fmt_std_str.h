/**
 * @file    fmt_std_str.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Standard formatter implementation for string types
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_fmt_std_str_included
#define sys_imp_fmt_std_str_included

#include <_core_.h>
#include <string_view_.h>
#include <imp/fmt_std.h>

_SYS_BEGIN_NS

template<>
struct formatter<string_view> : formatter_std
{
    using my_base = formatter_std;

    // Note: Ultimately, UTF-8 will be the assumed encoding here. As of now,
    //       we're assuming plain old ASCII.

    constexpr formatter()
    {
        get_format_spec().type_chars = "s?";
        get_format_spec().type       = 's';
    }

    constexpr virtual bool supports_precision() const noexcept
        { return true; }

    void sink_escaped_output(string_view val, formatter_sink& sink) const
    {
        sink.output('"');

        // TEMP: For now. Assuming ASCII encoding here.
        for (const auto& c : val) {
            switch(c) {
                case '\t': sink.output("\\t");  break;
                case '\n': sink.output("\\n");  break;
                case '\r': sink.output("\\r");  break;
                case '"' : sink.output("\\\""); break;
                case '\\': sink.output("\\\\"); break;
                default:   sink.output(c);      break;
            }
        }

        sink.output('"');
    }

    size_t get_escaped_output_length(string_view sv) const
    {
        // This is probably totally wrong for utf-8 code. Here,
        // we're counting chars, but the use below is to count
        // the width.

        count_insert_iterator<string_view::char_t> counter;
        imp::format_sink_imp sink(counter);

        sink_escaped_output(sv, sink);
        return counter.get_count();
    }

    /// Format output into given sink
    void format(const format_arg& fmt_arg, format_context& fmt_ctx) override
    {
        // The format argument value
        auto val = fmt_arg.get_variant().get<string_view>();

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

        formatter_sink& sink = fmt_ctx.get_sink();

        // TODO UTF-8 : This will eventually be assuming utf-8 encoding.
        //              For now, it's assuming plain old ASCII.

        // If we don't have a minimum field width or maximum precision then
        // just dump string and go.
        if (!width && !fs.have_precision) {
            if (escaped_output)
                sink_escaped_output(val, sink);
            else
                sink.output(val);
            return;
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
            sink.output(fill_char);

        // value
        if (escaped_output)
            sink_escaped_output(val, sink);
        else
            sink.output(val);

        // [post-fill]
        while (post_fill--)
            sink.output(fill_char);
    }
};

_SYS_END_NS

#endif // ifndef sys_imp_fmt_std_str_included
