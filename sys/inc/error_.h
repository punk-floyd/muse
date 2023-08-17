/**
 * @file    error_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   System error codes and exceptions
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_error__included
#define sys_error__included

#include <_core_.h>
#include <type_traits_.h>
#include <shared_string_.h>

_SYS_BEGIN_NS

// -- System error codes

enum class error_code : error_t {
    no_error,

    unknown_error,
    not_permitted,
    bad_parameter,
    bad_parameter_nullptr,
    no_such_item,
    io_error,
    allocation_error,
    bad_address,
    out_of_range,
    value_too_large
};

/// Returns true if given error codeis not no_error
constexpr bool is_error(error_code ec) noexcept
{
    return ec != error_code::no_error;
}

class string;

// -- System exceptions

/// A generic exception base class.
class exception
{
public:

    exception() = default;

    /// Construct an exception with a description
    template <string_view_convertible T>
    explicit exception(T message) : _msg(message) {}

    explicit exception(string&& s);

    /// Returns a description of the exception
    string_view get_msg() const noexcept
        { return  _msg.is_empty() ? get_def_msg() : _msg.get_view(); }

    virtual ~exception() noexcept = default;

    exception(const exception&) noexcept = default;
    exception(exception&&)      noexcept = default;

    exception& operator=(const exception&) noexcept = default;
    exception& operator=(exception&&)      noexcept = default;

protected:

    // Returns the default message
    virtual string_view get_def_msg() const noexcept
        { return "An unknown exception occurred"; }

private:

    shared_string       _msg;
};

/// General logic error
class error_logic : public exception {
public:

    error_logic() = default;

    template <string_view_convertible T>
    explicit error_logic (T svl) : exception(svl) {}

protected:

    string_view get_def_msg() const noexcept override
        { return "An logic error occurred"; }
};

/// Assertion error
class error_assert : public error_logic
{
public:

    error_assert() = default;

    template <string_view_convertible T>
    explicit error_assert (T svl) : error_logic(svl) { }

    /// Construct from a message, source file, and line number
    error_assert(string_view msg, string_view file, unsigned line = 0)
        : error_logic(msg), _file(file), _line(line)
    {
    }

    // -- Attributes

    string_view get_filename() const noexcept
        { return _file.get_view(); }
    unsigned get_line_number() const noexcept
        { return _line; }

protected:

    string_view get_def_msg() const noexcept override
        { return "An assertion failed"; }

private:

    shared_string       _file;
    unsigned            _line;
};

/// Out of bounds error
class error_bounds : public error_logic
{
public:

    error_bounds() = default;

    template <string_view_convertible T>
    explicit error_bounds(T svl) : error_logic(svl) { }

protected:

    string_view get_def_msg() const noexcept override
        { return "Out of bounds"; }
};

/// Out of bounds error with bad/max values
template <scalar S = size_t>
class error_bounds_ex  : public error_bounds
{
public:

    using size_type = S;

    error_bounds_ex() = default;

    template <string_view_convertible T>
    explicit error_bounds_ex (T svl) : error_bounds(svl) { }

    template <string_view_convertible T>
    explicit error_bounds_ex (T svl, size_type val_except, size_type val_max)
        : error_bounds(svl), _val_except(val_except), _val_max(val_max)
    {}

    error_bounds_ex (size_type val_except, size_type val_max)
        : _val_except(val_except), _val_max(val_max)
    {}

    // -- Attributes

    size_type get_bad_val() const noexcept { return _val_except; }
    size_type get_max_val() const noexcept { return _val_max;    }

private:

    size_type   _val_max{};
    size_type   _val_except{};
};

/// Length error
class error_length : public error_logic
{
public:

    error_length() = default;

    template <string_view_convertible T>
    explicit error_length(T svl) : error_logic(svl) { }

protected:

    string_view get_def_msg() const noexcept override
        { return "Length too long"; }
};

/// Bad format string
class error_format : public error_logic
{
public:

    error_format() = default;

    template <string_view_convertible T>
    explicit error_format(T svl) : error_logic(svl) { }

protected:

    string_view get_def_msg() const noexcept override
        { return "Bad format string"; }
};

/// Bad parameter
class error_parameter : public error_logic
{
public:

    error_parameter() = default;

    template <string_view_convertible T>
    explicit error_parameter(T svl) : error_logic(svl) { }

protected:

    string_view get_def_msg() const noexcept override
        { return "Invalid parameter"; }
};

/// Bad variant access
class error_variant_access : public error_logic
{
public:

    error_variant_access() = default;

    template <string_view_convertible T>
    explicit error_variant_access(T svl) : error_logic(svl) { }

protected:

    string_view get_def_msg() const noexcept override
        { return "Bad variant access"; }
};

/// Errors beyond the scope of the program
class error_runtime : public exception {};
    // System related error
    class error_system : public error_runtime {};
    // Memory allocation failure
    class error_malloc : public error_runtime {};

inline void assert(bool b, string_view msg = "")
{
    if (!b) [[unlikely]]
        throw error_assert(msg);
}

_SYS_END_NS

#endif // ifndef sys_error__included
