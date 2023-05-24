/**
 * @file    initializer_list_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   sys::initializer_list
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_initializer_list__included
#define sys_initializer_list__included

#include <_core_.h>

// std::initializer_list another class that is baked into the language.
// I can't define the class in sys and alias in std like I can with
// strong_ordering and friends, so I'll to the other way and define in
// std and alias in sys.
namespace std {

template <class T>
class initializer_list
{
public:

    using value_type = T;
    using size_type  = sys::size_t;

    using reference        = const value_type&;
    using const_reference  = const value_type&;

    using iterator       = const value_type*;
    using const_iterator = const value_type*;

    constexpr initializer_list() noexcept = default;

    /// Returns the number of elements in the initializer list
    constexpr size_type size() const noexcept { return _len; }

    constexpr iterator begin() const noexcept { return _arr; }
    constexpr iterator end()   const noexcept { return _arr + _len; }

private:

    // Constructed by the compiler
    constexpr initializer_list(iterator arr, size_type len)
        : _arr(arr), _len(len)
    {}

    iterator    _arr{nullptr};
    size_type   _len{0};
};

}

_SYS_BEGIN_NS
    template <class T> using initializer_list = std::initializer_list<T>;

    template <class T>
    const T* begin(initializer_list<T> il) noexcept
    {
        return il.begin();
    }

    template <class T>
    const T* end(initializer_list<T> il) noexcept
    {
        return il.end();
    }

_SYS_END_NS

#endif // ifndef sys_initializer_list__included
