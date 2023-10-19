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
#include <print_.h>

_SYS_BEGIN_NS

class Foo {
public:

    // If this is true then special functions emit to standard output
    static bool enable_output; // == false by default

    Foo()                                           { emit("Foo()"); }
    Foo(int a, char c, double d) : _a(a)            { emit("Foo(int,char,double)");}
    explicit Foo(int a) : _a(a)                     { emit("Foo(int)");}

    Foo(sys::initializer_list<int> il)
    {
        emit("Foo({{...}})");
        for (auto v : il) _a = v;
    }
    Foo(sys::initializer_list<int> il, double d)
    {
        emit("Foo({{...}},d)");
        for (auto v : il) _a = v;
    }

    Foo(const Foo& other)     : _a(other._a)        { emit("Foo(const Foo&)"); }
    Foo(Foo&& other) noexcept : _a(other._a)        { emit("Foo(Foo&&)"); }
    ~Foo()                                          { emit("~Foo"); }
    Foo& operator=(const Foo& other)
    {
        emit("Foo& operator=(const Foo&)");
        _a = other._a;
        return *this;
    }
    Foo& operator=(Foo&& other) noexcept
    {
        emit("Foo& operator=(Foo&&)");
        _a = other._a;
        return *this;
    }

    auto operator<=>(const Foo& other) const = default;
    bool operator==(const Foo& other) const = default;

    int get_a() const noexcept { return _a; }

private:

    template <class... Args>
    void emit(sys::format_string<Args...> fmt, Args&&... args)
    {
        if (enable_output)
            sys::println(fmt, sys::forward<Args>(args)...);
    }

    int _a{0};
};

_SYS_END_NS

#endif // ifndef sys_foo__included
