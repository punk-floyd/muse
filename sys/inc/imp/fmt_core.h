/**
 * @file    fmt_core.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Core definitions for format implementation
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <_core_.h>
#include <type_traits_.h>
#include <string_view_.h>
#include <concepts_.h>
#include <charconv_.h>
#include <iterator_.h>
#include <array_.h>
#include <variant_.h>
#include <string_.h>
#include <error_.h>
#include "fmt_buf.h"

_SYS_BEGIN_NS

// For muse: do an exception class derived from error_format that can contain parse error details

/// The default, unusable formatter
template <class T>
struct formatter
{
    constexpr formatter() = default;

    template <class ParseCtx>
    constexpr auto parse(ParseCtx&) -> ParseCtx::iterator
    {
        throw error_format{ "Unsupported type for format" };
    }

    template <class FormatCtx>
    constexpr auto format(T, FormatCtx&) -> FormatCtx::iterator
    {
        throw error_format{ "Unsupported type for format" };
    }
};

// ---------------- Format string ------------------------------------------

namespace imp {
    template <class... FmtArgs>
    static constexpr void check_format(sys::string_view sv);
}

template <class... FmtArgs>
class basic_format_string
{
public:

    template <sys::convertible_to<sys::string_view> FmtStr>
    consteval inline basic_format_string(const FmtStr& fmt_str)
        : _fmt_str(fmt_str)
    {
        //imp::check_format<FmtArgs...>(get_view());
    }

    constexpr sys::string_view get_view() const noexcept { return _fmt_str; }

private:

    sys::string_view    _fmt_str;
};

template <class... FmtArgs>
using format_string = basic_format_string<sys::type_identity_t<FmtArgs>...>;

// ---------------- Handle for custom formatters ---------------------------

class parse_context;

template <class FormatCtx>
class handle
{
    using format_func = void(*)(parse_context&, FormatCtx&, const void*);

    format_func _func{ nullptr };
    const void* _arg{ nullptr };

public:

    template <class Formattable>
    handle(const Formattable& obj)
        : _arg(&obj)
    {
        using TD = sys::remove_const_t<Formattable>;
        using TQ = const TD;

        _func = [](parse_context& p_ctx, FormatCtx& f_ctx, const void* argv) {
            formatter<TD> f;
            p_ctx.advance_to(f.parse(p_ctx));
            f_ctx.advance_to(f.format(*static_cast<TQ*>(argv), f_ctx));
        };
    }

    void format(parse_context& p_ctx, FormatCtx& f_ctx)
    {
        _func(p_ctx, f_ctx, _arg);
    }
};

// ---------------- Format arguments ---------------------------------------

template <class Context>
using basic_format_arg = sys::variant<
        // Invalid type for an argument
        monostate,
        // Integral types
        uint8_t, uint16_t, uint32_t, uint64_t, uint128_t,
        sint8_t, sint16_t, sint32_t, sint64_t, sint128_t,
        // The bool type
        bool,
        // Pointer types
        const void*, nullptr_t,
        // String types
        const char*, string_view,
        // User-defined types
        handle<Context>
>;

template <class Context, class... FmtArgs>
struct format_args_store
{
    constexpr format_args_store() = default;

    template <class... CxFmtArgs>
    constexpr format_args_store(CxFmtArgs&&... args)
        requires ((sizeof...(CxFmtArgs) > 0) && (sizeof...(CxFmtArgs) == sizeof...(FmtArgs)))
    {
        init_store<0, CxFmtArgs...>(sys::forward<CxFmtArgs>(args)...);
    }

    constexpr basic_format_arg<Context> get(sys::size_t idx) const noexcept
    {
        if (idx < sizeof...(FmtArgs))
            return _args[idx];

        return basic_format_arg<Context>{};
    }

private:

    template <class T>
    constexpr basic_format_arg<Context> init_arg(T&& val)
    {
        using Ts = remove_cvref_t<T>;
        using Td = remove_cvref_t<decay_t<T>>;

        if constexpr (is_same_v<Ts, bool> || is_same_v<Ts, typename Context::char_type>)
            return forward<T>(val);
        // Pull any (non-bool) integral value into its explicitly sized equivalent.
        else if constexpr (is_integral_v<Ts> && !is_same_v<Ts, bool>) {
            if constexpr (is_unsigned_v<Ts>) {
                if      constexpr (sizeof(Ts) ==  1) return static_cast<uint8_t>(val);
                else if constexpr (sizeof(Ts) ==  2) return static_cast<uint16_t>(val);
                else if constexpr (sizeof(Ts) ==  4) return static_cast<uint32_t>(val);
                else if constexpr (sizeof(Ts) ==  8) return static_cast<uint64_t>(val);
                else if constexpr (sizeof(Ts) == 16) return static_cast<uint128_t>(val);
                else static_assert(dependent_false_v<T>, "Unsigned integral type is too large");
            }
            else if constexpr (is_signed_v<Ts>) {
                if      constexpr (sizeof(Ts) ==  1) return static_cast<sint8_t>(val);
                else if constexpr (sizeof(Ts) ==  2) return static_cast<sint16_t>(val);
                else if constexpr (sizeof(Ts) ==  4) return static_cast<sint32_t>(val);
                else if constexpr (sizeof(Ts) ==  8) return static_cast<sint64_t>(val);
                else if constexpr (sizeof(Ts) == 16) return static_cast<sint128_t>(val);
                else static_assert(dependent_false_v<T>, "Signed integral type is too large");
            }
            else
                static_assert(dependent_false_v<T>, "Integral type is neither signed nor unsigned");
        }
        else if constexpr (is_floating_point_v<Ts>)
            return forward<T>(val);
        else if constexpr (is_same_v<Ts, string_view> || is_same_v<Ts, string>)
            return string_view(val.data(), val.size());
        else if constexpr (is_pointer_v<Td>) {
            if constexpr (is_same_v<Td, const char*> || is_same_v<Td, char*>)
                return static_cast<const char*>(val);
            else
                return static_cast<const void*>(val);
        }
        else
            return handle<Context>(val);
    }

    template <size_t Idx, class Arg, class... Args>
    constexpr void init_store(Arg&& arg, Args&&... args)
    {
        if constexpr (Idx < sizeof...(FmtArgs)) {
            _args[Idx] = init_arg(forward<Arg>(arg));

            if constexpr (sizeof...(Args) > 0)
                init_store<Idx + 1, Args...>(sys::forward<Args>(args)...);
        }
    }

    using arg_store = sys::array<basic_format_arg<Context>, sizeof...(FmtArgs)>;

    arg_store   _args{};
};

// Deduction guide
template <class Context, class... FmtArgs>
format_args_store(FmtArgs...) -> format_args_store<Context, FmtArgs...>;

template <class Context>
class basic_format_args
{
public:
    constexpr virtual ~basic_format_args() = default;
    constexpr virtual basic_format_arg<Context> get(sys::size_t idx) const noexcept = 0;
    constexpr virtual size_t count() const noexcept = 0;
};

template <class Context, class... FmtArgs>
class format_args_model final : public basic_format_args<Context>
{
public:

    template <class... CxFmtArgs>
    constexpr format_args_model(CxFmtArgs&&... args)
        : _store(sys::forward<CxFmtArgs>(args)...)
    {}

    constexpr ~format_args_model() = default;

    constexpr basic_format_arg<Context> get(sys::size_t idx) const noexcept override
        { return _store.get(idx); }

    constexpr size_t count() const noexcept
        { return sizeof...(FmtArgs); }

private:

    format_args_store<Context, FmtArgs...>   _store;
};

// Deduction guide
template <class Context, class... FmtArgs>
format_args_model(FmtArgs...) -> format_args_model<Context, FmtArgs...>;

// ---------------- Format and parse contexts ------------------------------

//template <sys::output_iterator<char> OutputIt>  MOOMOO iterator concept
template <class OutputIt>
class basic_format_context
{
public:

    using char_type = char;
    using iterator = OutputIt;

    using format_args = basic_format_args<basic_format_context<OutputIt>>;
    using format_arg  = basic_format_arg<basic_format_context<OutputIt>>;

    explicit constexpr basic_format_context(const format_args& args, OutputIt&& it) noexcept
        : _args(args), _it(sys::move(it))
    {}

    constexpr format_arg get_arg(size_t idx) const noexcept
        { return _args.get(idx); }

    constexpr [[nodiscard]] iterator out()
        { return _it; }

    constexpr void advance_to(OutputIt it)
        { _it = move(it); }

private:

    const format_args&  _args;
    OutputIt            _it;
};

using format_context = basic_format_context<back_insert_iterator<imp::fmt_buf<512>>>;

using format_args = basic_format_args<format_context>;

class parse_context
{
public:

    using const_iterator = sys::string_view::iterator_type;
    using iterator       = sys::string_view::iterator_type;

    constexpr parse_context(sys::string_view fmt_str, sys::size_t arg_count)
        : _fmt_str(fmt_str), _arg_count(arg_count)
    {}

    constexpr sys::string_view get_fmt_str() const noexcept { return _fmt_str; }
    constexpr sys::size_t get_arg_count() const noexcept { return _arg_count; }

    constexpr const_iterator begin() const noexcept { return _fmt_str.begin(); }
    constexpr const_iterator end()   const noexcept { return _fmt_str.end(); }
    constexpr bool is_empty() const noexcept { return begin() == end(); }
    constexpr void advance_to(const_iterator it)
    {
        _fmt_str.remove_prefix(it == end() ? _fmt_str.size() : (&*it - _fmt_str.data()));
    }
    constexpr void advance_by(size_t count)
    {
        _fmt_str.remove_prefix(count);
    }

    [[nodiscard]] constexpr sys::size_t next_arg_index()
    {
        if (_arg_mode == arg_index_mode_t::Initial)
            _arg_mode = arg_index_mode_t::Auto;
        if (_arg_mode != arg_index_mode_t::Auto)
            throw error_format("Cannot mix manual and auto argument index modes");
        if (_arg_auto_idx >= get_arg_count())
            throw error_format{ "Format argument missing" };

        return _arg_auto_idx++;
    }

    constexpr size_t check_arg_index(size_t idx)
    {
        if (_arg_mode == arg_index_mode_t::Initial)
            _arg_mode = arg_index_mode_t::Manual;
        if (_arg_mode != arg_index_mode_t::Manual)
            throw error_format{ "Cannot mix manual and auto argument index modes" };
        if (idx >= get_arg_count())
            throw error_format{ "Invalid argument index" };

        return idx;
    }

    constexpr void set_current_arg_idx(size_t idx)
    {
        if (idx >= get_arg_count())
            throw error_format{ "Invalid argument index" };
        _arg_cur_idx = idx;
    }

    constexpr size_t get_current_arg_idx() const noexcept
    {
        return _arg_cur_idx;
    }

private:

    enum class arg_index_mode_t { Initial, Auto, Manual };

    sys::string_view    _fmt_str;

    arg_index_mode_t    _arg_mode{};
    sys::size_t         _arg_count;
    sys::size_t         _arg_auto_idx{};
    sys::size_t         _arg_cur_idx{};
};

template <class Context = format_context, class... FmtArgs>
constexpr auto make_format_args(FmtArgs&... args)
{
    return format_args_model<Context, FmtArgs...>(args...);
}

// ---------------- Format parsing -----------------------------------------

namespace imp {

template <size_t Idx, size_t Count, class Arg, class... Args>
    requires (Idx < Count)
static constexpr void check_format_arg(parse_context& p_ctx)
{
    if (p_ctx.get_current_arg_idx() == Idx) {
        using sys::formatter;
        formatter<sys::remove_cvref_t<Arg>> f;
        p_ctx.advance_to(f.parse(p_ctx));
    }
    else {
        if constexpr (Idx + 1 < Count)
            check_format_arg<Idx + 1, Count, Args...>(p_ctx);
        else
            throw error_format{ "Invalid argument" };     // Necessary?
    }
}

static constexpr void validate_rf_arg_idx(parse_context& p_ctx)
{
    if (p_ctx.is_empty())
        throw error_format{ "Unterminated replacement field" };

    // p_ctx.begin() is the char after the opening '{'
    //
    // Valid possibilities here: {} {arg-id} {arg-id:fmt-spec} {:fmt-spec}
    //  Auto or Manual mode:      A  M        M                 A
    //
    // This function will advance through arg-id and ':' so that on exit
    // p_ctx.begin() will refer to the first char of fmt-spec or the closing
    // '}' (or end of string, which will be picked up in the next step)

    // Is this an auto indexing mode?
    const auto c = *p_ctx.begin();
    if ((c == '}') || (c == ':')) {
        p_ctx.set_current_arg_idx(p_ctx.next_arg_index());
        if (c == ':')
            p_ctx.advance_to(p_ctx.begin() + 1);
        return;
    }

    // This is a manual indexing operation. Parse out the arg-id, which must
    // be a decimal size_t.
    constexpr auto npos = string_view::npos;
    auto sv = p_ctx.get_fmt_str();
    size_t arg_id{};
    auto [pos, ec] = from_chars(arg_id, sv);
    // arg-id may only be terminated by ':' or '}'
    if ((is_error(ec)) || (pos == npos) || ((sv[pos] != ':') && (sv[pos] != '}')))
        throw error_format{ "Failed to parse argument index value" };
    p_ctx.set_current_arg_idx(p_ctx.check_arg_index(arg_id));
    if (sv[pos] == ':')
        ++pos;
    p_ctx.advance_by(pos);
}

template <class OutputIt>
static constexpr bool find_next_rf(parse_context& p_ctx, basic_format_context<OutputIt>& f_ctx)
{
    auto it_in  = p_ctx.begin();
    auto it_out = f_ctx.out();

    // Walk until we find the first solo '{'
    for (char last_char{}; it_in != p_ctx.end(); ++it_in) {
        if (last_char == '{') {

            // Are we are at the start of a replacement field?
            if (*it_in != '{')
                break;

            // We are at an escaped {
            last_char = {};
            *it_out++ = '{';
            continue;
        }

        if (last_char == '}') {

            //  Are we at an non-escaped (i.e., invalid) '}'?
            if (*it_in != '}')
                throw error_format("Unexpected '}'");

            // We are at an escaped }
            last_char = {};
            *it_out++ = '}';
            continue;
        }

        last_char = *it_in;

        if ((*it_in != '{') && (*it_in != '}'))
            *it_out++ = *it_in;
    }
    p_ctx.advance_to(it_in);
    f_ctx.advance_to(it_out);

    // If we have a replacement field, then validate the argument index and
    // make it accessible for the concrete parser.
    if (!p_ctx.is_empty())
        validate_rf_arg_idx(p_ctx);

    return !p_ctx.is_empty();
}

template <class... FmtArgs>
static constexpr void check_format(sys::string_view fmt)
{
    imp::fmt_buf buf;
    auto no_args = make_format_args();
    format_context f_ctx{no_args, back_insert_iterator(buf)};
    parse_context  p_ctx{fmt, sizeof...(FmtArgs)};

    while (find_next_rf(p_ctx, f_ctx)) {
        if constexpr (sizeof...(FmtArgs))
            check_format_arg<0, sizeof...(FmtArgs), FmtArgs...>(p_ctx);
        else
            throw error_format("Missing format argument");
    }
}

template <class OutputIt>
static constexpr void do_format(parse_context& p_ctx,
    basic_format_context<OutputIt>& f_ctx)
{
    while (find_next_rf(p_ctx, f_ctx)) {

        auto visitor = [&p_ctx, &f_ctx](auto a) {
            if constexpr (is_specialization_v<decltype(a), handle>) {
                a.format(p_ctx, f_ctx);
            }
            else {
                using sys::formatter;
                formatter<decltype(a)> f;
                p_ctx.advance_to(f.parse(p_ctx));
                f_ctx.advance_to(f.format(a, f_ctx));
            }
        };

        f_ctx.get_arg(p_ctx.get_current_arg_idx()).visit(visitor);
    }
}

}   // end namespace imp {

_SYS_END_NS
