/**
 * @file    iterator_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Iterator interface
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_iterator__included
#define sys_iterator__included

#include <_core_.h>
#include <compare_.h>
#include <functional_.h>

_SYS_BEGIN_NS

// -- Iterator categories

struct tag_iterator_output {};
struct tag_iterator_input {};
struct tag_iterator_forward : public tag_iterator_input {};
struct tag_iterator_bidir   : public tag_iterator_forward {};
struct tag_iterator_random  : public tag_iterator_bidir {};
struct tag_iterator_contig  : public tag_iterator_random {};

template <class T>
struct it_contig
{
    using iterator_category = tag_iterator_contig;
    using value_type        = T;
    using difference_type   = sys::ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;

    constexpr explicit it_contig(pointer ptr) noexcept : _cur(ptr) {}

    constexpr reference operator*() noexcept                // *it
        { return *_cur; }
    constexpr const reference operator*() const noexcept    // *it
        { return *_cur; }

    constexpr pointer operator->() noexcept                 // it->
        { return _cur; }
    constexpr const pointer operator->() const noexcept     // it->
        { return _cur; }

    constexpr it_contig& operator++() noexcept              // ++it
        { ++_cur; return *this; }
    constexpr it_contig  operator++(int) noexcept           // it++
    {
        auto prev = *this;
        ++_cur;
        return prev;
    }

    constexpr it_contig& operator--() noexcept              // --it
        { --_cur; return *this; }
    constexpr it_contig  operator--(int) noexcept           // it--
    {
        auto prev = *this;
        --_cur;
        return prev;
    }

    // Comparison
    constexpr auto operator<=>(const it_contig& rhs) const noexcept
    {
        return _cur <=> rhs._cur;
    }
    constexpr bool operator==(const it_contig& rhs) const noexcept = default;

    constexpr reference operator[](size_t n) noexcept       // it[n]
        { return _cur[n]; }
    constexpr const reference operator[](size_t n) const noexcept
        { return _cur[n]; }

    constexpr it_contig& operator+=(difference_type n)      // it += n
        { _cur += n; return *this; }
    constexpr it_contig& operator-=(difference_type n)      // it -= n
        { _cur -= n; return *this; }

    friend constexpr it_contig operator+ (it_contig it, difference_type n)  // it + n
        { return it += n; }
    friend constexpr it_contig operator+ (difference_type n, it_contig it)  // n + it
        { return it += n; }

    friend constexpr it_contig operator- (it_contig it, difference_type n)  // it - n
        { return it -= n; }
    friend constexpr it_contig operator- (difference_type n, it_contig it)  // n - it
        { return n - it._cur; }
    friend constexpr difference_type operator- (it_contig a, it_contig b)   // it - it2
        { return a._cur - b._cur; }

    /// Conversion to a const iterator
    constexpr operator it_contig<const T>() const noexcept
        requires (!is_const_v<T>)
    { return it_contig<const T>{_cur}; }

private:

    pointer         _cur;
};

/// Output iterator that appends elements into a container
template <class T>
struct back_insert_iterator
{
    using iterator_category = tag_iterator_output;
    using value_type        = void;
    using difference_type   = sys::ptrdiff_t;
    using pointer           = void;
    using reference         = void;
    using container_type    = T;

    constexpr explicit back_insert_iterator(T& container) noexcept
        : _cont(container)
    {}

    constexpr back_insert_iterator& operator=(const typename T::value_type& value)
        { _cont.get().push_back(value); return *this; }
    constexpr back_insert_iterator& operator=(typename T::value_type&& value)
        { _cont.get().push_back(sys::move(value)); return *this; }

    constexpr back_insert_iterator& operator*() noexcept
        { return *this; }
    constexpr back_insert_iterator& operator++() noexcept
        { return *this; }
    constexpr back_insert_iterator operator++(int) noexcept
        { return *this; }

protected:

    ref_wrap<container_type>    _cont;
};

/// Output iterator that does nothing with elements
template <class T>
struct null_insert_iterator
{
    using iterator_category = tag_iterator_output;
    using value_type        = void;
    using difference_type   = sys::ptrdiff_t;
    using pointer           = void;
    using reference         = void;
    using container_type    = void;

    constexpr explicit null_insert_iterator() noexcept = default;

    constexpr null_insert_iterator& operator=(const T&) noexcept { return *this; }
    constexpr null_insert_iterator& operator=(T&&)      noexcept { return *this; }
    constexpr null_insert_iterator& operator*()         noexcept { return *this; }
    constexpr null_insert_iterator& operator++()        noexcept { return *this; }
    constexpr null_insert_iterator  operator++(int)     noexcept { return *this; }
};

/// Output iterator that only counts elements
template <class T>
struct count_insert_iterator
{
    using iterator_category = tag_iterator_output;
    using value_type        = void;
    using difference_type   = sys::ptrdiff_t;
    using pointer           = void;
    using reference         = void;
    using container_type    = void;

    constexpr explicit count_insert_iterator() noexcept = default;

    constexpr count_insert_iterator& operator=(const T&) noexcept { _count++; return *this; }
    constexpr count_insert_iterator& operator=(T&&)      noexcept { _count++; return *this; }
    constexpr count_insert_iterator& operator*()         noexcept { return *this; }
    constexpr count_insert_iterator& operator++()        noexcept { return *this; }
    constexpr count_insert_iterator& operator++(int)     noexcept { return *this; }

    constexpr size_t get_count() const noexcept { return _count; }

private:

    size_t  _count{0};
};

template <class It>
auto distance(It first, It last) -> It::difference_type
{
    if constexpr (sys::is_same_v<typename It::iterator_category, tag_iterator_contig>)
        return last - first;
    else {
        typename It::difference_type d{0};
        while (first != last) {
            first++;
            d++;
        }
        return d;
    }
}

_SYS_END_NS

#endif // ifndef sys_iterator__included
