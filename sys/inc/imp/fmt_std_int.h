/**
 * @file    fmt_std_int.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Standard formatter implementation for integral types
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_fmt_std_int_included
#define sys_imp_fmt_std_int_included

#include <bit_.h>

_SYS_BEGIN_NS

// Char types for format. Note that signed char and unsigned char are not included!
template <class T> struct is_fmt_char_type : bool_constant<type_is_any_of_v<remove_cv_t<T>,
    char, wchar_t, char8_t, char16_t, char32_t>> {};

/// All integral types can use the standard formatter
template <class T>
    requires (integral<T> && !is_fmt_char_type<T>::value && !is_same_v<T, bool>)
struct formatter<T> : public formatter_std
{
    using my_base = formatter_std;

    constexpr formatter()
    {
        // Tweak defaults for integral types
        this->get_format_spec().type_chars = "bBcdoxX";
        this->get_format_spec().type       = 'd';
    }

    constexpr bool supports_sign() const noexcept override
        { return true; }
    constexpr bool supports_alt_form() const noexcept override
        { return true; }
    constexpr bool supports_leading_zeroes() const noexcept override
        { return true; }

    static void format_work(const format_spec_t& fs, const format_arg& fmt_arg, format_context& fmt_ctx)
    {
        auto get_val = [](const format_arg& arg){
            if constexpr (is_unsigned_v<T>) {
                if      constexpr (sizeof(T) ==  1) return arg.get_variant().get<uint8_t>();
                else if constexpr (sizeof(T) ==  2) return arg.get_variant().get<uint16_t>();
                else if constexpr (sizeof(T) ==  4) return arg.get_variant().get<uint32_t>();
                else if constexpr (sizeof(T) ==  8) return arg.get_variant().get<uint64_t>();
                else if constexpr (sizeof(T) == 16) return arg.get_variant().get<uint128_t>();
                else static_assert(dependent_false_v<T>, "Unsigned integral type is too large");
            }
            else if constexpr (is_signed_v<T>) {
                if      constexpr (sizeof(T) ==  1) return arg.get_variant().get<sint8_t>();
                else if constexpr (sizeof(T) ==  2) return arg.get_variant().get<sint16_t>();
                else if constexpr (sizeof(T) ==  4) return arg.get_variant().get<sint32_t>();
                else if constexpr (sizeof(T) ==  8) return arg.get_variant().get<sint64_t>();
                else if constexpr (sizeof(T) == 16) return arg.get_variant().get<sint128_t>();
                else static_assert(dependent_false_v<T>, "Signed integral type is too large");
            }
            else {
                static_assert(dependent_false_v<T>);
            }
        };

        auto val = get_val(fmt_arg);

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

        // Declare a char buffer large enough to hold largest possibe
        // number. This can vary from 1 byte (e.g., base-36 uint8_t) to
        // 128 bytes (e.g., base-2 uint128_t). So, worst case is 1 char
        // per 1 bit (base 2). Add one more char for potentnial sign.
        constexpr size_t numbuf_bytes = (sizeof(T) << 3) + 1;
        char numbuf[numbuf_bytes];
        string_view numstr{};

        if (fs.type == 'c') {
            if (val > numeric_limits<char>::max)
                throw error_format("Bad parameter for type 'c'");
            numbuf[0] = static_cast<char>(val);
            numstr = string_view(numbuf, 1);
        }
        else {
            auto&& [endp, ec] = to_chars(numbuf, numbuf + numbuf_bytes, val, base);
            if (is_error(ec))
                throw error_format("Failed to convert value"); // Shouldn't happen
            numstr = string_view(numbuf, endp - numbuf);
        }

        bool is_neg = false;
        if constexpr (is_signed_v<T>) {
            is_neg = (val < 0);
            if (is_neg && !no_sign)
                numstr.remove_prefix(1); // Lop off the sign
        }

        // TODO : Not yet handled: fs.use_locale (L) (locale specific digit grouping)

        formatter_sink& sink = fmt_ctx.get_sink();

        // Determine length of value including any sign and base prefix
        size_t fld_len = numstr.length();
        if (fs.alt_form)                fld_len += base_prefix.length();
        if ((fs.sign != '-') || is_neg) fld_len += 1;

        // Handle zero padding: [sign] [base-prefix] [0-pad] value
        if (fs.zero_pad && !fs.align) {
            size_t zeros = (width && fld_len < width) ? (width - fld_len) : 0;

            // [sign]
            if (!no_sign) {
                if (is_neg)
                    sink.output('-');
                else if ((fs.sign == ' ') || (fs.sign == '+'))
                    sink.output(fs.sign);
            }

            // [base-prefix]
            if (fs.alt_form && !base_prefix.is_empty())
                sink.output(base_prefix);

            // [0-pad]
            while (zeros--)
                sink.output('0');

            // value
            sink.output(numstr);
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
                sink.output(fill_char);

            // [sign]
            if (!no_sign) {
                if (is_neg)
                    sink.output('-');
                else if ((fs.sign == ' ') || (fs.sign == '+'))
                    sink.output(fs.sign);
            }

            // [base-prefix]
            if (fs.alt_form && !base_prefix.is_empty())
                sink.output(base_prefix);

            // value
            sink.output(numstr);

            // [post-fill]
            while (post_fill--)
                sink.output(fill_char);
        }
    }

    /// Format output into given sink
    void format(const format_arg& fmt_arg, format_context& fmt_ctx) override
    {
        format_work(this->get_format_spec(), fmt_arg, fmt_ctx);
    }
};

template <>
struct formatter<bool> final : public formatter_std
{
    using my_base = formatter_std;

    constexpr formatter()
    {
        // Tweak defaults for integral types
        this->get_format_spec().type_chars = "bBdosxX";
        this->get_format_spec().type       = 's';
    }

    /// Format output into given sink
    void format(const format_arg& fmt_arg, format_context& fmt_ctx) override
    {
        // The format argument value
        auto val = fmt_arg.get_variant().template get<bool>();

        if (get_format_spec().type == 's')
            fmt_ctx.get_sink().output(val ? "true" : "false");
        else {
            format_arg uc_arg{static_cast<unsigned char>(val)};
            const auto& fs = this->get_format_spec();
            formatter<unsigned char>::format_work(fs, uc_arg, fmt_ctx);
        }
    }
};

// Formatter for nullptr_t or any pointer, except [const] char*; they're
// handled by the string code in fmt_std_str.h.
template <class T>
    requires is_same_v<T, nullptr_t> ||
        (is_pointer_v<T>
            && !is_same_v<remove_cvref_t<T>, const char*>
            && !is_same_v<remove_cvref_t<T>,       char*>)
struct formatter<T> final : public formatter_std
{
    using my_base = formatter_std;

    constexpr formatter()
    {
        // Tweak defaults for pointer types
        this->get_format_spec().type_chars = "p";
        this->get_format_spec().type       = 'p';
    }

    /// Format output into given sink
    constexpr void format(const format_arg& fmt_arg, format_context& fmt_ctx) override
    {
        // TODO : Fix this up when we have "visit" capabilities
        format_arg up_arg;
        if constexpr (is_same_v<remove_cvref_t<T>, nullptr_t>)
            up_arg = format_arg{bit_cast<unsigned long>(fmt_arg.get_variant().get<nullptr_t>())};
        else
            up_arg = format_arg{bit_cast<unsigned long>(fmt_arg.get_variant().get<const void*>())};

        format_spec_t fs{this->get_format_spec()};
        fs.type = 'x';
        fs.alt_form = true;
        fs.zero_pad = true;
        if (!fs.width_in_arg && !fs.width)
            fs.width = sizeof(uintptr_t) << 1;
        formatter<unsigned long>::format_work(fs, up_arg, fmt_ctx);
    }
};

_SYS_END_NS

#endif // ifndef sys_imp_fmt_std_int_included
