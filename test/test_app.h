#include <app.h>
#include <error_.h>
#include <string_view_.h>
#include <print_.h>

class TestApp : public sys::app
{
public:

    TestApp() = default;

    class test_failure : public sys::error_logic
    {
    public:
        test_failure() = default;

        template <sys::string_view_convertible T>
        explicit test_failure (T svl) : sys::error_logic(svl) {}
    };

    constexpr unsigned get_error_count() const noexcept
        { return m_error_count; }

    // TODO : Update Verify* to take source info (file/line)

    inline bool Verify(bool b, sys::string_view msg = "")
    {
        if (!b)
            sys::println(" Test failure ({}): {}", ++m_error_count, msg);

        return b;
    }

    inline void VerifyThrow(bool b, sys::string_view msg = "")
    {
        if (!b) {
            ++m_error_count;
            throw test_failure{msg};
        }
    }

    virtual bool RunTests() = 0;

    int Run() override
    {
        bool test_passed = true;        // Optimism

        try {
            test_passed = RunTests() && (0 == get_error_count());
        }
        catch (test_failure& e) {
            stout()->out("Fundamental test failed. Aborting\n");
            test_passed  = false;       // Disappoinment
        }
        catch (sys::exception& e) {
            stout()->out("Exception: ");
            stout()->out(e.get_msg());
            stout()->out("\n");
            return -1;
        }
        catch (...) {
            stout()->out("Some non sys exception occurred.\n");
            return -2;
        }

        if (test_passed)
            stout()->out("Testing complete: All tests passed\n");
        else
            stout()->out("Testing complete: One or more tests failed\n");

        return 0;
    }

private:

    unsigned        m_error_count{0};
};
