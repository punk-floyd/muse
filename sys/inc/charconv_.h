/**
 * @file    charconv_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Character conversion routines
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_charconv__included
#define sys_charconv__included

#include <_core_.h>
#include <concepts_.h>
#include <limits_.h>
#include <error_.h>

_SYS_BEGIN_NS

/// Return value for from_chars
struct from_chars_result {

    using size_type = string_view::size_type;

    constexpr from_chars_result() = default;
    constexpr from_chars_result(size_type pos, error_code code = error_code{})
        : pos_stop(pos), ec(code)
    {}

    size_type       pos_stop{string_view::npos};
    error_code      ec{};
};

template <integral T>
constexpr from_chars_result from_chars(T& value, string_view s, unsigned radix = 10) noexcept(true)
{
    using ch_traits  = string_view::traits_t;
    using int_traits = numeric_limits<T>;
    using result     = from_chars_result;
    using value_type = T;

    // Check radix. This implementation will handle up to base 36
    constexpr string_view radix_chars{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    if ((radix == 1) || radix > radix_chars.length())
        return result(0, error_code::bad_parameter);

    // Track original view so we can return appropriate offset. We use s as
    // our working copy and adjust the view as we consume characters.
    string_view s_og{s};

    // Eat leading whitespace
    if (s.trim(true, false).is_empty())
        return result(0, error_code::bad_parameter);

    // Used for conversion of negative signed number. Should be optimized
    // out for unsigned conversions.
    value_type fact{1};

    // Eat optional sign (+/-)
    int sign = (s[0] == '-') ? -1 : (s[0] == '+') ? 1 : 0;
    if (sign < 0) {
        // Negative not allowed for unsigned numbers
        if constexpr (int_traits::is_unsigned)
            return result(0, error_code::out_of_range);
        else
            fact = value_type{-1};
    }
    if (sign)
        s.remove_prefix(1);

    // Handle auto-radix: 0xn=hex, 0n=oct, 0b=binary, else dec
    if (radix == 0) {
        if      ((s.length() > 2) && (s[0] == '0') && (ch_traits::to_lower(s[1] == 'x')))
            radix = 16; // Hexadecimal
        else if ((s.length() > 1) && (s[0] == '0')) // checking for 0n[...]
            radix =  8; // Octal
        else if ((s.length() > 2) && (s[0] == '0') && (ch_traits::to_lower(s[1] == 'b')))
            radix =  2; // Binary
        else
            radix = 10; // Decimal
    }

    // Eat optional hex/oct/binary prefixes
    if ((radix == 16) && (s.starts_with("0x") || s.starts_with("0X")))
        s.remove_prefix(2);
    else if ((radix == 8) && s.starts_with("0") && (s.length() > 1))
        s.remove_prefix(1);
    else if ((radix == 2) && (s.starts_with("0b") || s.starts_with("0B")))
        s.remove_prefix(2);

    // Eat the digits
    value_type digit = 0, radix_mult = static_cast<value_type>(radix);
    value_type val_work = value_type{0};    // Working value
    bool matched = false;
    while (!s.is_empty()) {

        // Is this a valid digit?
        char ch = ch_traits::to_upper(s[0]);
        if (ch_traits::is_digit_dec(ch))
            digit = static_cast<value_type>(ch - '0');
        else if (ch_traits::is_alpha(ch))
            digit = static_cast<value_type>(ch - 'A') + value_type{10};
        else
            digit = radix;
        if (digit >= radix)
            break;  // Nope, we're done

        // val_work *= radix, but with overflow detection
        if (multiply_overflow(val_work, radix_mult, val_work))
            return result(&s[0] - &s_og[0], error_code::out_of_range);
        // val_work += digit, but with overflow detection
        if (add_overflow(val_work, digit * fact, val_work))
            return result(&s[0] - &s_og[0], error_code::out_of_range);

        s.remove_prefix(1);
        matched = true;
    }
    if (!matched)
        return result(0, error_code::bad_parameter);

    value = val_work;
    return result(s.is_empty() ? s_og.length() :  &s[0] - &s_og[0]);
}

/// Result from to_chars
struct to_chars_result
{
    constexpr to_chars_result() noexcept = default;
    constexpr to_chars_result(char* ptr, error_code ec = error_code::no_error) noexcept
        : ptr(ptr), ec(ec)
    {}

    char*           ptr{nullptr};
    error_code      ec{};
};

template <integral T>
constexpr to_chars_result to_chars(char* begin, char* end, const T& val, unsigned base = 10)
{
    // Check radix. This implementation will handle up to base 36
    constexpr string_view radix_chars{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    if ((base < 2) || (base > radix_chars.length()))
        return to_chars_result(begin, error_code::bad_parameter);

    T val_use = val;
    auto at = begin;
    bool first = true; // Ensures we handle 0 for unsigned types

    // Handle negative and zero
    if constexpr (is_signed_v<T>) {
        if (val_use <= T{0}) {
            if (val_use < T{0}) {
                if (begin == end)
                    return to_chars_result(at, error_code::value_too_large);
                *at++ = '-';
                begin++;    // Advance so we don't include in swap below.
            }

            // Do first round here. This will decimate the number by one digit
            // at which point it will be safe to negate and run through the
            // main loop below.
            if (at == end)
                return to_chars_result(at, error_code::value_too_large);
            *at++ = radix_chars[(val_use % T{base}) * (val_use < T{0} ? -1:1)];
            val_use /= -T{base};
            first = false;
        }
    }

    // Quickly calculate the digits; they will be in reverse order
    while (val_use || first) {
        if (at == end)
            return to_chars_result(at, error_code::value_too_large);
        *at++ = radix_chars[val_use % base];
        val_use /= base;
        first = false;
    }

    // Reverse the digits
    auto loops = (at - begin) >> 1;
    for (ptrdiff_t i=0; i<loops; ++i)
        sys::swap(begin[i], at[-(i+1)]);

    return to_chars_result{at};
}

_SYS_END_NS

#endif // ifndef sys_charconv__included
