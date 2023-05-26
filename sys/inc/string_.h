/**
 * @file    string_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   sys::string - A general purpose char-based null-terminated string
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_String__included
#define sys_String__included

#include <_core_.h>
#include <string_view_.h>
#include <iterator_.h>
#include <utility_.h>
#include <error_.h>
#include "imp/string_buf.h"

_SYS_BEGIN_NS

class string
{
public:

    using char_t         = char;
    using size_type      = size_t;
    using traits_t       = char_traits<char_t>;

    using value_type     = char_t;
    using iterator       = it_contig<char_t>;
    using const_iterator = it_contig<const char_t>;

    using reference       = char_t&;
    using const_reference = const char_t&;

    /// Special value. The exact meaning depends on the context
    static constexpr size_type npos = size_type(-1);

    // -- Construction

private:

    /// Construct with default initialized data
    constexpr explicit string(size_type count)
    {
        ensure_buf(count);
    }

public:

    /// Default constructor; constructs an empty string ("")
    constexpr string() noexcept = default;

    /// Construct from your grand pappy's NULL-terminated string
    constexpr explicit string(const char_t* s)
    {
        auto len = traits_t::length(s);
        traits_t::copy(ensure_buf(len), s, len);
    }

    /// Construct from a piece of a raw string
    constexpr string(const char_t* s, size_type count)
    {
        traits_t::copy(ensure_buf(count), s, count);
    }

    /// Constructs the string with count copies of character ch
    constexpr string(char_t ch, size_type count)
    {
        traits_t::fill(ensure_buf(count), ch, count);
    }

    /// Construct from something that can be converted to string_view
    template <string_view_like T>
    constexpr explicit string(const T& svl)
    {
        traits_t::copy(ensure_buf(svl.length()), svl.data(), svl.length());
    }

    /// Construct from a piece of another string
    constexpr string(const string& other, size_type pos, size_type count = npos)
    {
        string_view svl(other.substr_view(pos, count));
        traits_t::copy(ensure_buf(svl.length()), svl.data(), svl.length());
    }

    /// Construct from a piece of a string-view-like thing
    template <string_view_like T>
    constexpr string(const T& other, size_type pos, size_type count = npos)
    {
        string_view svl(other.substr_view(pos, count));
        traits_t::copy(ensure_buf(svl.length()), svl.data(), svl.length());
    }

    /// Copy constructor
    constexpr string(const string& other)
        : _sbuf(other._sbuf)
    {
    }

    /// Move constructor
    constexpr string(string&& other) noexcept
        : _sbuf(sys::move(other._sbuf))
    {
    }

    /// string cannot be constructed from nullptr
    string(sys::nullptr_t) = delete;

    // -- Operations

    constexpr string& assign(char_t ch, size_type count)
    {
        traits_t::fill(ensure_buf(count), ch, count);
        return *this;
    }

    constexpr string& assign(const string& str)
    {
        traits_t::copy(ensure_buf(str.length()), str.data(), str.length());
        return *this;
    }

    constexpr string& assign(const string& str, size_type pos, size_type count = npos)
    {
        string_view sv(str.substr_view(pos, count));
        traits_t::copy(ensure_buf(sv.length()), sv.data(), sv.length());

        return *this;
    }

    constexpr string& assign(string&& other) noexcept
    {
        _sbuf = sys::move(other._sbuf);
        return *this;
    }

    constexpr string& assign(const char_t* s, size_type count) noexcept
    {
        traits_t::copy(ensure_buf(count), s, count);
        return *this;
    }

    constexpr string& assign(const char_t* s) noexcept
    {
        return assign(s, traits_t::length(s));
    }

    template <string_view_like T>
    constexpr string& assign(const T& svl)
    {
        traits_t::copy(ensure_buf(svl.length()), svl.data(), svl.length());
        return *this;
    }

    template <string_view_like T>
    constexpr string& assign(const T& svl, size_type pos, size_type count = npos)
    {
        string_view sv(svl.substr_view(pos, count));
        traits_t::copy(ensure_buf(sv.length()), sv.data(), sv.length());
        return *this;
    }

    /// Resize the string to contain count characters
    constexpr void resize(size_type count)
    {
        ensure_buf(count, true);
    }

    /// Resize the string to contain count characters, appending with ch
    constexpr void resize(size_type count, char_t ch)
    {
        auto og_len = length();
        ensure_buf(count, true);
        if (count > og_len)
            traits_t::fill(data() + og_len, ch, count - og_len);
    }

    // - Clearing and deleting pieces of string

    /// Clear string content (including reserve)
    constexpr void clear() noexcept
    {
        _sbuf.clear();
    }

    /// Remove characters from string
    constexpr string& erase(size_type pos = 0, size_type count = npos)
    {
        check_pos(pos);

        // Clip count to end of string if necessary
        if ((count == npos) || (pos + count > length()))
            count = length() - pos;

        // Move back any chars after the deleted region. The region being
        // moved back and the deleted region cannot overlap so memcpy is
        // safe here.
        if (size_type char_count = length() - (pos + count))
            traits_t::copy(data() + pos, data() + pos + count, char_count);

        // Note that we're just adjusting length; capacity remains the same
        _sbuf.internal_set_length(length() - count);
        return *this;
    }

    /// Remove a character from string
    constexpr string& erase(const_iterator it)
    {
        check_it(it);

        return erase(it - cbegin(), 1);
    }

    /// Remove characters from string [first, last)
    constexpr string& erase(const_iterator first, const_iterator last)
    {
        check_it(first);
        check_it(last);

        return erase(first - cbegin(), last - first);
    }

    /// Trim leading and/or trailing whitespace from string
    constexpr string& trim(bool trim_left = true, bool trim_right = true,
        string_view ws = " \t\n\r\f\v")
    {
        if (trim_left)
            erase(0, find_first_not_of(ws));
        if (trim_right)
            erase(find_last_not_of(ws) + 1);

        return *this;
    }

    // - Substring generation

    /// Generate a view of substring of this object
    constexpr string_view substr_view(size_type pos = 0, size_type count = npos) const
    {
        check_pos(pos);

        if (pos + count > length()) // Clip count to end of string
            count = npos;

        size_type count_use = (count == npos) ? length() - pos : count;

        return string_view(data() + pos, count_use);
    }

    /// Generate a new string containing a substring of this object
    constexpr string substr(size_type pos = 0, size_type count = npos) const
    {
        return string(substr_view(pos, count));
    }

protected:

    constexpr string& impl_insert_chars(size_type pos, size_type count, char_t ch)
    {
        // Convert npos to its actual offset
        if (pos == npos)
            pos = length();
        bool appending = pos == length();

        check_length(count);
        check_pos(pos);

        if (count == 0) [[unlikely]]
            return *this;

        // Does insertion fit into current capacity?
        if (length() + count <= capacity()) {
            if (!appending)
                traits_t::move(data() + pos + count, data() + pos, length() - pos);
            traits_t::fill(data() + pos,  ch, count);
            _sbuf.internal_set_length(length() + count);
            return *this;
        }

        // Generate a new string and become it
        string st(length() + count);
        traits_t::copy(st.data(), data(), pos);
        traits_t::fill(st.data() + pos,  ch, count);
        if (!appending)
            traits_t::copy(st.data() + pos + count, data() + pos, length() - pos);
        return *this = sys::move(st);
    }

    constexpr string& impl_insert(size_type pos, const char_t* s, size_type count)
    {
        // Convert npos to its actual offset
        if (pos == npos)
            pos = length();
        bool appending = pos == length();

        check_length(count);
        check_pos(pos);

        if (count == 0) [[unlikely]]
            return *this;

        // Does insertion fit into current capacity?
        if (length() + count <= capacity()) {
            if (!appending)
                traits_t::move(data() + pos + count, data() + pos, length() - pos);
            traits_t::copy(data() + pos, s, count);
            _sbuf.internal_set_length(length() + count);
            return *this;
        }

        // Generate a new string and become it
        string st(length() + count);
        traits_t::copy(st.data(), data(), pos);
        traits_t::copy(st.data() + pos, s, count);
        if (!appending)
            traits_t::copy(st.data() + pos + count, data() + pos, length() - pos);
        return *this = sys::move(st);
    }

public:

    // -- Inserting and appending

    constexpr void push_back(char_t ch)
    {
        check_length(1);

        // MDTODO : This is terrible allocation strategy.
        //  We should double buffer size here if we need to resize
        //  Similarly for others like append

        ensure_buf(1 + length());
        back() = ch;
    }

    constexpr char_t pop_back()
    {
        char_t ret = back();

        if (auto len = length())
            _sbuf.internal_set_length(len - 1);

        return ret;
    }

    /// Insert a number of chars at the given position
    constexpr string& insert(size_type pos, char_t ch, size_type count = 1)
    {
        return impl_insert_chars(pos, count, ch);
    }
    /// Insert a raw NULL-terminated string at the given position
    constexpr string& insert(size_type pos, const char_t* s)
    {
        return impl_insert(pos, s, traits_t::length(s));
    }
    /// Insert a raw string at the given position
    constexpr string& insert(size_type pos, const char_t* s, size_type count)
    {
        return impl_insert(pos, s, count);
    }
    /// Insert a string at the given position
    constexpr string& insert(size_type pos, const string& str)
    {
        return impl_insert(pos, str.data(), str.length());
    }
    /// Insert a substring of another string at the given position
    constexpr string& insert(size_type pos, const string& str,
        size_type str_pos, size_type str_count = npos)
    {
        string_view sub(str.substr_view(str_pos, str_count));
        return impl_insert(pos, sub.data(), sub.length());
    }
    /// Insert a number of chars at the given position
    constexpr iterator insert(const_iterator pos, char_t ch, size_type count = 1)
    {
        check_it(pos);

        size_type index = pos - cbegin();
        impl_insert_chars(index, count, ch);

        return begin() + index;
    }
    /// Insert characters from range at the given position
    // template <class InputIt>
    // constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
    // {
    //     // MOOMOO: TODO: need input iterator definition
    //      // Don't forget the corresponding append down below, too.
    // }
    template <string_view_like T>
    constexpr string& insert(size_type pos, const T& svl)
    {
        return impl_insert(pos, svl.data(), svl.length());
    }
    /// Insert a substring of a view at the given position
    template <string_view_like T>
    constexpr string& insert(size_type pos, const T& svl,
        size_type svl_pos, size_type svl_count = npos)
    {
        string_view sub(svl.substr_view(svl_pos, svl_count));
        return impl_insert(pos, sub.data(), sub.length());
    }

    constexpr string& append(char_t ch, size_type count = 1)
    {
        return impl_insert_chars(npos, count, ch);
    }
    constexpr string& append(const string& s)
    {
        return impl_insert(npos, s.data(), s.length());
    }
    constexpr string& append(const string& s, size_type pos,
        size_type count = npos)
    {
        string_view sv(s.substr_view(pos, count));
        return impl_insert(npos, sv.data(), sv.length());
    }
    constexpr string& append(const char_t* s, size_type count)
    {
        return impl_insert(npos, s, count);
    }
    constexpr string& append(const char_t* s)
    {
        return impl_insert(npos, s, traits_t::length(s));
    }
    /// Insert characters from range at the given position
    // template <class InputIt>
    // constexpr string& append(InputIt first, InputIt last)
    // {
    //     return impl_insert(npos, first, last);
    // }
    template <string_view_like T>
    constexpr string& append(const T& svl)
    {
        return impl_insert(npos, svl.data(), svl.length());
    }
    template <string_view_like T>
    constexpr string& append(const T& svl,
        size_type svl_pos, size_type svl_count = npos)
    {
        string_view sub(svl.substr_view(svl_pos, svl_count));
        return impl_insert(npos, sub.data(), sub.length());
    }

    // -- Element access

    /// Accesses the specified character with bounds checking
    constexpr reference at(size_type pos)
        { check_pos_ref(pos); return data()[pos]; }

    /// Accesses the specified character with bounds checking
    constexpr const_reference at(size_type pos) const
        { check_pos_ref(pos); return data()[pos]; }

    /// Accesses the first character
    constexpr reference front()
        { return data()[0]; }

    /// Accesses the first character
    constexpr const_reference front() const
        { return data()[0]; }

    /// Accesses the last character
    constexpr reference back()
        { return data()[length() - 1]; }

    /// Accesses the last character
    constexpr const_reference back() const
        { return data()[length() - 1]; }

    /// Access underlying string as a null terminated array
    constexpr const char_t* c_str() const
        { return data(); }

    // -- Comparison

    constexpr auto operator<=>(const string& rhs) const noexcept
        { return static_cast<string_view>(*this) <=> static_cast<string_view>(rhs); }
    constexpr bool operator==(const string& rhs) const noexcept
        { return static_cast<string_view>(*this) ==  static_cast<string_view>(rhs); }

    /// (1) Compare this string to another string
    constexpr int compare(const string& str) const noexcept
        { return substr_view().compare(str); }
    /// (2) Compare substring of this to another string
    constexpr int compare(size_type pos1, size_type count1, const string& str) const
        { return substr_view(pos1, count1).compare(str); }
    /// (3) Compare substring of this to a substring of another string
    constexpr int compare(size_type pos1, size_type count1,
        const string& str, size_type pos2, size_type count2) const
        { return substr_view(pos1, count1).compare(str.substr_view(pos2, count2)); }
    /// (4) Compare this string to a NULL-terminated string
    constexpr int compare(const char_t* str) const noexcept
        { return substr_view().compare(str); }
    /// (5) Compare substring of this to a NULL-terminated string
    constexpr int compare(size_type pos1, size_type count1, const char_t* str) const
        { return substr_view(pos1, count1).compare(str); }
    /// (6) Compare this string to something that can be converted to a string_view
    template <string_view_like T>
    constexpr int compare(const T& sv) noexcept
        { return substr_view().compare(sv); }
    /// (7) Compare a substring of this to something string_view like
    template <string_view_like T>
    constexpr int compare(size_type pos1, size_type count1, const T& sv)
        { return substr_view(pos1, count1).compare(sv); }
    /// (8) Compare a substring of this to a substring of something string_view like
    template <string_view_like T>
    constexpr int compare(size_type pos1, size_type count1,
        const T& sv, size_type pos2, size_type count2)
        { return substr_view(pos1, count1).compare(sv.substr_view(pos2, count2)); }

    // -- Search

    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(const string& str, size_type pos = 0) const noexcept
    {
        const auto sv_this = static_cast<const string_view>(*this);
        return sv_this.find_first(static_cast<string_view>(str), pos);
    }
    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(const char_t* str, size_type pos = 0)  const noexcept
    {
        const auto sv_this = static_cast<const string_view>(*this);
        return sv_this.find_first(str, pos);
    }
    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(const char_t* str, size_type pos, size_type count)  const noexcept
    {
        const auto sv_this = static_cast<const string_view>(*this);
        return sv_this.find_first(str, pos, count);
    }
    /// Finds the first substring equal to the given character sequence
    template <string_view_like T>
    constexpr size_type find_first(const T& str, size_type pos = 0)
    {
        const auto sv_this = static_cast<const string_view>(*this);
        return sv_this.find_first(str, pos);
    }
    /// Finds the first substring equal to the given character sequence
    template <string_view_like T>
    constexpr size_type find_first(const T& str, size_type pos, size_type count)
    {
        const auto sv_this = static_cast<const string_view>(*this);
        return sv_this.find_first(str, pos, count);
    }
    /// Finds the first substring equal to the given character sequence
    constexpr size_type find_first(char_t ch, size_type pos = 0) const noexcept
    {
        const auto sv_this = static_cast<const string_view>(*this);
        return sv_this.find_first(ch, pos);
    }

    /// Finds the first char equal to any of the chars in the given sequence
    constexpr size_type find_first_of(const string& s, size_type pos = 0) const noexcept
        { return substr_view().find_first_of(s, pos); }
    /// Finds the first char equal to any of the chars in the given sequence
    constexpr size_type find_first_of(const char_t* s, size_type count, size_type pos = 0) const noexcept
        { return substr_view().find_first_of(s, count, pos); }
    /// Finds the first char equal to any of the chars in the given sequence
    constexpr size_type find_first_of(const char_t* s, size_type pos = 0) const noexcept
        { return substr_view().find_first_of(string_view(s), pos); }
    /// Finds the first char equal to any of the chars in the given sequence
    constexpr size_type find_first_of(char_t ch, size_type pos = 0) const noexcept
        { return substr_view().find_first_of(ch, pos); }
    /// Finds the first char equal to any of the chars in the given sequence
    template <string_view_like T>
    constexpr size_type find_first_of(const T& svl, size_type pos = 0) const noexcept
        { return substr_view().find_first_of(static_cast<string_view>(svl), pos); }

    /// Finds the first char not equal to any of the chars in the given sequence
    constexpr size_type find_first_not_of(const string& s, size_type pos = 0) const noexcept
        { return substr_view().find_first_not_of(s, pos); }
    /// Finds the first char not equal to any of the chars in the given sequence
    constexpr size_type find_first_not_of(const char_t* s, size_type count, size_type pos) const noexcept
        { return substr_view().find_first_not_of(s, count, pos); }
    /// Finds the first char not equal to any of the chars in the given sequence
    constexpr size_type find_first_not_of(const char_t* s, size_type pos = 0) const noexcept
        { return substr_view().find_first_not_of(s, pos); }
    // Finds the first char not equal to any of the chars in the given sequence
    constexpr size_type find_first_not_of(char_t ch, size_type pos = 0) const noexcept
        { return substr_view().find_first_not_of(ch, pos); }
    /// Finds the first char equal to any of the chars in the given sequence
    template <string_view_like T>
    constexpr size_type find_first_not_of(const T& svl, size_type pos = 0) const noexcept
        { return substr_view().find_first_not_of(static_cast<string_view>(svl), pos); }

    /// Find the last occurrence of a substring
    constexpr size_type find_last(const char_t* s, size_type pos = npos) const noexcept
        { return substr_view().find_last(s, pos); }
    /// Find the last occurrence of a substring
    constexpr size_type find_last(const char_t* s, size_type pos, size_type count) const noexcept
        { return substr_view().find_last(s, pos, count); }
    /// Find the last occurrence of a substring
    constexpr size_type find_last(const string& s, size_type pos = npos) const noexcept
        { return substr_view().find_last(static_cast<string_view>(s), pos); }
    /// Find the last occurrence of a substring
    template <string_view_like T>
    constexpr size_type find_last(const T& str, size_type pos = npos)
    { return substr_view().find_last(str, pos); }
    /// Find the last occurrence of a substring
    template <string_view_like T>
    constexpr size_type find_last(const T& str, size_type pos, size_type count)
        { return substr_view().find_last(str, pos, count); }
    /// Find the last occurrence of a substring
    constexpr size_type find_last(char_t ch, size_type pos = npos) const noexcept
        { return substr_view().find_last(ch, pos); }

    /// Finds the last char equal to one of chars in the given sequence
    constexpr size_type find_last_of(const string& s, size_type pos = npos) const noexcept
        { return substr_view().find_last_of(s, pos); }
    /// Finds the last char equal to one of chars in the given sequence
    constexpr size_type find_last_of(const char_t* s, size_type count, size_type pos) const noexcept
        { return substr_view().find_last_of(s, count, pos); }
    /// Finds the last char equal to one of chars in the given sequence
    constexpr size_type find_last_of(const char_t* s, size_type pos = npos) const noexcept
        { return substr_view().find_last_of(s, pos); }
    /// Finds the last char equal to one of chars in the given sequence
    constexpr size_type find_last_of(char_t ch, size_type pos = npos) const noexcept
        { return substr_view().find_last_of(ch, pos); }
        /// Finds the last char equal to one of chars in the given sequence
    template <string_view_like T>
    constexpr size_type find_last_of(const T& svl, size_type pos = npos)
        { return substr_view().find_last_of(svl, pos); }

    /// Finds the last char not equal to one of chars in the given sequence
    constexpr size_type find_last_not_of(const string& s, size_type pos = npos) const noexcept
        { return substr_view().find_last_not_of(s, pos); }
    /// Finds the last char not equal to one of chars in the given sequence
    constexpr size_type find_last_not_of(const char_t* s, size_type count, size_type pos) const noexcept
        { return substr_view().find_last_not_of(s, count, pos); }
    /// Finds the last char not equal to one of chars in the given sequence
    constexpr size_type find_last_not_of(const char_t* s, size_type pos = npos) const noexcept
        { return substr_view().find_last_not_of(s, pos); }
    /// Finds the last char not equal to one of chars in the given sequence
    constexpr size_type find_last_not_of(char_t ch, size_type pos = npos) const noexcept
        { return substr_view().find_last_not_of(ch, pos); }
    /// Finds the last char not equal to one of chars in the given sequence
    template <string_view_like T>
    constexpr size_type find_last_not_of(const T& svl, size_type pos = npos)
        { return substr_view().find_last_not_of(svl, pos); }

    /// Checks if the string begins with the given prefix
    template <string_view_like T>
    constexpr bool starts_with(const T& svl) const noexcept
        { return static_cast<string_view>(*this).starts_with(svl); }
    /// Checks if the string begins with the given char
    constexpr bool starts_with(char_t ch) const noexcept
        { return !is_empty() && (front() == ch); }
    /// Checks if the string begins with the given prefix
    constexpr bool starts_with(const char_t* s) const noexcept
        { return static_cast<string_view>(*this).starts_with(s); }

    /// Checks if the string ends with the given suffix
    template <string_view_like T>
    constexpr bool ends_with(const T& svl) const noexcept
        { return static_cast<string_view>(*this).ends_with(svl); }
    /// Checks if the string ends with the given char
    constexpr bool ends_with(char_t ch) const noexcept
        { return !is_empty() && (back() == ch); }
    /// Checks if the string ends with the given prefix
    constexpr bool ends_with(const char_t* s) const noexcept
        { return static_cast<string_view>(*this).ends_with(s); }

    /// Checks if the string contains the given substring
    template <string_view_like T>
    constexpr bool contains(const T& svl) const noexcept
        { return static_cast<string_view>(*this).contains(svl); }
    /// Checks if the string contains the given char
    constexpr bool contains(char_t ch) const noexcept
        { return static_cast<string_view>(*this).contains(ch); }
    /// Checks if the string contains the given string
    constexpr bool contains(const char_t* s) const noexcept
        { return static_cast<string_view>(*this).contains(s); }

protected:

    /// Implementation for replace methods; combination of erase and insert
    constexpr string& imp_replace(size_type pos, size_type count, string_view sv)
    {
        // If we're not inserting anything then this is just an erasure
        if (sv.is_empty()) [[unlikely]]
            return erase(pos, count);
        // If we're not erasing anything then this is just an insertion
        if (count == 0) [[unlikely]]
            return insert(pos, sv);

        check_pos(pos);

        // Check length only if we'll have a net addition of chars
        if (sv.length() > count)
            check_length(sv.length() - count);

        // If erasure area is larger than insertion area, we can use same
        // buffer. Copy in the inserted string and then erase the balance
        // between the two if necessary.
        auto cmp = count <=> sv.length();
        if (is_gte(cmp)) {
            traits_t::copy(data() + pos, sv.data(), sv.length());
            if (is_gt(cmp))
                return erase(pos + sv.length(), count - sv.length());
            return *this;
        }

        // Erasure area is smaller than insertion area, so length will grow.
        //
        //     |-| -> count
        // aaaaxxxbbbbb : replace xxx with nnnn... (more n's than x's)
        //     ^- pos
        //
        // - The "head" is the stuff to keep before replacement (the a's) [0,pos)
        // - The "tail" is the stuff to keep after replacement (the b's) [pos+count, npos)
        // - The erasure area is the stuff to erase (the x's)
        // - The insertion data is the n's. At this point we know there are
        //   more n's than x's (sv.length() > count)

        // If we have enough capacity now, then do the replacement.
        if (length() - count + sv.length() <= capacity()) {
            // Move the possible "tail" (bbbbb) to make room for the
            // insertion, insert the string, and update the new length
            if (pos + count < length()) {
                traits_t::move(data() + pos + sv.length(), data() + pos + count,
                    (length() - (pos + count)));
            }
            traits_t::copy(data() + pos, sv.data(), sv.length());
            _sbuf.internal_set_length(length() + sv.length() - count);
            return *this;
        }

        // We need to allocate a new buffer. Build up from a new string.
        string tmp(length() + sv.length() - count);
        // Copy the possible "head" (aaaa)
        if (pos) traits_t::copy(tmp.data(), data(), pos);
        // Copy the inserted string
        traits_t::copy(tmp.data() + pos, sv.data(), sv.length());
        // Copy the possible "tail" (bbbbb)
        if (pos + count < length()) {
            traits_t::copy(tmp.data() + pos + sv.length(), data() + pos + count,
                length() - (pos + count));
        }
        return *this = sys::move(tmp);
    }

    /// Implementation for replace with char fill methods
    constexpr string& imp_replace(size_type pos, size_type count, char_t ch, size_type ch_count)
    {
        // If we're not inserting anything then this is just an erasure
        if (0 == ch_count) [[unlikely]]
            return erase(pos, count);
        // If we're not erasing anything then this is just an insertion
        if (count == 0) [[unlikely]]
            return insert(pos, ch, ch_count);

        check_pos(pos);

        // Check length only if we'll have a net addition
        if (ch_count > count)
            check_length(ch_count - count);

        // If erasure area is larger than insertion area, we can use same
        // buffer. Copy in the inserted string and then erase the balance
        // between the two if necessary.
        auto cmp = count <=> ch_count;
        if (is_gte(cmp)) {
            traits_t::fill(data() + pos, ch, ch_count);
            if (is_gt(cmp))
                return erase(pos + ch_count, count - ch_count);
            return *this;
        }

        // If we have enough capacity now, then do the replacement.
        if (length() - count + ch_count <= capacity()) {
            // Move the possible "tail" (bbbbb) to make room for the
            // insertion, insert the string, and update the new length
            if (pos + count < length()) {
                traits_t::move(data() + pos + ch_count, data() + pos + count,
                    (length() - (pos + count)));
            }
            traits_t::fill(data() + pos, ch, ch_count);
            _sbuf.internal_set_length(length() + ch_count - count);
            return *this;
        }

        // We need to allocate a new buffer. Build up from a new string.
        string tmp(length() + ch_count - count);
        // Copy the possible "head" (aaaa)
        if (pos) traits_t::copy(tmp.data(), data(), pos);
        // Copy the inserted string
        traits_t::fill(tmp.data() + pos, ch, ch_count);
        // Copy the possible "tail" (bbbbb)
        if (pos + count < length()) {
            traits_t::copy(tmp.data() + pos + ch_count, data() + pos + count,
                length() - (pos + count));
        }
        return *this = sys::move(tmp);
    }

public:

    // -- Replace

    /// (1) Replace subset of this string [pos, pos+count) with the given string
    constexpr string& replace(size_type pos, size_type count, const string& str)
    {
        return imp_replace(pos, count, str);
    }
    /// (2) Replace subset of this string [first, last) with the given string
    constexpr string& replace(const_iterator first, const_iterator last,
        const string& str)
    {
        check_it(first);
        check_it(last);

        return imp_replace(first - cbegin(), last - first, str);
    }
    /// (3) Replace subset of this string with subset of another string
    constexpr string& replace(size_type pos1, size_type count1,
        const string& str, size_type pos2, size_type count2 = npos)
    {
        return imp_replace(pos1, count1, str.substr_view(pos2, count2));
    }
    // (4)  ---
    // template <class InputIt>
    // constexpr string& replace(const_iterator first, const_iterator last,
    //     InputIt first2, InputIt last2)
    // {
    //     // MOOMOO: TODO: need input iterator definition
    // }
    /// (5) Replace subset of this string with subset of another string
    constexpr string& replace(size_type pos1, size_type count1,
        const char_t* s, size_type count2)
    {
        return imp_replace(pos1, count1, string_view(s, count2));
    }
    /// (6) Replace subset of this string with subset of another string
    constexpr string& replace(const_iterator first, const_iterator last,
        const char_t* s, size_type count2)
    {
        check_it(first);
        check_it(last);

        return imp_replace(first - cbegin(), last - first,
            string_view(s, count2));
    }
    /// (7) Replace subset of this string with subset of another string
    constexpr string& replace(size_type pos1, size_type count1,
        const char_t* s)
    {
        return imp_replace(pos1, count1, string_view(s));
    }
    /// (8) Replace subset of this string with subset of another string
    constexpr string& replace(const_iterator first, const_iterator last,
        const char_t* s)
    {
        check_it(first);
        check_it(last);

        return imp_replace(first - cbegin(), last - first, string_view(s));
    }
    /// (9) Replace subset of this string with a number of characters
    constexpr string& replace(size_type pos1, size_type count1,
        char_t ch, size_type ch_count)
    {
        return imp_replace(pos1, count1, ch, ch_count);
    }
    /// (10) Replace subset of this string with something string_view like
    template <string_view_like T>
    constexpr string& replace(size_type pos1, size_type count1, const T& svl)
    {
        return imp_replace(pos1, count1, svl);
    }
    /// (11) Replace subset of this string with something string_view like
    template <string_view_like T>
    constexpr string& replace(const_iterator first, const_iterator last,
        const T& svl)
    {
        check_it(first);
        check_it(last);

        return imp_replace(first - cbegin(), last - first, svl);
    }
    /// (12) Replace subset of this string with subset of something string_view like
    template <string_view_like T>
    constexpr string& replace(size_type pos1, size_type count1,
        const T& svl, size_type pos2, size_type count2 = npos)
    {
        return imp_replace(pos1, count1, svl.substr_view(pos2, count2));
    }

    // -- Iterators

    constexpr iterator begin() noexcept
        { return iterator(data()); }
    constexpr iterator end() noexcept
        { return iterator(data() + length()); }

    constexpr const_iterator cbegin() const noexcept
        { return const_iterator(data()); }
    constexpr const_iterator cend() const noexcept
        { return const_iterator(data() + length()); }

    // -- Implementation

    constexpr ~string() noexcept = default;

    constexpr void swap(string& other) noexcept
    {
        _sbuf.swap(other._sbuf);
    }

    /// Copy assignment
    constexpr string& operator=(const string& other)
    {
        _sbuf = other._sbuf;
        return *this;
    }

    /// Move assignment
    constexpr string& operator=(string&& other) noexcept
    {
        _sbuf = sys::move(other._sbuf);
        return *this;
    }

    constexpr string& operator=(nullptr_t) = delete;

    /// Replaces contents with a NULL-terminated string
    constexpr string& operator=(const char_t* s)
    {
        auto len = traits_t::length(s);
        traits_t::copy(ensure_buf(len), s, len);

        return *this;
    }

    /// Replace contents with something like a string_view
    template <string_view_like T>
    constexpr string& operator=(const T& svl)
    {
        traits_t::copy(ensure_buf(svl.length()), svl.data(), svl.length());
        return *this;
    }

    constexpr string& operator+=(const string& s)
        { return append(s); }
    constexpr string& operator+=(char_t ch)
        { return append(ch, 1); }
    constexpr string& operator+=(const char_t* s)
        { return append(s); }
    template <string_view_like T>
    constexpr string& operator+=(const T& svl)
        { return append(svl); }

    constexpr operator sys::string_view() const noexcept
    {
        return sys::string_view(data(), length());
    }

    /// Accesses the specified character
    constexpr reference operator[](size_type pos)
    {
        return data()[pos];
    }

    /// Accesses the specified character
    constexpr const_reference operator[](size_type pos) const
    {
        return data()[pos];
    }

    // -- Attributes

    [[nodiscard]] constexpr bool is_empty() const noexcept { return 0 == length(); }

    constexpr const char_t* data() const noexcept
        { return _sbuf.data(); }
    constexpr       char_t* data()       noexcept
        { return _sbuf.data(); }
    constexpr size_type length()   const noexcept
        { return _sbuf.length(); }
    constexpr size_type capacity() const noexcept
        { return _sbuf.capacity(); }
    constexpr size_type max_size() const noexcept
        { return _sbuf.max_size(); }

    /// Returns maximum capacity for short string optimized (sso) strings
    static constexpr size_type sso_capacity() noexcept
        { return imp::string_buf::sso_capacity(); }

private:

    constexpr char_t* ensure_buf(size_type count, bool set_length = true)
    {
        return _sbuf.ensure_buf(count, set_length);
    }

    /// Check that iterator is valid for this string
    constexpr void check_it(const_iterator it) const
    {
        if ((it < cbegin()) || (it > cend()))
            throw sys::error_bounds();
    }

    /// Check position for a copy/insert/erase: [begin, end]
    constexpr void check_pos(size_type pos) const
    {
        if (pos > length())
            throw sys::error_bounds();
    }

    /// Check position for a reference: [begin, end)
    constexpr void check_pos_ref(size_type pos) const
    {
        if (pos >= length())
            throw sys::error_bounds();
    }

    /// Check that current length + count doesn't exceed max_size
    constexpr void check_length(size_type count)
    {
        if (length() + count > max_size())
            throw sys::error_length();
    }

    friend exception::exception(string&&);

    imp::string_buf _sbuf;          ///< The string buffer
};

_SYS_END_NS

#endif // ifndef sys_String__included
