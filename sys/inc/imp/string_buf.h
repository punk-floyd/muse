/**
 * @file    string_buf.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Implements a SSO-optimized buffer for null-terminated string objects
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_string_buf__included
#define sys_imp_string_buf__included

#include <string_view_.h>
#include <limits_.h>

_SYS_BEGIN_NS
namespace imp {

class string_buf
{
public:

    using char_t         = char;
    using size_type      = size_t;
    using traits_t       = char_traits<char_t>;

    /// Default constructor; constructs an empty string ("")
    constexpr string_buf() noexcept = default;

    template <string_view_like T>
    constexpr string_buf(T svl)
    {
        traits_t::copy(ensure_buf(svl.length()), svl.data(), svl.length());
    }

    /// Copy constructor
    constexpr string_buf(const string_buf& other)
    {
        traits_t::copy(ensure_buf(other.capacity(), false), other.data(), other.length());
        internal_set_length(other.length());
    }

    /// Move constructor
    constexpr string_buf(string_buf&& other) noexcept
    {
        // Do this first; it determines long/short mode
        _state.len = other._state.len;

        if (is_long_mode()) {
            _state.ls = other._state.ls;

            // Invalidate the other side only if it's in long mode
            other._state.ls.dat = nullptr;
            other._state.ls.cap = 0;
            other._state.len    = 0;
        }
        else {
            _state.ss = other._state.ss;
        }
    }

    [[nodiscard]] constexpr bool is_empty() const noexcept { return 0 == length(); }

    constexpr const char_t* data() const noexcept
        { return is_long_mode() ? _state.ls.dat : _state.ss.dat; }
    constexpr       char_t* data()       noexcept
        { return is_long_mode() ? _state.ls.dat : _state.ss.dat; }
    constexpr size_type length()   const noexcept
        { return internal_get_length(); }
    constexpr size_type capacity() const noexcept
        { return is_long_mode() ? _state.ls.cap : sso_capacity(); }
    static constexpr size_type max_size() noexcept
        { return size_type(-1) - lm_bit; }

    /** Calculate the new capacity to use for a minimal requested capacity
     *
     * This function is called when we've determined that adding one or more
     * elements will exceed the current capacity. It figures out what the
     * new capacity should be.
     *
     * This function does not adjust the capacity!
     *
     * @param cap_request   The requested capacity, which serves as the
     *  minimal capacity that we'll need. Depending on state, we may want
     *  to allocate more than this.
     *
     * @return Returns the size that we should increase the capacity to.
    */
    constexpr size_type calc_new_capacity(size_type cap_request) const
    {
        // Make sure requested minimal capacity is not too large
        if (cap_request > max_size()) [[unlikely]]
            throw_error_length();

        // If we are presently empty, then we'll use the requested capacity
        // as-is. This will be the case for things like construction or an
        // initial assignment. It'll require a single allocation of optimal
        // length.
        if (0 == capacity())
            return cap_request;

        // If we're not empty, then we're growing the buffer. In this case,
        // we want to double the current capacity until it's large enough to
        // hold the requested capacity.
        size_type cap_try = capacity() * 2;
        if (0 == cap_try)
            cap_try = 1;
        while (cap_try < cap_request) {
            if (sys::multiply_overflow(cap_try, 2, cap_try) || (cap_try > max_size())) [[unlikely]]
                cap_try = cap_request;
        }

        return cap_try;
    }

    constexpr char_t* ensure_buf(size_type count, bool set_length = true, bool explicit_reserve = false)
    {
        if (count > capacity()) {

            // count + 1: +1 for the NULL terminator
            size_type new_cap =
                explicit_reserve ? count + 1 : calc_new_capacity(count + 1);

            char_t* new_data = new char_t[new_cap];
            if (data() && length())
                traits_t::copy(new_data, data(), length());

            // We'll need to free the old buffer if we're already in long mode
            auto buf_free = is_long_mode() ? _state.ls.dat : nullptr;

            _state.ls.dat = new_data;
            _state.ls.cap = count;
            _state.len |= lm_bit;           // This enables long mode

            delete[] buf_free;
        }

        if (set_length)
            internal_set_length(count);

        return data();
    }

    /// Clear string content (including reserve)
    constexpr void clear() noexcept
    {
        if (is_long_mode()) {
            delete[] _state.ls.dat;
            _state.ls.dat = nullptr;
            _state.ls.cap = 0;
        }

        _state.len = 0;     // This disables long mode
    }

    // -- Implementation

    /// Returns maximum capacity for short string optimized (sso) strings
    static constexpr size_type sso_capacity() noexcept
        { return sso_capacity_chars - 1; }  // One byte for null terminator

    constexpr ~string_buf() noexcept
    {
        clear();
    }

    constexpr void swap(string_buf& other) noexcept
    {
        // MOOMOO : Default to a generic templatized swap?
        string_buf tmp(sys::move(other));
        other = sys::move(*this);
        *this = sys::move(tmp);
    }

    /// Replaces the contents with a copy of str
    constexpr string_buf& operator=(const string_buf& other)
    {
        if (this == &other) [[unlikely]]
            return *this;

        if (capacity() != other.capacity())
            clear();

        traits_t::copy(ensure_buf(other.capacity(), false), other.data(), other.length());
        internal_set_length(other.length());

        return *this;
    }

    /// Move assignment
    constexpr string_buf& operator=(string_buf&& other) noexcept
    {
        if (this == &other) [[unlikely]]
            return *this;

        if (!is_empty())
            clear();

        // Do this first; it determines long/short mode
        _state.len = other._state.len;

        if (is_long_mode()) {
            _state.ls = other._state.ls;

            // Invalidate the other side only if it's in long mode
            other._state.ls.dat = nullptr;
            other._state.ls.cap = 0;
            other._state.len    = 0;
        }
        else {
            _state.ss = other._state.ss;
        }

        return *this;
    }

    /// Change the length of our string; sets NULL terminator
    constexpr void internal_set_length(size_type len)
    {
        if (is_long_mode())
            _state.len = len |  lm_bit;
         else
            _state.len = len & ~lm_bit;

        data()[len] = char_t(0);
    }

