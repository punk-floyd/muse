/**
 * @file    char_traits_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Basic string and character operations
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef char_traits__included
#define char_traits__included

#include <_core_.h>
#include <memory_.h>

_SYS_BEGIN_NS

template <class T>
struct char_traits {

    using char_t = T;

    /// Terminator for null-terminated strings
    static constexpr char_t null_term = char_t(0);

    /// Returns the length of the character sequence pointed to by s
    static constexpr inline size_t length(const char_t* s)
    {
        size_t len = 0;
        while (*s++ != null_term) ++len;
        return len;
    }

    /// Copy non-overlapping chars; returns dst
    static constexpr inline char_t* copy(char_t* dst, const char_t* src, size_t count)
    {
        return mem::copy_bytes(dst, src, count);
    }

    /// Copy possibly-overlapping chars; returns dst
    static constexpr inline char_t* move(char_t* dst, const char_t* src, size_t count)
    {
        return mem::move_bytes(dst, src, count);
    }

    static constexpr inline char_t* fill(char_t* dst, char_t value, size_t count)
    {
        return mem::fill(dst, value, count);
    }

    /// Compares the first count characters of the character strings s1 and s2
    static constexpr inline int compare(const char_t* s1, const char_t* s2, size_t count)
    {
        while (count && (*s1 != null_term) && (*s2 != null_term) && (*s1 == *s2))
            s1++, s2++, count--;
        return (!count || (*s1 == *s2)) ? 0 : (*s1 < *s2) ? -1 : 1;
    }

    /// Compares characters of the character strings s1 and s2 up to null terminator
    static constexpr inline int compare(const char_t* s1, const char_t* s2)
    {
        while ((*s1 != null_term) && (*s2 != null_term) && (*s1 == *s2))
            s1++, s2++;
        return (*s1 == *s2) ? 0 : (*s1 < *s2) ? -1 : 1;
    }

    /// Return lower case version of c
    static constexpr inline char_t to_lower(char_t c)
        { return ((c >= 'A') && (c <= 'Z')) ? (c + 32) : c; }
    /// Return upper case version of c
    static constexpr inline char_t to_upper(char_t c)
        { return ((c >= 'a') && (c <= 'z')) ? (c - 32) : c; }

    // TODO : This is a quick-and-dirty implementation. It will be faster to
    // implement this stuff via LUT. Or, maybe move this stuff into an
    // encoding (utf-8) traits?

    /// Returns true if given char is a control character
    static constexpr inline bool is_control(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return (uc < 32) || (uc == 127);
    }
    /// Returns true if given char is printable (not a control character)
    static constexpr inline bool is_printable(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return (uc >= 32) && (uc < 127);
    }
    /// Returns true if given char is whitespace, tab, or space
    static constexpr inline bool is_space(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return (uc == 32) || ((uc >= 9) && (uc <= 13));
    }
    /// Returns true if given char is tab or space
    static constexpr inline bool is_blank(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return (uc == 32) || (uc == 9);
    }
    /// Returns true if given char has graphical representation
    static constexpr inline bool is_graph(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return (uc >= 33) && (uc <= 126);
    }
    /// Returns true if given char is punctuation
    static constexpr inline bool is_punctuation(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return
            ((uc >= 48) && (uc <=  57)) ||
            ((uc >= 65) && (uc <=  90)) ||
            ((uc >= 97) && (uc <= 122));
    }
    /// Returns true if given char is alphanumeric
    static constexpr inline bool is_alnum(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return
            ((uc >= 'a') && (uc <= 'z')) ||
            ((uc >= 'A') && (uc <= 'Z')) ||
            ((uc >= '0') && (uc <= '9'));
    }
    /// Returns true if given char is alphabetic
    static constexpr inline bool is_alpha(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return
            ((uc >= 'a') && (uc <= 'z')) ||
            ((uc >= 'A') && (uc <= 'Z'));
    }
    /// Returns true if given char is upper case
    static constexpr inline bool is_upper(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return ((uc >= 'A') && (uc <= 'Z'));
    }
    /// Returns true if given char is lower case
    static constexpr inline bool is_lower(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return ((uc >= 'a') && (uc <= 'z'));
    }
    /// Returns true if given char is a decimal digit
    static constexpr inline bool is_digit_dec(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return ((uc >= '0') && (uc <= '9'));
    }
    /// Returns true if given char is a hexadecimal digit
    static constexpr inline bool is_digit_hex(char_t c)
    {
        auto uc = static_cast<unsigned char>(c);
        return
            ((uc >= '0') && (uc <= '9')) ||
            ((uc >= 'A') && (uc <= 'F')) ||
            ((uc >= 'a') && (uc <= 'f'));
    }
};

_SYS_END_NS

#endif // ifndef char_traits__included
