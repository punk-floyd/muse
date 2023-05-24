/**
 * @file    fmt_base.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Base definitions for format implementation
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_fmt_base_included
#define sys_imp_fmt_base_included

#include <_core_.h>

_SYS_BEGIN_NS

// -- Forward declarations -------------------------------------------------

class parse_context_base;
class basic_format_args;
class format_arg;

namespace imp {

/// Ensure all arguments are supported by formatter<T>
template <class Arg, class... Args>
constexpr inline void check_format_args();

/// Check format string with given argument types
template <class... FmtArgs>
constexpr void check_format_string(string_view fmt);

/// Check format string with no arguments
constexpr void check_format_string_no_args(string_view fmt);

}   // end namespace imp

// -- Formatter definitions ------------------------------------------------

/// Type erased interface for a formatter to send its output
struct formatter_sink
{
    virtual void output(const char& c) = 0;
    virtual void output(string_view sv)
        { for (const auto& c : sv) output(c); }
};

/// Type erased context object for format operations
class format_context
{
public:

    constexpr format_context(formatter_sink& sink, const basic_format_args& fmt_args)
        : _sink(sink), _fmt_args(fmt_args)
    {}

    constexpr const formatter_sink& get_sink() const noexcept { return _sink; }
    constexpr       formatter_sink& get_sink()       noexcept { return _sink; }

    constexpr const basic_format_args& get_format_args() const noexcept
        { return _fmt_args; }

private:

    formatter_sink&             _sink;
    const basic_format_args&    _fmt_args;
};

/// Type erased interface for a formatter object
struct formatter_base
{
    /// Parse the given replacement field
    constexpr virtual void parse(string_view rf, parse_context_base& pctx) = 0;
    /// Format output into given sink
    virtual void format(const format_arg& fmt_arg, format_context& fmt_ctx) = 0;
};

/// The default, unusable formatter
template <class T>
struct formatter final : public formatter_base
{
    constexpr formatter() = default;

    constexpr void parse(string_view, parse_context_base&) override
        { static_assert(dependent_false_v<T>, "Unsupported type for format"); }
};

namespace imp {

    /// fmt_store_work: Recursive storage for formatter objects
    template <class... Args>
    struct fmt_store_work;      // Primary template

    template <class Arg, class... Args>
    struct fmt_store_work<Arg, Args...>
    {
        formatter<Arg>              fmt{};
        fmt_store_work<Args...>     store;
    };
    template <class Arg>
    struct fmt_store_work<Arg>
    {
        formatter<Arg>              fmt{};
    };
    template <>
    struct fmt_store_work<>
    {
    };
}

/// Storage for formatters
template <class... Args>
class formatter_store
{
public:

    constexpr formatter_store() = default;
    constexpr ~formatter_store() = default;

    constexpr formatter_base& get_formatter(size_t idx)
    {
        if constexpr (sizeof...(Args) > 0)
            return get_formatter_work(idx, _fmt_store);
        else
            throw error_bounds{};
    }

protected:

    template <size_t idx_at = 0, class FArg, class... FArgs>
    constexpr formatter_base& get_formatter_work(size_t idx,
        imp::fmt_store_work<FArg, FArgs...>& fs)
    {
        if (idx == idx_at)
            return fs.fmt;

        if constexpr (sizeof...(FArgs))
            return get_formatter_work<idx_at + 1, FArgs...>(idx, fs.store);
        else
            throw error_bounds{};
    }

private:

    using fmt_store_t = imp::fmt_store_work<remove_cvref_t<Args>...>;

    fmt_store_t     _fmt_store;
};

// -- Format string --------------------------------------------------------

/// Encapsulates a format string with compile time format checking
template <class... FmtArgs>
class basic_format_string
{
public:

    template <class T>
        requires (sys::is_convertible_v<const T&, sys::string_view>)
    consteval inline basic_format_string(const T& str) : _str(str)
    {
        if constexpr (sizeof...(FmtArgs) == 0) {
            imp::check_format_string_no_args(get_view());
        }
        else {
            // Make sure all arguments support formatter<T>
            imp::check_format_args<FmtArgs...>();
            // Check the validity of the format string
            imp::check_format_string<FmtArgs...>(get_view());
        }
    }

    inline operator sys::string_view() const noexcept { return _str; }
    inline constexpr sys::string_view get_view() const noexcept { return _str; }

private:
    sys::string_view _str;
};

template <class... FmtArgs>
using format_string = basic_format_string<sys::type_identity_t<FmtArgs>...>;

_SYS_END_NS

#endif // ifndef sys_imp_fmt_base_included