private:

    /// Returns true if we're in long mode (heap-allocated data)
    constexpr bool is_long_mode() const noexcept
    {
        return 0 != (_state.len & lm_bit);
    }

    constexpr size_type internal_get_length() const noexcept
    {
        return _state.len & ~lm_bit;
    }

    /// State for "long mode": heap-allocated memory for data
    struct long_state {
        char_t*     dat{nullptr};   ///< String buffer
        size_type   cap{0};         ///< String capacity
    };

    // Short string optimization: Rather than always using heap-allocated
    // memory, we can use the string state itself as storage. I initially
    // included length in the shared state which gives us 7 more bytes of
    // storage, but this does not play well with constexpr code since we
    // need somthing to tell us which union element is the active member.

    // Short string capacity limit; one byte reserved for length
    static constexpr size_t sso_capacity_chars =
        (sizeof(long_state) / sizeof(char_t)) - 1;

    /// State for "short mode": local-storage of data for short strings
    struct short_state {
        char_t          dat[sso_capacity_chars]{};
    };

    /// MSB of long_state/short_state::len is set for long mode
    static constexpr size_type lm_bit = sys::msb<size_type>();

    struct string_state {

        // Default to short mode
        constexpr string_state() noexcept : ss() {}

        union {
            long_state  ls;     ///< Long state  (heap-allocated buffer)
            short_state ss;     ///< Short state (local buffer)
        };

        /// Length of our string; MSB flags long mode (heap-alloc data)
        size_type       len{0};
    };

    string_state    _state;
};


}
_SYS_END_NS

#endif // sys_imp_string_buf__included
