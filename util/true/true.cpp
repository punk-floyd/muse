#include <app.h>

#include <type_traits_.h>
#include <format_.h>
#include <error_.h>

#include <initializer_list_.h>
class Foo {
public:

    Foo() { sys::io::stout->out("Foo\n"); }
    Foo(int a, char c, double d) {sys::io::stout->out("Foo(int,char,double)\n");}

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

#include <vector_.h>

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
        //sys::vector<Foo> v;
        //v.push_back(Foo{});
        //v.push_back(Foo{});
        //v.emplace(100, 'C', 123.456);

        sys::vector<int> vi;
        vi.push_back(1);
        vi.push_back(2);
        stout()->out("Moop\n");

        for (const auto& it : vi)
            stout()->out(sys::format("{} ", it));
        stout()->out("\n");


#if 0
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
#endif
        return 0;
    }
};

sys::app* CreateApp()
{
    return new True();
}
