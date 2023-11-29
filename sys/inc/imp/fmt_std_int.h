/**
 * @file    fmt_std_int.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Standard formatter implementation for integral types
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <bit_.h>
#include "fmt_std.h"

_SYS_BEGIN_NS

// Char types for format. Note that signed char and unsigned char are not included!
template <class T> struct is_fmt_char_type : bool_constant<type_is_any_of_v<remove_cv_t<T>,
    char, wchar_t, char8_t, char16_t, char32_t>> {};

/// All integral types can use the standard formatter
template <class T>
    requires (integral<T> && !is_fmt_char_type<T>::value && !is_same_v<T, bool>)
struct formatter<T> : public formatter_std
{
    constexpr formatter()
    {
        // Tweak defaults for integral types
        get_format_spec().type_chars = "bBcdoxX";
        get_format_spec().type       = 'd';

        supports_sign           = true;
        supports_alt_form       = true;
        supports_leading_zeroes = true;
    }

    constexpr formatter(const format_spec_t& spec) noexcept
        : formatter_std(spec)
    {}

    template <class ParseCtx>
    constexpr auto parse(ParseCtx& p_ctx) -> ParseCtx::iterator
    {
        parse_std(p_ctx);
        return p_ctx.begin();
    }

    template <class FormatCtx>
    constexpr auto format(T val, FormatCtx& fmt_ctx) -> FormatCtx::iterator
    {
        const auto& fs = get_format_spec();

        // Resolve field width argument
        size_t width = fs.width_in_arg
            ? get_width_from_arg(fs.width, fmt_ctx) : fs.width;

        // Figure out base and base prefix
        string_view base_prefix{};
        unsigned base = 10;
        bool no_sign = false;
        switch (fs.type) {
        case 'b': base = 2;  base_prefix = "0b"; break;
        case 'B': base = 2;  base_prefix = "0B"; break;
        case 'c': no_sign = true; break;
        case 'd': break;
        case 'o': base = 8;  if (val) base_prefix = "0"; break;
        case 'x': base = 16; base_prefix = "0x"; break;
        case 'X': base = 16; base_prefix = "0X"; break;
        default: throw error_format("Bad format type");
        }

        // Declare a char buffer large enough to hold largest possible
        // number. This can vary from 1 byte (e.g., base-36 uint8_t) to
        // 128 bytes (e.g., base-2 uint128_t). So, worst case is 1 char
        // per 1 bit (base 2). Add one more char for potential sign.
        constexpr size_t num_buf_bytes = (sizeof(T) << 3) + 1;
        char num_buf[num_buf_bytes];
        string_view num_str{};

        if (fs.type == 'c') {
            if (val > numeric_limits<char>::max)
                throw error_format("Bad parameter for type 'c'");
            num_buf[0] = static_cast<char>(val);
            num_str = string_view(num_buf, 1);
        }
        else {
            auto&& [endp, ec] = to_chars(num_buf, num_buf + num_buf_bytes, val, base);
            if (is_error(ec))
                throw error_format("Failed to convert value"); // Shouldn't happen
            num_str = string_view(num_buf, endp - num_buf);
        }

        bool is_neg = false;
        if constexpr (is_signed_v<T>) {
            is_neg = (val < 0);
            if (is_neg && !no_sign)
                num_str.remove_prefix(1); // Lop off the sign
        }
        // TODO : Not yet handled: fs.use_locale (L) (locale specific digit grouping)

        // Determine length of value including any sign and base prefix
        size_t fld_len = num_str.length();
        if (fs.alt_form)                fld_len += base_prefix.length();
        if ((fs.sign != '-') || is_neg) fld_len += 1;

        auto it_out {fmt_ctx.out()};

        // Handle zero padding: [sign] [base-prefix] [0-pad] value
        if (fs.zero_pad && !fs.align) {
            size_t zeros = (width && fld_len < width) ? (width - fld_len) : 0;

            // [sign]
            if (!no_sign) {
                if (is_neg)
                    *it_out++ = '-';
                else if ((fs.sign == ' ') || (fs.sign == '+'))
                    *it_out++ = fs.sign;
            }

            // [base-prefix]
            if (fs.alt_form && !base_prefix.is_empty()) {
                for (auto c: base_prefix)
                    *it_out++ = c;
            }

            // [0-pad]
            while (zeros--)
                *it_out++ = '0';

            // value
            for (auto c : num_str)
                *it_out++ = c;
        }
        // Handle align/fill: [pre-fill] [sign] [base-prefix] value [post-fill]
        else {
            // Figure out alignment and fill
            size_t pre_fill = 0, post_fill = 0;
            if (width && (fld_len < width)) {
                size_t fill = width - fld_len;
                switch(fs.align) {
                    case '<': post_fill = fill; break;
                    case '^': pre_fill = fill >> 1; post_fill = fill - pre_fill; break;
                    default : pre_fill = fill;
                }
            }

            char fill_char = fs.fill ? fs.fill : ' ';

            // [pre-fill]
            while (pre_fill--)
                *it_out++ = fill_char;

            // [sign]
            if (!no_sign) {
                if (is_neg)
                    *it_out++ = '-';
                else if ((fs.sign == ' ') || (fs.sign == '+'))
                    *it_out++ = fs.sign;
            }

            // [base-prefix]
            if (fs.alt_form && !base_prefix.is_empty()) {
                for (auto c : base_prefix)
                    *it_out++ = c;
            }

            // value
            for (auto c : num_str)
                *it_out++ = c;

            // [post-fill]
            while (post_fill--)
                *it_out++ = fill_char;
        }

        return it_out;
    }
};

template <>
struct formatter<bool> : public formatter_std
{
    constexpr formatter()
    {
        // Tweak defaults for integral types
        get_format_spec().type_chars = "bBdosxX";
        get_format_spec().type       = 's';
    }

    template <class ParseCtx>
    constexpr auto parse(ParseCtx& p_ctx) -> ParseCtx::iterator
    {
        parse_std(p_ctx);
        return p_ctx.begin();
    }

    template <class FormatCtx>
    constexpr auto format(bool val, FormatCtx& fmt_ctx) -> FormatCtx::iterator
    {
        if (get_format_spec().type == 's') {
            auto it_out {fmt_ctx.out()};
            for (auto c : string_view{val ? "true" : "false"})
                *it_out++ = c;
            return it_out;
        }

        formatter<unsigned char> f{get_format_spec()};
        return f.format(static_cast<unsigned char>(val), fmt_ctx);
    }
};

// Formatter for nullptr_t or any pointer, except [const] char*; they're
// handled by the string code in fmt_std_str.h.
template <class T>
    requires is_same_v<T, nullptr_t> ||
        (is_pointer_v<T>
            && !is_same_v<remove_cvref_t<T>, const char*>
            && !is_same_v<remove_cvref_t<T>,       char*>)
struct formatter<T> : public formatter_std
{
    constexpr formatter()
    {
        // Tweak defaults for pointer types
        get_format_spec().type_chars = "p";
        get_format_spec().type       = 'p';
    }

    template <class ParseCtx>
    constexpr auto parse(ParseCtx& p_ctx) -> ParseCtx::iterator
    {
        parse_std(p_ctx);
        return p_ctx.begin();
    }

    template <class FormatCtx>
    constexpr auto format(T val, FormatCtx& fmt_ctx) -> FormatCtx::iterator
    {
        const unsigned long val_ul = bit_cast<unsigned long>(val);

        format_spec_t fs{get_format_spec()};
        fs.type = 'x';
        fs.alt_form = true;
        fs.zero_pad = true;
        if (!fs.width_in_arg && !fs.width)
            fs.width = sizeof(uintptr_t) << 1;

        formatter<unsigned long> f(fs);
        return f.format(val_ul, fmt_ctx);
    }
};

_SYS_END_NS
