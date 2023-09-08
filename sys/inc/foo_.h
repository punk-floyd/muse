/**
 * @file    foo_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Class foo; a silly test class that can output various events
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_foo__included
#define sys_foo__included

#include <initializer_list_.h>

_SYS_BEGIN_NS

static bool foo_noisy = false;
class Foo {
public:

    Foo() { if (foo_noisy) sys::io::stout->out("Foo\n"); }
    Foo(int a, char c, double d) : _a(a) { if (foo_noisy) sys::io::stout->out("Foo(int,char,double)\n");}
    explicit Foo(int a) : _a(a) { if (foo_noisy) sys::io::stout->out("Foo(int)\n");}

    Foo(sys::initializer_list<int> il)
        { if (foo_noisy) sys::io::stout->out("Foo({...})\n");}
    Foo(sys::initializer_list<int> il, double d)
        { if (foo_noisy) sys::io::stout->out("Foo({...},d)\n");}

    Foo(const Foo& other)     : _a(other._a) { if (foo_noisy) sys::io::stout->out("Foo copy constructor\n"); }
    Foo(Foo&& other) noexcept : _a(other._a) { if (foo_noisy) sys::io::stout->out("Foo move constructor\n"); }
    ~Foo()                    { if (foo_noisy) sys::io::stout->out("~Foo\n"); }
    Foo& operator=(const Foo& other)
    {
        if (foo_noisy) sys::io::stout->out("Foo copy assignment\n");
        _a = other._a;
        return *this;
    }
    Foo& operator=(Foo&& other) noexcept
    {
        if (foo_noisy) sys::io::stout->out("Foo move assignment\n");
        _a = other._a;
        return *this;
    }

    auto operator<=>(const Foo& other) const
    {
        return _a <=> other._a;
    }

    bool operator==(const Foo& other) const
    {
        return _a == other._a;
    }

    int _a{0};
};

_SYS_END_NS

#endif // ifndef sys_MOOMOO__included
