/**
 * @file    fmt_parse.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Top-level parsing for format et al.
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_fmt_parse_included
#define sys_imp_fmt_parse_included

#include <_core_.h>
#include <imp/fmt_args.h>

_SYS_BEGIN_NS

class parse_context_base
{
public:

    /// Format string argument ID mode
    enum class argid_mode_t {
        Undefined,      // We don't know yet; just starting out
        Auto,           // Automatic indexing   {:...}
        Manual          // Manual indexing      {#:...}
    };

    // -- Construction

    constexpr parse_context_base(basic_format_args& args) noexcept
        : _fmt_args(args)
    {}

    // -- Attributes

    constexpr argid_mode_t get_argid_mode() const noexcept { return _argid_mode; }
    constexpr void set_argid_mode(argid_mode_t argid_mode)
    {
        if (argid_mode != get_argid_mode()) {
            if (argid_mode_t::Undefined != get_argid_mode())
                throw error_format("Cannot mix auto and manual arg-ids");
            _argid_mode = argid_mode;
        }
    }

    /** @brief Return next index for auto arg-id mode
     *
     * This function implicitly sets Auto mode and will throw if manual
     * mode has already been set.
     *
     * This function doesn't validate that the returned argument index is
     * valid.
    */
    constexpr size_t get_next_auto_arg_idx()
    {
        if (get_argid_mode() != argid_mode_t::Auto)
            set_argid_mode(argid_mode_t::Auto);

        return _arg_idx++;
    }

    constexpr const basic_format_args& get_format_args() const noexcept
        { return _fmt_args; }
    constexpr basic_format_args& get_format_args() noexcept
        { return _fmt_args; }

    /// Sink for formatted output
    virtual void format_out(char c) = 0;
    /// Sink for formatted output
    virtual void format_out(string_view v)
    {
        for (auto c : v)
            format_out(c);
    }

private:

    argid_mode_t        _argid_mode{argid_mode_t::Undefined};
    size_t              _arg_idx{0};     ///< Index for auto indexing
    basic_format_args&  _fmt_args;
};

namespace imp {

template <class OutputIt>
class parse_context final : public parse_context_base
{
public:

    constexpr parse_context(basic_format_args& args, OutputIt& out) noexcept
        : parse_context_base(args), _out(out)
    {}

    constexpr OutputIt& get_output_iterator() noexcept { return _out; }

    /// Sink for formatted output
    constexpr void format_out(char c) override
        { *get_output_iterator() = c; }

private:

    OutputIt&           _out;
};

template <class OutputIt>
struct format_sink_imp : public ::sys::formatter_sink
{
    constexpr  format_sink_imp(OutputIt& it) : _it(it) {}
    constexpr ~format_sink_imp() = default;

