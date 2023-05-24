/**
 * @file    shared_string_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   sys::shared_string: A shared, read-only null-terminated string
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_shared_string__included
#define sys_shared_string__included

#include <_core_.h>
#include <atomic_.h>
#include <iterator_.h>
#include <string_view_.h>
#include "imp/string_buf.h"

_SYS_BEGIN_NS

class shared_string
{
public:

    using char_t         = char;
    using size_type      = size_t;

    using iterator_type = it_contig<const char_t>;

    // -- Constructors

    shared_string() noexcept = default;

    template <class T>
        requires sys::is_convertible_v<T, sys::string_view>
    explicit shared_string(T str)
        : _state(new shared_state(str))
    {}

    shared_string(imp::string_buf&& sbuf)
        : _state(new shared_state(sys::move(sbuf)))
    {}

    // Copy constructor
    shared_string(shared_string& rhs) noexcept
        : _state(rhs._state ? rhs._state->add_ref() : nullptr)
    {}

    // Move constructor
    shared_string(shared_string&& rhs) noexcept
        : _state(rhs._state)
    {
        rhs._state = nullptr;   // Invalidate other side
    }

    /// Returns a string_view of underlying string
    string_view get_view() const noexcept
    {
        if (_state) {
            const auto& sbuf = _state->get_sbuf();
            return string_view(sbuf.data(), sbuf.length());
        }

        return string_view();
    }

    // -- Accessors

    /// Returns true if underlying string is empty
    bool is_empty() const noexcept
    {
        return length() == 0;
    }

    /// Access underlying string as a null terminated array
    const char_t* c_str() const noexcept
    {
        return data();
    }

    /// Returns a pointer to the first character of a string
    const char_t* data() const noexcept
    {
        return _state ? _state->get_sbuf().data() : "";
    }

    /// Returns the length of the string in characters, not including null terminator
    size_type length() const noexcept
    {
        return _state ? _state->get_sbuf().length() : 0;
    }

    // -- Iterators

    iterator_type begin() noexcept
        { return iterator_type(data()); }
    iterator_type end() noexcept
        { return iterator_type(data() + length()); }

    iterator_type cbegin() const noexcept
        { return iterator_type(data()); }
    iterator_type cend() const noexcept
        { return iterator_type(data() + length()); }

    // -- Implementation

    ~shared_string() noexcept
    {
        release();
    }

    // Copy assignment
    shared_string& operator=(const shared_string& other) noexcept
    {
        if (this == &other) [[unlikely]]
            return *this;

        release();

        _state = other._state ? other._state->add_ref() : nullptr;

        return *this;
    }

    // Move assignment
    shared_string& operator=(shared_string&& other) noexcept
    {
        if (this == &other) [[unlikely]]
            return *this;

        release();

        _state = other._state;
        other._state = nullptr;

        return *this;
    }

private:

    void release() noexcept
    {
        if (_state) {
            _state->release();
            _state = nullptr;
        }
    }

    struct shared_state {

        // -- Construction

        shared_state() noexcept = default;
        shared_state(string_view view) noexcept : sbuf(view) {}
        shared_state(imp::string_buf&& sbuf) noexcept : sbuf(sys::move(sbuf)) {}

        // -- Attributes

        imp::string_buf& get_sbuf() noexcept
            { return sbuf; }
        const imp::string_buf& get_sbuf() const noexcept
            { return sbuf; }

        /// Add a reference and return this
        shared_state* add_ref() noexcept
        {
            ref_count++;
            return this;
        }

        /// Drop a reference, deleting when 0
        void release() noexcept
        {
            if (0 == --ref_count)
                delete this;
        }

        private:

        atomic<unsigned>    ref_count{1};
        imp::string_buf     sbuf{};
    };

    shared_state*   _state{nullptr};
};

_SYS_END_NS

#endif // ifndef sys_shared_string__included
