/**
 * @file    fmt_args.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   format argument management
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_fmt_args_included
#define sys_imp_fmt_args_included

#include <_core_.h>
#include <variant_.h>
#include <utility_.h>

_SYS_BEGIN_NS

/// A single format argument
class format_arg
{
public:

    using arg_type = variant<
        // Invalid type for an argument
        monostate,
        // Integral types
        signed char,    unsigned char,
        int,            unsigned int,
        short,          unsigned short,
        long,           unsigned long,
        long long,      unsigned long long,
        uint128_t,      sint128_t,
        // The bool type
        bool,
        // Pointer types
        const void*, void*, nullptr_t,
        // String types
        string_view
    >;

    constexpr format_arg() = default;

    template <class T>
    constexpr format_arg(T&& val)
        : _value(in_place_type<remove_cvref_t<T>>, forward<T>(val))
    {}

    constexpr arg_type get_variant() const noexcept { return _value; }

private:

    template <class... FmtArgs> friend class format_arg_store;

    arg_type        _value{};
};

/// Interface for an format arguments storage
class format_arg_store_base
{
public:

    constexpr format_arg_store_base() = default;
    constexpr virtual ~format_arg_store_base() = default;

    constexpr virtual size_t get_arg_count() const noexcept = 0;
    constexpr virtual format_arg get_arg(size_t idx) const noexcept = 0;
    constexpr virtual formatter_base& get_formatter(size_t idx) = 0;
};

/// Format argument store implementation
template <class... FmtArgs>
class format_arg_store final : public format_arg_store_base
{
public:

    /// Construct a store with no arguments, but WITH formatters!
    constexpr format_arg_store() = default;

    /// Construct a store with the specified arguments
    constexpr format_arg_store(FmtArgs&&... args)
        requires (sizeof...(FmtArgs) > 0)
    { init_store<0, FmtArgs...>(forward<FmtArgs>(args)...); }

    constexpr ~format_arg_store() = default;

    /// Returns the number of arguments/formatters in the store
    constexpr size_t get_arg_count() const noexcept override
        { return sizeof...(FmtArgs); }

    /// Returns the format argument at the specified index
    constexpr format_arg get_arg(size_t idx) const noexcept override
        { return (idx < get_arg_count()) ? _args[idx] : format_arg{}; }

    /// Returns the formatter object at the specified index
    constexpr formatter_base& get_formatter(size_t idx) override
        { return _fmts.get_formatter(idx); }

private:

    /// Type that holds formatter objects
    using fmt_store = formatter_store<remove_cvref_t<FmtArgs>...>;
    /// Type that holds formatting arguments
    using arg_store = array<format_arg, sizeof...(FmtArgs)>;
    /// The type of a single formatting argument
    using arg_type  = format_arg::arg_type;

    template <size_t Idx, class Arg>
    constexpr void init_store(Arg&& arg)
    {
        _args[Idx]._value =
            arg_type {in_place_type<remove_cvref_t<Arg>>, forward<Arg>(arg)};
    }

    template <size_t Idx, class Arg, class... Args>
    constexpr void init_store(Arg&& arg, Args&&... args)
    {
        _args[Idx]._value =
            arg_type {in_place_type<remove_cvref_t<Arg>>, forward<Arg>(arg)};

        if constexpr (sizeof...(Args) > 0)
            init_store<Idx + 1, Args...>(forward<Args>(args)...);
    }

    arg_store   _args;  ///< Format arguments
    fmt_store   _fmts;  ///< Formatter objects
};

class basic_format_args
{
public:

    template <class... FmtArgs>
    constexpr basic_format_args(format_arg_store<FmtArgs...>& store)
        : _store(store)
    {}

    constexpr size_t get_arg_count() const noexcept
        { return _store.get_arg_count(); }

    constexpr format_arg get_arg(size_t idx) const noexcept
        { return _store.get_arg(idx); }

    constexpr formatter_base& get_formatter(size_t idx)
        { return _store.get_formatter(idx); }

private:

    format_arg_store_base&   _store;
};

namespace imp {

/// Check all format arguments to ensure they are formattable
template <class Arg, class... Args>
constexpr inline void check_format_args()
{
    [[maybe_unused]] formatter<remove_cvref_t<Arg>> czech_format;
    if constexpr (sizeof...(Args))
        check_format_args<Args...>();
}

} // end namespace imp

_SYS_END_NS

#endif // ifndef sys_imp_fmt_args_included