    constexpr void output(const char& c) override
        { *_it = c; }

private:
    OutputIt&   _it;
};

/** @brief  Find the bounds of the next replacement field
 *
 * This function does respect nested replacement fields to any depth.
 *
 * @param sv   The view to check
 * @param end  If this parameter is not nullptr and a replacement field was
 *  found, the iterator pointed to by this parameter will be set to point to
 *  the character after the teminal '}' (which may be sv.end()).
 *
 * @return     Returns an iterator pointing to the opening '{' of the next
 *  replacement field.
 */
constexpr string_view::iterator_type find_next_rf(string_view sv,
    string_view::iterator_type* end)
{
    // Find next rf start, ignoring 'escaped' "{{"
    string_view::size_type b = 0;
    while (1) {
        b = sv.find_first('{', b);
        if (b == string_view::npos)
            return sv.end();   // No more replacement fields
        if ((b+1 == sv.length()) || (sv[b+1] != '{'))
            break;      // Not escaped ("{{"); here's our guy
        b += 2;         // Jump over escaped char
    }

    // Find the end of the replacement field. Replacement field can contain
    // nested replacement fields. This implementation assumes arbitrary
    // nested replacement field depth. Also, brackets cannot be escaped
    // within a replacement field.
    size_t endp, depth = 1;
    for (endp = b + 1; endp < sv.length(); ++endp) {
        // Is this a closing bracket }
        if ((sv[endp] == '}') && (0 == --depth)) {
            endp++; // Advance to one beyond
            break;
        }
        // Is this the start of a nested replacement field?
        if (sv[endp] == '{')
            ++depth;
    }
    if (depth)
        throw error_format("Missing replacement field terminator");

    if (end)
        *end = sv.begin() + endp;
    return sv.begin() + b;
}

constexpr void check_format_string_no_args(string_view fmt)
{
    if (find_next_rf(fmt, nullptr) != fmt.end())
        throw error_format("Missing argument");
}

template <bool Generate = true, class OutputIt>
constexpr void format_parse_rf(string_view rf, parse_context<OutputIt>& pctx)
{
    size_t arg_id = size_t{-1};

    // Valid possibilities here: {} {arg-id} {arg-id:fmt-spec} {:fmt-spec}
    // Note: rf contains everything between the {}

    // Trim leading whitespace: {   arg-id:fmt-spec}
    rf.trim(true, false);

    // Handle the optional arg-id. If the firt char is not a ':' then it
    // must be the start of an arg-id.
    if (!rf.is_empty() && (rf.front() != ':')) {
        // Convert and remove arg-id (and any trailing whitespace). We will
        // validate index semantics down below.
        auto&& [pos, ec] = from_chars(arg_id, rf);
        if (is_error(ec))
            throw error_format("Invalid arg-id");
        rf.remove_prefix(pos).trim(true, false);

        // Set manual argid mode; throws if we're already set to auto
        pctx.set_argid_mode(parse_context<OutputIt>::argid_mode_t::Manual);
    }
    // If anything remains, then it's a format specification that must be
    // preceded by a ':'
    if (!rf.is_empty()) {
        if (rf.front() != ':')
            throw error_format("Missing ':'");
        // Eat the delineator; what remains is the format specification. We
        // don't trim here because the whitespace may be relevant to the
        // formatter.
        rf.remove_prefix(1);
    }

    if (arg_id == size_t{-1})
        arg_id = pctx.get_next_auto_arg_idx();

    // Validate argument index
    auto& fmt_args = pctx.get_format_args();
    if (arg_id > fmt_args.get_arg_count())
        throw error_format("Bad argument index");

    // Pull in the concrete formatter object for argument
    auto& my_formatter = fmt_args.get_formatter(arg_id);

    // Now have the formatter parse the format specification
    my_formatter.parse(rf, pctx);

    if constexpr (Generate) {

        imp::format_sink_imp sink(pctx.get_output_iterator());
        auto fmt_arg = pctx.get_format_args().get_arg(arg_id);

        format_context fmt_ctx(sink, fmt_args);
        my_formatter.format(fmt_arg, fmt_ctx);
    }
}

template <bool Generate = true, class OutputIt>
    // requires TODO (and above)
constexpr void format_parse(string_view fmt, parse_context<OutputIt>& pctx)
{
    constexpr auto npos = string_view::npos;

    while(!fmt.is_empty()) {

        // Find the next replacement field
        string_view::iterator_type rf_end = fmt.end();
        auto p = imp::find_next_rf(fmt, &rf_end);

        // Consume all chars up to the replacement field
        auto out = pctx.get_output_iterator();
        for (auto i = fmt.begin(); i != p; *out = *i++);

        // If we don't have a replacement field then we're done
        if (p == fmt.end())
            break;

        // Get a view of the replacement field: everything between {}
        string_view rf(&p[1], rf_end - (p + 2));

        // Process the replacement field
        format_parse_rf<Generate>(rf, pctx);

        // Jump over the replacement field
        fmt.remove_prefix(rf_end - fmt.begin());
    }
}

/// Check the format string itself; this is run at compile time
template <class... FmtArgs>
constexpr void check_format_string(string_view fmt)
{
    // Construct format args, but with no args for constexpr
    format_arg_store<FmtArgs...> arg_store{};
    basic_format_args fmt_args(arg_store);
    // Dummy output iterator that just sinks data
    null_insert_iterator<char> black_hole;

    imp::parse_context pctx(fmt_args, black_hole);
    format_parse<false>(fmt, pctx);
}

} // namespace imp

_SYS_END_NS

#endif // ifndef sys_imp_fmt_parse_included
