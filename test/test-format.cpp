#include "test_app.h"
#include <string_view_.h>
#include <string_.h>
#include <format_.h>
#include <print_.h>

using namespace sys;

class MyClass
{
    int _x{0};
public:
    constexpr explicit MyClass(int x) : _x(x) {}
    constexpr int get() const noexcept { return _x; }
};

template<> struct sys::formatter<MyClass>
{
    bool _not{false};
public:

    constexpr formatter() = default;

    template <class ParseCtx>
    constexpr auto parse(ParseCtx& p_ctx) -> ParseCtx::iterator
    {
        // Dumb format syntax: {} or {!}
        auto it = p_ctx.begin();
        if (*it == '!') {
            _not = true;
            it++;
        }

        if ((it == p_ctx.end()) || (*it != '}'))
            throw error_format("Bad MyClassFormatter replacement field");
        it++;

        return it;
    }

    template <class FormatCtx>
    constexpr auto format(MyClass val, FormatCtx& f_ctx) -> FormatCtx::iterator
    {
        auto it_out = f_ctx.out();

        if (_not) {
            *it_out++ ='!';
            f_ctx.advance_to(it_out);
        }

        sys::formatter<int> f_int;
        return f_int.format(val.get(), f_ctx);
    }
};

class TestFormat : public TestApp
{
public:

