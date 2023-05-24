#include "test_app.h"
#include <charconv_.h>

using namespace sys;

class TestCharConv : public TestApp
{
public:

    /// Convert given value to a string, then back to a value and compare
    template <integral I, unsigned Radix>
    constexpr static bool round_trip(const I& val)
    {
        // Large enough to hold longest representation (i.e., binary)
        constexpr size_t cbuf_size = numeric_limits<I>::bits + 1; // +1 for possible sign
        char cbuf[cbuf_size] = {0};

        auto do_to_chars = [&cbuf](const I& val, unsigned radix) {
            auto&& [ptr,ec] = to_chars(cbuf, cbuf+sizeof(cbuf), val, radix);
            return is_error(ec) ? string_view{} : string_view(cbuf, ptr - cbuf);
        };

        auto do_from_chars = [](I& value, string_view sv, unsigned radix) {
            auto&& [pos, ec] = from_chars(value, sv, radix);
            return !is_error(ec);
        };

        // Number to string
        string_view sv{do_to_chars(val, Radix)};
        if (sv.is_empty())
            return false;       // Failed to convert

        // String to number
        I val_redux{};
        if (!do_from_chars(val_redux, sv, Radix))
            return false;

        return val == val_redux;
    }

    template <unsigned Radix, class T, class... Ts>
    constexpr void test_round_trips()
    {
        if constexpr (Radix <= 36) {
            // Test the min, max, and 0 values for the current radix and type
            static_assert(round_trip<T, Radix>(numeric_limits<T>::min));
            static_assert(round_trip<T, Radix>(T{0}));
            static_assert(round_trip<T, Radix>(numeric_limits<T>::max));

            // Test the remaining types for this radix
            if constexpr (sizeof...(Ts) > 0)
                test_round_trips<Radix, Ts...>();

            // Test the next radix
            test_round_trips<Radix + 1, T, Ts...>();
        }
    }

    bool RunTests() override
    {
        // Note: All tests are constexpr here, so if it compiles, then it
        //       passes!

        // Starting at radix 2, test all of the specified types. The test
        // is to convert min/max/0 values for the current radix and type
        // into a string and then back to a value which shoulc equal the
        // original value.
        test_round_trips<2,     // Start at radix 2; test will go up to radix 36
            unsigned short, unsigned int, unsigned long, unsigned long long, unsigned __int128,
                     short,          int,          long,          long long,          __int128,
            char32_t, char16_t, char8_t, wchar_t, char, signed char, unsigned char
        >();

        // TODO : Probably want to verify some failures here, too.

        return true;
    }
};

sys::app* CreateApp()
{
    return new TestCharConv();
}
