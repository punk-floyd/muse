/**
 * @file    fmt_std.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Standard formatter implementation
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <_core_.h>
#include <charconv_.h>
#include <limits_.h>
#include "fmt_core.h"

_SYS_BEGIN_NS

struct formatter_std
{
    using char_t = char;

    constexpr formatter_std() = default;

protected:

    /// True if format supports sign {:[+- ]}
    bool    supports_sign{false};
    /// True if format supports alternate form {:#}
    bool    supports_alt_form{false};
    /// True if format supports leading zeroes {:0}
    bool    supports_leading_zeroes{false};
    /// True if format supports precision {:.foo}
    bool    supports_precision{false};

    template <class ParseCtx>
    constexpr bool check_done(ParseCtx& p_ctx)
    {
        if (p_ctx.is_empty())
            throw error_format("Unterminated replacement field");

        if (*p_ctx.begin() == '}') {
            p_ctx.advance_by(1);
            return true;
        }

        return false;
    }

    template <class ParseCtx>
    constexpr bool advance_and_check_done(ParseCtx& p_ctx, size_t advance = 1)
    {
        p_ctx.advance_by(advance);
        return check_done(p_ctx);
    }

    template <class ParseCtx>
    constexpr bool parse_align_fill(ParseCtx& p_ctx)
    {
        // [[fill]align] | fill: [^{}] | align: [<>^]

        // [align]
        auto it = p_ctx.begin();
        const auto& ac = format_spec_t::align_chars;
        if (ac.find_first_of(*it) != string_view::npos) {
            get_format_spec().align = *it;
            return advance_and_check_done(p_ctx);
        }

        // [fill[align]]
        const auto fill = *it++;
        if ((it != p_ctx.end()) && (ac.find_first_of(*it) != string_view::npos)) {
            if ((fill == '{') || (fill == '}'))
                throw error_format("Invalid fill char");
            get_format_spec().fill  = fill;
            get_format_spec().align = *it;
            return advance_and_check_done(p_ctx, 2);
        }

        return false;
    }

    template <class ParseCtx>
    constexpr bool parse_sign(ParseCtx& p_ctx)
    {
        // [sign] | sign: [+- ]
        const auto it = p_ctx.begin();
        if (format_spec_t::sign_chars.find_first_of(*it) != string_view::npos) {
            if (!supports_sign)
                throw error_format("Sign {:[+- ]} not supported for this type");
            get_format_spec().sign = *it;
            return advance_and_check_done(p_ctx);
        }

        return false;
    }

    template <class ParseCtx>
    constexpr bool parse_alt_form(ParseCtx& p_ctx)
    {
        // [#] alternate form
        const auto it = p_ctx.begin();
        if (*it == format_spec_t::alt_form_char) {
            if (!supports_alt_form)
                throw error_format("Alternate form {:#} not supported for this type");
            get_format_spec().alt_form = true;
            return advance_and_check_done(p_ctx);
        }

        return false;
    }

    template <class ParseCtx>
    constexpr bool parse_leading_zeroes(ParseCtx& p_ctx)
    {
        // [0] leading zeros
        const auto it = p_ctx.begin();
        if (*it == format_spec_t::zero_pad_char) {
            if (!supports_leading_zeroes)
                throw error_format("Leading zeroes {:0} not supported for this type");
            get_format_spec().zero_pad = true;
            return advance_and_check_done(p_ctx);
        }

        return false;
    }

    template <class ParseCtx>
    constexpr bool parse_width(ParseCtx& p_ctx)
    {
        using ch_traits = string_view::traits_t;

        auto& fs = get_format_spec();

        // [width] | positive-decimal or nested rf ({} or {n})
        auto it = p_ctx.begin();
        if (ch_traits::is_digit_dec(*it)) {       // Explicit?
            auto [pos, ec] = from_chars(fs.width, p_ctx.get_fmt_str());
            if (is_error(ec) || (fs.width < 1))
                throw error_format("Invalid width specification");
            return advance_and_check_done(p_ctx, pos);
        }

        if (*it == '{') {                    // In nested rf?

            fs.width_in_arg = true;

            // Eat the '{'
            p_ctx.advance_by(1);
            if (++it == p_ctx.end())
                throw error_format("Incomplete width nested replacement field");

            // Check for argument index
            if (ch_traits::is_digit_dec(*it)) {
                auto [pos, ec] = from_chars(fs.width, p_ctx.get_fmt_str());
                if (is_error(ec))
                    throw error_format("Invalid width value argument index");
                p_ctx.advance_by(pos);

                // Validate the argument index
                p_ctx.check_arg_index(fs.width);
            }
            else {
                // Consume the next argument index
                fs.width = p_ctx.next_arg_index();
            }

            // Eat the '}'
            it = p_ctx.begin();
            if ((it == p_ctx.end()) || (*it != '}'))
                throw error_format("Incomplete width nested replacement field");
            return advance_and_check_done(p_ctx);
        }

        return false;
    }

    template <class ParseCtx>
    constexpr bool parse_precision(ParseCtx& p_ctx)
    {
        auto& fs = get_format_spec();

        // [precision]
        auto it = p_ctx.begin();
        if (*it != '.')
            return false;

        if (!supports_precision)
            throw error_format("Precision not supported for this type");

        fs.have_precision = true;

        if (advance_and_check_done(p_ctx))
            throw error_format("Invalid precision specification");

        if (string_view::traits_t::is_digit_dec(*it)) {       // Explicit?
            auto [pos, ec] = from_chars(fs.precision, p_ctx.get_fmt_str());
            if (is_error(ec) || (fs.precision < 0))
                throw error_format("Invalid precision specification");
            return advance_and_check_done(p_ctx, pos);
        }

        if (*it == '{') {                    // In nested rf?

            fs.prec_in_arg = true;

            // Eat the '{'
            p_ctx.advance_by(1);
            if (++it == p_ctx.end())
                throw error_format("Incomplete precision nested replacement field");

            // Check for argument index
            if (string_view::traits_t::is_digit_dec(*it)) {
                auto [pos, ec] = from_chars(fs.precision, p_ctx.get_fmt_str());
                if (is_error(ec))
                    throw error_format("Invalid precision value argument index");
                p_ctx.advance_by(pos);

                // Validate the argument index
                p_ctx.check_arg_index(fs.precision);
            }
            else {
                // Consume the next argument index
                fs.precision = p_ctx.next_arg_index();
            }

            // Eat the '}'
            it = p_ctx.begin();
            if ((it == p_ctx.end()) || (*it != '}'))
                throw error_format("Incomplete precision nested replacement field");
            return advance_and_check_done(p_ctx);
        }

        throw error_format("Invalid precision specification");
    }

    template <class ParseCtx>
    constexpr bool parse_locale_specific(ParseCtx& p_ctx)
    {
        // [L] locale-specific formatting
        if (*p_ctx.begin() == format_spec_t::use_locale_char) {
            get_format_spec().use_locale = true;
            return advance_and_check_done(p_ctx);
        }

        return false;
    }

    template <class ParseCtx>
    constexpr bool parse_type(ParseCtx& p_ctx)
    {
        // [type] | type: [aAbBcdeEfFgGopsxX] or subset thereof
        auto it = p_ctx.begin();
        if (get_format_spec().type_chars.find_first_of(*it) != string_view::npos) {
            get_format_spec().type = *it;
            return advance_and_check_done(p_ctx);
        }

        return false;
    }

    template <class ParseCtx>
    constexpr void parse_std(ParseCtx& p_ctx)
    {
        if (check_done(p_ctx))              return;
        if (parse_align_fill(p_ctx))        return;
        if (parse_sign(p_ctx))              return;
        if (parse_alt_form(p_ctx))          return;
        if (parse_leading_zeroes(p_ctx))    return;
        if (parse_width(p_ctx))             return;
        if (parse_precision(p_ctx))         return;
        if (parse_locale_specific(p_ctx))   return;
        if (parse_type(p_ctx))              return;

        // Anything remaining is a format error
        if (!check_done(p_ctx))
            throw error_format("Missing terminal replacement field terminator");
    }


    // Pull width from a format argument
    template <class FormatCtx>
    size_t get_width_from_arg(size_t arg_idx, FormatCtx& f_ctx)
    {
        const auto varg = f_ctx.get_arg(arg_idx);
        if (varg.template holds_alternative<unsigned>())
            return varg.template get<unsigned>();
        if (varg.template holds_alternative<int>()) {
            int width = varg.template get<int>();
            if (width < 0)
                throw error_format("Invalid width argument value; must be non-negative");
            return static_cast<size_t>(width);
        }

        throw error_format("Invalid type for width argument index; must be int or unsigned int");
    }

    // Pull precision from a format argument
    template <class FormatCtx>
    static size_t get_precision_from_arg(size_t arg_idx, FormatCtx& f_ctx)
    {
        auto varg = f_ctx.get_arg(arg_idx);
        if (varg.template holds_alternative<unsigned>())
            return varg.template get<unsigned>();
        if (varg.template holds_alternative<int>()) {
            int precision = varg.template get<int>();
            if (precision < 0)
                throw error_format("Invalid precision argument value; must be non-negative");
            return static_cast<size_t>(precision);
        }

        throw error_format("Invalid type for precision argument index; must be int or unsigned int");
    }

    // [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
    // fill:  [^{}] == anything but { or }
    // align: [<>^]
    // sign:  [+- ]
    // type:  [aAbBcdeEfFgGopsxX] depending on context

    struct format_spec_t {

        static constexpr string_view align_chars{"<>^"};
        static constexpr string_view sign_chars{"-+ "};
        static constexpr char_t      alt_form_char{'#'};
        static constexpr char_t      zero_pad_char{'0'};
        static constexpr char_t      use_locale_char{'L'};

        // All defined type chars; subclass may whittle down
        string_view type_chars{"aAbBcdeEfFgGopsxX"};

        size_t      width{0};           // Minimum field width
        size_t      precision{0};       // precision

        char_t      fill{0};            // Fill char; anything but { or }
        char_t      align{0};           // Alignment: < > ^
        char_t      sign{'-'};          // Sign for numbers: + - ' '
        char_t      type{0};            // How data shall be presented

        bool        alt_form{false};    // #
        bool        zero_pad{false};    // 0
        bool        use_locale{false};  // L
        bool        width_in_arg{false};
        bool        prec_in_arg{false};
        bool        have_precision{false};
    };

    constexpr const format_spec_t& get_format_spec() const noexcept { return _spec; }
    constexpr       format_spec_t& get_format_spec()       noexcept { return _spec; }

    constexpr formatter_std(const format_spec_t& spec) noexcept
        : _spec(spec)
    {}

private:

    format_spec_t   _spec{};
};

_SYS_END_NS
