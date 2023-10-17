#include "test_app.h"
#include <string_view_.h>
#include <string_.h>
#include <format_.h>

using namespace sys;

class TestFormat : public TestApp
{
public:

    void TestEasySingleConversions()
    {
        io::stout->out("-- Basic single conversions: Ensuring nothing throws\n");
        // ...and that everything compiles!

        // -- String like things -------------------------------------------

#define STRING_PAYLOAD      "moop"

        io::stout->out("String like things: Expected string payload is '");
        io::stout->out(STRING_PAYLOAD);
        io::stout->out("'\n");

        string_view sv(STRING_PAYLOAD);
        string s(STRING_PAYLOAD);
        const char* sp = STRING_PAYLOAD;
        char cbuf[] = STRING_PAYLOAD;
        char* pchar = cbuf;

        io::stout->out("String like things: Literals and char buffers\n");
        io::stout->out(format(" A string literal:       {}\n", STRING_PAYLOAD));
        io::stout->out(format(" A const char*:          {}\n", sp));
        io::stout->out(format(" A char[]:               {}\n", cbuf));
        io::stout->out(format(" A char*:                {}\n", pchar));

        io::stout->out("String like things: string\n");
        io::stout->out(format(" A const string&:        {}\n", s));
        io::stout->out(format(" A string&&:             {}\n", string(STRING_PAYLOAD)));

        io::stout->out("String like things: string_view\n");
        io::stout->out(format(" A const string_view&:   {}\n", sv));
        io::stout->out(format(" A string_view&&:        {}\n", string_view(STRING_PAYLOAD)));

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

        io::stout->out("\nIntegeral types: const foo&\n");
        io::stout->out(format(" signed char:            {}\n", _schar));
        io::stout->out(format(" unsigned char:          {}\n", _uchar));
        io::stout->out(format(" int:                    {}\n", _int));
        io::stout->out(format(" unsigned int:           {}\n", _uint));
        io::stout->out(format(" short:                  {}\n", _short));
        io::stout->out(format(" unsigned short:         {}\n", _ushort));
        io::stout->out(format(" long:                   {}\n", _long));
        io::stout->out(format(" unsigned long:          {}\n", _ulong));
        io::stout->out(format(" sint128_t:              {}\n", _s128));
        io::stout->out(format(" uint128_t:              {}\n", _u128));

        io::stout->out("Integeral types: foo&&\n");
        io::stout->out(format(" signed char:            {}\n", static_cast<signed char>(42)));
        io::stout->out(format(" unsigned char:          {}\n", static_cast<unsigned char>(42)));
        io::stout->out(format(" int:                    {}\n", static_cast<int>(42)));
        io::stout->out(format(" unsigned int:           {}\n", static_cast<unsigned int>(42)));
        io::stout->out(format(" short:                  {}\n", static_cast<short>(42)));
        io::stout->out(format(" unsigned short:         {}\n", static_cast<unsigned short>(42)));
        io::stout->out(format(" long:                   {}\n", static_cast<long>(42)));
        io::stout->out(format(" unsigned long:          {}\n", static_cast<unsigned long>(42)));
        io::stout->out(format(" sint128_t:              {}\n", static_cast<sint128_t>(42)));
        io::stout->out(format(" uint128_t:              {}\n", static_cast<uint128_t>(42)));

        io::stout->out("Integeral types: Various common convertibles\n");
        io::stout->out(format(" uint8_t:                {}\n", static_cast<uint8_t> (42)));
        io::stout->out(format(" sint8_t:                {}\n", static_cast<sint8_t> (42)));
        io::stout->out(format(" uint16_t:               {}\n", static_cast<uint16_t>(42)));
        io::stout->out(format(" sint16_t:               {}\n", static_cast<sint16_t>(42)));
        io::stout->out(format(" uint32_t:               {}\n", static_cast<uint32_t>(42)));
        io::stout->out(format(" sint32_t:               {}\n", static_cast<sint32_t>(42)));
        io::stout->out(format(" uint64_t:               {}\n", static_cast<uint64_t>(42)));
        io::stout->out(format(" sint64_t:               {}\n", static_cast<sint64_t>(42)));
        io::stout->out(format(" uintptr_t:              {}\n", static_cast<uintptr_t>(42)));
        io::stout->out(format(" sintptr_t:              {}\n", static_cast<sintptr_t>(42)));
        io::stout->out(format(" uint128_t:              {}\n", static_cast<uint128_t>(42)));
        io::stout->out(format(" sint128_t:              {}\n", static_cast<sint128_t>(42)));
        io::stout->out(format(" uintmax_t:              {}\n", static_cast<uintmax_t>(42)));
        io::stout->out(format(" sintmax_t:              {}\n", static_cast<sintmax_t>(42)));
        io::stout->out(format(" time_t:                 {}\n", static_cast<time_t>(42)));

        // -- Boolean things -----------------------------------------------

        constexpr bool bool_val = true;

        io::stout->out("\nBooleans:\n");
        io::stout->out(format(" const bool&:            {}\n", bool_val));
        io::stout->out(format(" bool&& (true):          {}\n", true));
        io::stout->out(format(" bool&& (false):         {}\n", false));

        static_assert(is_integral_v<bool>);

        // -- Pointer things -----------------------------------------------

        constexpr const void* cvp = nullptr;
        constexpr       void*  vp = nullptr;
        constexpr auto         np = nullptr;

        class Foo {} foo;
        Foo* foo_ptr = &foo;

        io::stout->out("\nPointers: const foo&\n");
        io::stout->out(format(" const void*:            {}\n", cvp));
        io::stout->out(format(" void*:                  {}\n", vp));
        io::stout->out(format(" nullptr_t:              {}\n", np));
        io::stout->out(format(" class Foo*:             {}\n", foo_ptr));
        io::stout->out("Pointers: foo&&\n");
        io::stout->out(format(" const void*:            {}\n", static_cast<const void*>(nullptr)));
        io::stout->out(format(" void*:                  {}\n", static_cast<void*>(nullptr)));
        io::stout->out(format(" nullptr_t:              {}\n", nullptr));
        io::stout->out(format(" class Foo*:             {}\n", &foo));
    }

    bool RunTests() override
    {
        try { TestEasySingleConversions(); }
        catch (sys::exception& e) {
            io::stout->out("** Test failed due to exception: ");
            io::stout->out(e.get_msg());
            io::stout->out("\n");
            return false;
        }

        return true;
    }
};

app* CreateApp()
{
    return new TestFormat();
}

