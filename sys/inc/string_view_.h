/**
 * @file    string_view_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   sys::string
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_string_view__included
#define sys_string_view__included

#include <_core_.h>
#include <utility_.h>
#include <memory_.h>
#include <iterator_.h>
#include <char_traits_.h>
#include "imp/string_helper.h"

_SYS_BEGIN_NS

class string_view
{
public:
    using char_t = char;
    using size_type = size_t;
    using traits_t = char_traits<char_t>;

    using pointer         = char_t*;
    using const_pointer   = const char_t*;
    using reference       = char_t&;
    using const_reference = const char_t&;

    using iterator_type = it_contig<const char_t>;

    /// Special value. The exact meaning depends on the context
    static constexpr size_type npos = size_type(-1);

    // -- Construction and assignment

    /// Default constructor
    constexpr string_view() noexcept = default;
    /// Copy constructor
    constexpr string_view(const string_view&) noexcept = default;
    /// Construct from a NULL terminated string
    constexpr string_view(const char_t* s)
        : _dat(s), _len(traits_t::length(s))
    {}
    /// Construct from any string
    constexpr string_view(const char_t* s, size_type count)
        : _dat(s), _len(count)
    {}
    /// Construct from an array
    template <size_t N>
    constexpr string_view(const char (&a)[N])
        : _dat(a), _len(N)
    {}

    /// string_view cannot be constructed from nullptr
    string_view(sys::nullptr_t) = delete;

    constexpr string_view& operator=(const string_view&) noexcept = default;

    // -- Element access

    /// Returns a pointer to the first character of a view
    constexpr const char_t* data() const noexcept
        { return _dat; }

    /// Accesses the specified character
    constexpr const_reference operator[](size_type pos) const noexcept
        { return data()[pos]; }

    /// Accesses the specified character with bounds checking
    constexpr const_reference at(size_type pos) const
        { check_pos(pos); return data()[pos]; }

    /// Accesses the first character
    constexpr const_reference front() const
        { return data()[0]; }

    /// Accesses the last character
    constexpr const_reference back() const
        { return data()[length() - 1]; }

    // -- Capacity

    /// Returns the number of characters
    constexpr size_type length() const noexcept
        { return _len; }
    /// Returns the number of characters
    constexpr size_type size() const noexcept
        { return _len; }

    /// Returns true if our string view is not empty
    constexpr explicit operator bool() const noexcept
        { return !is_empty(); }

    /// Checks whether the view is empty
    constexpr bool is_empty() const noexcept
        { return 0 == length(); }

    // -- Modifiers

    /// Moves the start of the view forward by n characters; returns *this
    constexpr string_view& remove_prefix(size_type n) noexcept
    {
        if (n > length()) [[unlikely]]
            n = length();

        _dat += n;
        _len -= n;

        return *this;
    }

    /// Moves the end of the view back by n characters; returns *this
    constexpr string_view& remove_suffix(size_type n) noexcept
    {
        if (n > length()) [[unlikely]]
            n = length();

        _len -= n;

        return *this;
    }

    /// Truncate view to specified length
    constexpr string_view& truncate(size_type len) noexcept
    {
        if (len < length())
            _len = len;

        return *this;
    }

    /// Moves the start of the view forward by one character; returns the character
    constexpr char_t pop_front()
    {
        char_t ch = front();
        remove_prefix(1);
        return ch;
    }

    /// Moves the start of the view forward by n characters; returns view of removed chars
    constexpr string_view pop_front(size_type n)
    {
        if (0 == n)
            return string_view{};

        string_view sv(substr_view(0, n));
        remove_prefix(n);
        return sv;
    }

    /// Trim leading and/or trailing whitespace from string_view
    constexpr string_view& trim(bool trim_left = true, bool trim_right = true,
        string_view ws = " \t\n\r\f\v")
    {
        if (trim_left) {
            auto p = find_first_not_of(ws);
            if (p == string_view::npos)
                p = length();
            remove_prefix(p);
        }

        if (trim_right) {
            auto p = find_last_not_of(ws);
            if (p == string_view::npos)
                remove_suffix(length());
            else
                remove_suffix(length() - p - 1);
        }

        return *this;
    }

    /// Swap the contents of two string_view objects
    constexpr void swap(string_view& sv) noexcept
    {
        sys::swap(_dat, sv._dat);
        sys::swap(_len, sv._len);
    }

    // -- Iterators

    constexpr iterator_type begin() const noexcept
        { return iterator_type(data()); }
    constexpr iterator_type end() const noexcept
        { return iterator_type(data() + length()); }

    constexpr iterator_type cbegin() const noexcept
        { return iterator_type(data()); }
    constexpr iterator_type cend() const noexcept
        { return iterator_type(data() + length()); }

    // -- Substring generation

    constexpr string_view substr_view(size_type pos = 0, size_type count = npos) const
    {
        check_pos(pos);

        if (pos + count > length()) // Clip count to end of string view
            count = npos;

        size_type count_use = (count == npos) ? length() - pos : count;

        return string_view(data() + pos, count_use);
    }

    // -- Search

    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(string_view str, size_type pos = 0) const noexcept
    {
        if (is_empty() || (pos >= length())) [[unlikely]]
            return npos;
        if (str.is_empty()) [[unlikely]]
            return pos;

        if (length() >= str.length()) {
            for (; pos < length() - str.length() + 1; ++pos) {
                if (0 == traits_t::compare(&at(pos), str.data(), str.length()))
                    return pos;
            }
        }

        return npos;
    }
    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(const char_t* s, size_type pos = 0) const noexcept
        { return find_first(string_view(s), pos); }
    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(const char_t* s, size_type pos, size_type count) const noexcept
        { return find_first(string_view(s, count), pos); }
    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(char_t ch, size_type pos = 0) const noexcept
    {
        if (is_empty() || (pos >= length())) [[unlikely]]
            return npos;

        for (auto p = data() + pos; p<data() + length(); ++p) {
            if (*p == ch)
                return p - data();
        }

        return npos;
    }

    /// Find the last occurrence of a substring
    constexpr size_type find_last(string_view s, size_type pos = npos) const noexcept
    {
        // If we are empty then we can't find anything
        if (is_empty()) [[unlikely]]
            return npos;

        // Resolve pos to an absolute index
        if ((pos == npos) || (pos > length() - 1))
            pos = length() - 1;

        // Anything matches the empty search string, so return first search pos
        if (s.is_empty()) [[unlikely]]
             return pos;

        // Depending on the length of the search string, we may be able to
        // jump over positions that can't possibly match. For example, if
        // searching for "banana" then it doesn't make any sense to consider
        // the last 5 positions of the string since we need 6 chars to
        // match. If we happen to jump before the start of the string (i.e.,
        // the search string is longer than the source string) then the loop
        // below won't run and we return npos.
        if (pos > length() - s.length())
            pos = length() - s.length();
        auto p = data() + pos;

        for (; p >= data(); --p) {
            if (0 == traits_t::compare(p, s.data(), s.length()))
                return p - data();
        }

        return npos;
    }
    /// Find the last occurrence of a substring
    constexpr size_type find_last(const char_t* s, size_type pos = npos) const noexcept
        { return find_last(string_view(s), pos); }
    /// Find the last occurrence of a substring
    constexpr size_type find_last(const char_t* s, size_type pos, size_type count) const noexcept
        { return find_last(string_view(s, count), pos); }
    /// Find the last occurrence of a substring
    constexpr size_type find_last(char_t ch, size_type pos = npos) const noexcept
    {
        // If we are empty then we can't find anything
        if (is_empty()) [[unlikely]]
            return npos;

        auto p = (pos < length()) ? (data() + pos) : (data() + length() - 1);
        for (; p >= data() && (*p != ch); --p);
        return (p < data()) ? npos : size_type(p - data());
    }

    /// Finds the first char equal to any of the chars in the given sequence
    constexpr size_type find_first_of(string_view sv, size_type pos = 0) const noexcept
    {
        for (; pos < length(); ++pos) {
            if (npos != sv.find_first(data()[pos]))
                return pos;
        }

        return npos;
    }
    /// Finds the first occurence of char in string
    constexpr size_type find_first_of(char_t ch, size_type pos = 0) const noexcept
        { return find_first_of(&ch, 1, pos); }
    /// Finds the first char equal to any of the chars in the given sequence
    constexpr size_type find_first_of(const char_t* s, size_type count, size_type pos) const noexcept
        { return find_first_of(string_view(s, count), pos); }
    /// Finds the first char equal to any of the chars in the given sequence
    constexpr size_type find_first_of(const char_t* s, size_type pos = 0) const noexcept
        { return find_first_of(string_view(s), pos); }

    /// Finds first char equal to none of the chars in the given sequence
    constexpr size_type find_first_not_of(string_view sv, size_type pos = 0) const noexcept
    {
        // If we are empty then we can't find anything
        if (is_empty()) [[unlikely]]
            return npos;

        for (; pos < length(); ++pos) {
            if (npos == sv.find_first(data()[pos]))
                return pos;
        }

        return npos;
    }
    constexpr size_type find_first_not_of(char_t ch, size_type pos = 0) const noexcept
        { return find_first_not_of(string_view(&ch, 1), pos); }
    constexpr size_type find_first_not_of(const char_t* s, size_type count, size_type pos) const noexcept
        { return find_first_not_of(string_view(s, count), pos); }
    constexpr size_type find_first_not_of(const char_t* s, size_type pos = 0) const noexcept
        { return find_first_not_of(string_view(s), pos); }

    /// Finds the last char equal to one of chars in the given sequence
    constexpr size_type find_last_of(string_view sv, size_type pos = npos) const noexcept
    {
        // If we are empty then we can't find anything
        if (is_empty()) [[unlikely]]
            return npos;

        auto p = (pos < length()) ? (data() + pos) : (data() + length() - 1);
        for (; p > data(); --p) {
            if (sv.find_first(*p) != npos)
                return size_type(p - data());
        }
        return npos;
    }
    constexpr size_type find_last_of(char_t ch, size_type pos = npos) const noexcept
        { return find_last_of(string_view(&ch, 1), pos); }
    constexpr size_type find_last_of(const char_t* s, size_type count, size_type pos) const noexcept
        { return find_last_of(string_view(s, count), pos); }
    constexpr size_type find_last_of(const char_t* s, size_type pos = npos) const noexcept
        { return find_last_of(string_view(s), pos); }

    constexpr size_type find_last_not_of(string_view sv, size_type pos = npos) const noexcept
    {
        // If we are empty then we can't find anything
        if (is_empty()) [[unlikely]]
            return npos;

        auto p = (pos < length()) ? (data() + pos) : (data() + length() - 1);
        for (; p > data(); --p) {
            if (sv.find_first(*p) == npos)
                return size_type(p - data());
        }
        return npos;
    }
    constexpr size_type find_last_not_of(char_t ch, size_type pos = npos) const noexcept
        { return find_last_not_of(string_view(&ch, 1), pos); }
    constexpr size_type find_last_not_of(const char_t* s, size_type count, size_type pos) const noexcept
        { return find_last_not_of(string_view(s, count), pos); }
    constexpr size_type find_last_not_of(const char_t* s, size_type pos = npos) const noexcept
        { return find_last_not_of(string_view(s), pos); }

    /// Checks if the string view begins with the given prefix
    constexpr bool starts_with(string_view sv) const noexcept
        { return substr_view(0, sv.length()) == sv; }
    /// Checks if the string view begins with the given prefix
    constexpr bool starts_with(char_t ch) const noexcept
        { return !is_empty() && (front() == ch); }
    /// Checks if the string view begins with the given prefix
    constexpr bool starts_with(const char_t* s)
        { return starts_with(string_view(s)); }

    /// Checks if the string view ends with the given suffix
    constexpr bool ends_with(string_view sv) const noexcept
    {
        return (length() >= sv.length()) &&
            (substr_view(length() - sv.length()) == sv);
    }
    /// Checks if the string view ends with the given suffix
    constexpr bool ends_with(char_t ch) const noexcept
        { return !is_empty() && (back() == ch); }
    /// Checks if the string view ends with the given suffix
    constexpr bool ends_with(const char_t* s) const noexcept
        { return ends_with(string_view(s)); }

    /// Checks if the string view contains the given substring
    constexpr bool contains(string_view sv) const noexcept
        { return find_first(sv) != npos; }
    /// Checks if the string view contains the given character
    constexpr bool contains(char_t ch) const noexcept
        { return find_first(ch) != npos; }
    /// Checks if the string view contains the given substring
    constexpr bool contains(const char_t* s) const noexcept
        { return find_first(s) != npos; }

    // -- Comparison

    constexpr int compare(const string_view sv) const noexcept
    {
        // First check common string content
        auto min_len = min(length(), sv.length());
        if (auto cmp = traits_t::compare(data(), sv.data(), min_len))
            return cmp;

        // Then fall back on string length: the longer string will have at
        // least one more character makes it greater.
        const auto lcmp = length() <=> sv.length();
        return is_eq(lcmp) ? 0 : is_lt(lcmp) ? -1 : 1;
    }

    /// Implements string_view comparison
    constexpr auto operator<=>(const string_view& rhs) const noexcept
        { return compare(rhs) <=> 0; }
    constexpr bool operator==(const string_view& rhs) const noexcept
        { return 0 == compare(rhs); }

protected:

    constexpr void check_pos(size_type pos) const
    {
        if (pos >= length())
            throw_error_bounds(pos, length());
    }

private:

    const_pointer   _dat{ nullptr };    ///< String buffer
    size_type       _len{ 0 };          ///< String length
};

// Helper for template methods that take something that converts to string_view
template <class T, class Char_t = char>
struct is_string_view_like : bool_constant<
     is_convertible_v<const T&, string_view> &&
    !is_convertible_v<const T&, const Char_t*>
> {};

template <class T, class Char_t = char>
inline constexpr bool is_string_view_like_v = is_string_view_like<T, Char_t>::value;

/// Specifies a type that can be converted to string_view, but not a raw pointer
template <class T, class Char_t = char>
concept string_view_like = sys::is_string_view_like_v<T, Char_t>;

/// Specifies a type that can be converted to string_view
template <class T, class Char_t = char>
concept string_view_convertible = sys::is_convertible_v<T, string_view>;


_SYS_END_NS

#endif // ifndef sys_string_view__included
