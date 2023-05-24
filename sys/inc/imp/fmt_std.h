/**
 * @file    fmt_std.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Standard formatter implementation
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_fmt_formatter_included
#define sys_imp_fmt_formatter_included

#include <_core_.h>
#include <charconv_.h>
#include <limits_.h>
#include "fmt_base.h"
#include "fmt_parse.h"
#include "fmt_args.h"

_SYS_BEGIN_NS

struct formatter_std : public formatter_base
{
    using char_t = char;

    constexpr formatter_std() = default;

protected:

    /// Returns true if format supports sign {:[+- ]}
    constexpr virtual bool supports_sign() const noexcept
        { return false; }
    /// Returns true if format supports alternate form {:#}
    constexpr virtual bool supports_alt_form() const noexcept
        { return false; }
    /// Returns true if format supports leading zeroes {:0}
    constexpr virtual bool supports_leading_zeroes() const noexcept
        { return false; }
    /// Returns true if format supports precision {:.foo}
    constexpr virtual bool supports_precision() const noexcept
        { return false; }

    constexpr void parse(string_view rf, parse_context_base& pctx) override
    {
        using ch_traits = string_view::traits_t;

        constexpr auto npos = string_view::npos;

        if (rf.is_empty())
            return;     // Use defaults

        auto& fs = get_format_spec();

        // [[fill]align] | fill: [^{}] | align: [<>^]
        const auto& ac = format_spec_t::align_chars;
         if (ac.find_first_of(rf[0]) != npos)
            fs.align = rf.pop_front();  // [align]
        else if ((rf.length() > 1) && (ac.find_first_of(rf[1]) != npos)) {
            // [fill[align]]
            if ((rf[0] == '{') || (rf[0] == '}'))
                throw error_format("Invalid fill char");
            fs.fill  = rf.pop_front();
            fs.align = rf.pop_front();
        }
        if (rf.is_empty())
            return;

        // [sign] | sign: [+- ]
        const auto& sc = format_spec_t::sign_chars;
        if (sc.find_first_of(rf[0]) != npos) {
            if (!supports_sign())
                throw error_format("Sign {:[+- ]} not supported for this type");
            fs.sign = rf.pop_front();
            if (rf.is_empty())
                return;
        }

        // [#] alternate form
        if (rf[0] == format_spec_t::alt_form_char) {
            if (!supports_alt_form())
                throw error_format("Alternate form {:#} not supported for this type");
            fs.alt_form = true;
            if (rf.remove_prefix(1).is_empty())
                return;
        }

        // [0] leading zeros
        if (rf[0] == format_spec_t::zero_pad_char) {
            if (!supports_leading_zeroes())
                throw error_format("Leading zeroes {:0} not supported for this type");
            fs.zero_pad = true;
            if (rf.remove_prefix(1).is_empty())
                return;
        }

        // [width] | positive-decimal or nested rf ({} or {n})
        if (ch_traits::is_digit_dec(rf[0])) {       // Explicit?
            auto&& [pos, ec] = from_chars(fs.width, rf);
            if (is_error(ec) || (fs.width < 1))
                throw error_format("Invalid width specification");
            if (rf.remove_prefix(pos).is_empty())
                return;
        }
        else if (rf[0] == '{') {                    // In nested rf?

            fs.width_in_arg = true;

            // Eat the '{'
            if (rf.remove_prefix(1).is_empty())
                throw error_format("Incomplete width nested replacement field");

            // Check for argument index.
            if (ch_traits::is_digit_dec(rf.front())) {
                auto&& [pos, ec] = from_chars(fs.width, rf);
                if (is_error(ec))
                    throw error_format("Invalid width value argument index");
                rf.remove_prefix(pos);

                // Mode has already been set, but this will throw if invalid
                pctx.set_argid_mode(parse_context_base::argid_mode_t::Manual);
            }
            else {
                // Consume the next argument index
                fs.width = pctx.get_next_auto_arg_idx();
            }

            // Make sure the argument is appropriately typed (int or unsigned)
            auto& args = pctx.get_format_args();
            if (fs.width >= args.get_arg_count())
                throw error_format("Invalid width argument index");
            auto varg = args.get_arg(fs.width).get_variant();
            if (!varg.holds_alternative<int>() && !varg.holds_alternative<unsigned>())
                throw error_format("Invalid type for width argument index; must be int or unsigned int");

            // Eat the '}'
            if (rf.is_empty() || rf.front() != '}')
                throw error_format("Incomplete width nested replacement field");
            if (rf.remove_prefix(1).is_empty())
                return;
        }

        // [precision]
        if (rf[0] == '.') {

            if (!supports_precision())
                throw error_format("Precision not supported for this type");

            fs.have_precision = true;

            rf.remove_prefix(1);

            if (ch_traits::is_digit_dec(rf[0])) {       // Explicit?
                auto&& [pos, ec] = from_chars(fs.precision, rf);
                if (is_error(ec) || (fs.precision < 0))
                    throw error_format("Invalid precision specification");
                if (rf.remove_prefix(pos).is_empty())
                    return;
            }
            else if (rf[0] == '{') {                    // In nested rf?

                fs.prec_in_arg = true;

                // Eat the '{'
                if (rf.remove_prefix(1).is_empty())
                    throw error_format("Incomplete precision nested replacement field");

                // Check for argument index
                if (ch_traits::is_digit_dec(rf.front())) {
                    auto&& [pos, ec] = from_chars(fs.precision, rf);
                    if (is_error(ec))
                        throw error_format("Invalid precision value argument index");
                    rf.remove_prefix(pos);

                    // Mode has already been set, but this will throw if invalid
                    pctx.set_argid_mode(parse_context_base::argid_mode_t::Manual);
                }
                else {
                    // Consume the next argument index
                    fs.precision = pctx.get_next_auto_arg_idx();
                }

                // Make sure the argument is appropriately typed (int or unsigned)
                auto& args = pctx.get_format_args();
                if (fs.precision >= args.get_arg_count())
                    throw error_format("Invalid precision argument index");
                const auto& varg = args.get_arg(fs.precision).get_variant();
                if (!varg.holds_alternative<int>() && !varg.holds_alternative<unsigned>())
                    throw error_format("Invalid type for precision argument index; must be int or unsigned int");

                // Eat the '}'
                if (rf.is_empty() || rf.front() != '}')
                    throw error_format("Incomplete precision nested replacement field");
                if (rf.remove_prefix(1).is_empty())
                    return;
            }
            else
                throw error_format("Invalid precision specification");
        }

        // [L] locale-specific formatting
        if (rf[0] == format_spec_t::use_locale_char) {
            fs.use_locale = true;
            if (rf.remove_prefix(1).is_empty())
                return;
        }

        // [type] | type: [aAbBcdeEfFgGopsxX] or subset thereof
        if (fs.type_chars.find_first_of(rf[0]) != npos) {
            fs.type = rf.pop_front();
            if (rf.is_empty())
                return;
        }

        // Anything remaining is a format error
        throw error_format("Unprocessed trailing characters in replacement field.");
    }

    // Pull width from a format argument
    static size_t get_width_from_arg(size_t arg_idx, format_context& fmt_ctx)
    {
        const auto& args = fmt_ctx.get_format_args();
        if (arg_idx >= args.get_arg_count())
            throw error_format("Invalid width argument index");
        auto varg = args.get_arg(arg_idx).get_variant();
        if (varg.holds_alternative<unsigned>())
            return varg.get<unsigned>();
        if (varg.holds_alternative<int>()) {
            int iwidth = varg.get<int>();
            if (iwidth < 0)
                throw error_format("Invalid width argument value; must be non-negative");
            return static_cast<size_t>(iwidth);
        }

        throw error_format("Invalid type for width argument index; must be int or unsigned int");
    }

    // Pull precision from a format argument
    static size_t get_precision_from_arg(size_t arg_idx, format_context& fmt_ctx)
    {
        const auto& args = fmt_ctx.get_format_args();
        if (arg_idx >= args.get_arg_count())
            throw error_format("Invalid precision argument index");
        auto varg = args.get_arg(arg_idx).get_variant();
        if (varg.holds_alternative<unsigned>())
            return varg.get<unsigned>();
        if (varg.holds_alternative<int>()) {
            int iprec = varg.get<int>();
            if (iprec < 0)
                throw error_format("Invalid precision argument value; must be non-negative");
            return static_cast<size_t>(iprec);
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

private:

    format_spec_t   _spec{};
};

_SYS_END_NS

#include "fmt_std_int.h"
#include "fmt_std_str.h"

#endif // ifndef sys_imp_fmt_formatter_included