    void TestEasySingleConversions()
    {
        using sys::print_str;

        print_str("-- Basic single conversions: Ensuring nothing throws\n");
        // ...and that everything compiles!

        // -- String like things -------------------------------------------

#define STRING_PAYLOAD      "moop"

        print_str("String like things: Expected string payload is '");
        print_str(STRING_PAYLOAD);
        print_str("'\n");

        string_view sv(STRING_PAYLOAD);
        string s(STRING_PAYLOAD);
        const char* sp = STRING_PAYLOAD;
        char cbuf[] = STRING_PAYLOAD;
        char* pchar = cbuf;

        // Note: we could just use sys::print[ln] here but we want to
        //       explicitly use sys::format for the testing.

        print_str("String like things: Literals and char buffers\n");
        print_str(format(" A string literal:       {}\n", STRING_PAYLOAD));
        print_str(format(" A const char*:          {}\n", sp));
        print_str(format(" A char[]:               {}\n", cbuf));
        print_str(format(" A char*:                {}\n", pchar));

        print_str("String like things: string\n");
        print_str(format(" A const string&:        {}\n", s));
        print_str(format(" A string&&:             {}\n", string(STRING_PAYLOAD)));

        print_str("String like things: string_view\n");
        print_str(format(" A const string_view&:   {}\n", sv));
        print_str(format(" A string_view&&:        {}\n", string_view(STRING_PAYLOAD)));

        // -- Integral like things -----------------------------------------

        constexpr signed char       _schar      = 42;
        constexpr unsigned char     _uchar      = 42;
        constexpr int               _int        = 42;
        constexpr unsigned          _uint       = 42;
        constexpr short             _short      = 42;
        constexpr unsigned short    _ushort     = 42;
        constexpr long              _long       = 42;
        constexpr unsigned long     _ulong      = 42;
        constexpr sint128_t         _s128       = 42;
        constexpr uint128_t         _u128       = 42;

        print_str("\nIntegeral types: const foo&\n");
        print_str(format(" signed char:            {}\n", _schar));
        print_str(format(" unsigned char:          {}\n", _uchar));
        print_str(format(" int:                    {}\n", _int));
        print_str(format(" unsigned int:           {}\n", _uint));
        print_str(format(" short:                  {}\n", _short));
        print_str(format(" unsigned short:         {}\n", _ushort));
        print_str(format(" long:                   {}\n", _long));
        print_str(format(" unsigned long:          {}\n", _ulong));
        print_str(format(" sint128_t:              {}\n", _s128));
        print_str(format(" uint128_t:              {}\n", _u128));

        print_str("Integeral types: foo&&\n");
        print_str(format(" signed char:            {}\n", static_cast<signed char>(42)));
        print_str(format(" unsigned char:          {}\n", static_cast<unsigned char>(42)));
        print_str(format(" int:                    {}\n", static_cast<int>(42)));
        print_str(format(" unsigned int:           {}\n", static_cast<unsigned int>(42)));
        print_str(format(" short:                  {}\n", static_cast<short>(42)));
        print_str(format(" unsigned short:         {}\n", static_cast<unsigned short>(42)));
        print_str(format(" long:                   {}\n", static_cast<long>(42)));
        print_str(format(" unsigned long:          {}\n", static_cast<unsigned long>(42)));
        print_str(format(" sint128_t:              {}\n", static_cast<sint128_t>(42)));
        print_str(format(" uint128_t:              {}\n", static_cast<uint128_t>(42)));

        print_str("Integeral types: Various common convertibles\n");
        print_str(format(" uint8_t:                {}\n", static_cast<uint8_t> (42)));
        print_str(format(" sint8_t:                {}\n", static_cast<sint8_t> (42)));
        print_str(format(" uint16_t:               {}\n", static_cast<uint16_t>(42)));
        print_str(format(" sint16_t:               {}\n", static_cast<sint16_t>(42)));
        print_str(format(" uint32_t:               {}\n", static_cast<uint32_t>(42)));
        print_str(format(" sint32_t:               {}\n", static_cast<sint32_t>(42)));
        print_str(format(" uint64_t:               {}\n", static_cast<uint64_t>(42)));
        print_str(format(" sint64_t:               {}\n", static_cast<sint64_t>(42)));
        print_str(format(" uintptr_t:              {}\n", static_cast<uintptr_t>(42)));
        print_str(format(" sintptr_t:              {}\n", static_cast<sintptr_t>(42)));
        print_str(format(" uint128_t:              {}\n", static_cast<uint128_t>(42)));
        print_str(format(" sint128_t:              {}\n", static_cast<sint128_t>(42)));
        print_str(format(" uintmax_t:              {}\n", static_cast<uintmax_t>(42)));
        print_str(format(" sintmax_t:              {}\n", static_cast<sintmax_t>(42)));
        print_str(format(" time_t:                 {}\n", static_cast<time_t>(42)));

        // -- Boolean things -----------------------------------------------

        constexpr bool bool_val = true;

        print_str("\nBooleans:\n");
        print_str(format(" const bool&:            {}\n", bool_val));
        print_str(format(" bool&& (true):          {}\n", true));
        print_str(format(" bool&& (false):         {}\n", false));

        static_assert(is_integral_v<bool>);

        // -- Pointer things -----------------------------------------------

        constexpr const void* cvp = nullptr;
        constexpr       void*  vp = nullptr;
        constexpr auto         np = nullptr;

        class Foo {} foo;
        Foo* foo_ptr = &foo;

        print_str("\nPointers: const foo&\n");
        print_str(format(" const void*:            {}\n", cvp));
        print_str(format(" void*:                  {}\n", vp));
        print_str(format(" nullptr_t:              {}\n", np));
        print_str(format(" class Foo*:             {}\n", foo_ptr));
        print_str("Pointers: foo&&\n");
        print_str(format(" const void*:            {}\n", static_cast<const void*>(nullptr)));
        print_str(format(" void*:                  {}\n", static_cast<void*>(nullptr)));
        print_str(format(" nullptr_t:              {}\n", nullptr));
        print_str(format(" class Foo*:             {}\n", &foo));

        // -- Custom formatter ---------------------------------------------

        MyClass cf(1975);

        print_str("\nCustom formatter\n");
        print_str(format(" MyClass{{}}:              {}\n",   cf));
        print_str(format(" MyClass{{!}}:             {:!}\n", cf));
    }

    void TestFormattedSize()
    {
        sys::println_str("-- Basic formatted_size functionality");

        VerifyThrow(sys::formatted_size("cat")           == 3);
        VerifyThrow(sys::formatted_size("cat{}", 10)     == 5);
        VerifyThrow(sys::formatted_size("cat{}", "dog")  == 6);
        VerifyThrow(sys::formatted_size("cat{:10}", 0)   == 13);
    }

    bool RunTests() override
    {
        try {
            TestEasySingleConversions();
            TestFormattedSize();
        }
        catch (sys::exception& e) {
            // This is a good candidate for sys::print, but since we're testing
            // format here, we'll not use it in the exception handler.
            print_str("** Test failed due to exception: ");
            print_str(e.get_msg());
            print_str("\n");
            return false;
        }

        return true;
    }
};

app* CreateApp()
{
    return new TestFormat();
}
