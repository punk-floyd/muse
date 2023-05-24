#include <app.h>

#include <type_traits_.h>
#include <format_.h>
#include <error_.h>

/*
    Strategy:

    Concrete function:
        e.g., sys::string format("Hello {}...", s...);

    If we have some template object that takes literal fmt and pack of types
    then we can check several things at compile time:
        - number of arguments
        - that each type is a valid type
        - that the format string for that type is valid
    This object should provide an interface for where the data is sent:
        string, ostream, ???
*/

#include <initializer_list_.h>

class Foo {
public:

    Foo() { sys::io::stout->out("Foo\n"); }
    Foo(int a, char c, double d) {sys::io::stout->out("Foo(...)\n");}

    Foo(sys::initializer_list<int> il)
        {sys::io::stout->out("Foo({...})\n");}
    Foo(sys::initializer_list<int> il, double d)
        {sys::io::stout->out("Foo({...},d)\n");}

    Foo(const Foo& other)     { sys::io::stout->out("Foo copy constructor\n"); }
    Foo(Foo&& other) noexcept { sys::io::stout->out("Foo move constructor\n"); }
    ~Foo()                    { sys::io::stout->out("~Foo\n"); }
    Foo& operator=(const Foo& other)
    {
        sys::io::stout->out("Foo copy assignment\n");
        return *this;
    }
    Foo& operator=(Foo&& other) noexcept
    {
        sys::io::stout->out("Foo move assignment\n");
        return *this;
    }

    sys::weak_ordering operator<=>(const Foo&) const
    {
        return sys::weak_ordering::equivalent;
    }
};

class True : public sys::app
{
public:

    int Run() override
    {
#if 1
        int a{0x12345678};
        double d{};
        sys::string s{};
        //Foo foo{};

        try {
            auto fs = sys::format("What say you? {}\n", 100);
            stout()->out(fs);
        }
        catch(sys::exception& e) {
            stout()->out("Exception: ");
            stout()->out(e.get_msg());
            stout()->out("\n");
        }
#endif
        return 0;
    }
};

sys::app* CreateApp()
{
    return new True();
}
