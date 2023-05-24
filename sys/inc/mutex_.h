/**
 * @file    mutex_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Declares sys::mutex
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_mutex__included
#define sys_mutex__included

#include <_core_.h>
#include <memory_.h>

_SYS_BEGIN_NS

/// Provides basic mutual exclusion facility; non recursive
class mutex
{
public:

    // -- Construction

    constexpr mutex() noexcept
    {
        sys::mem::copy_bytes(_impl, static_initializer, __sizeof_native_mutex);
    }

    // No copying or moving
    mutex(const mutex&) = delete;
    mutex(mutex&&) = delete;

    /// Locks the mutex, blocks if the mutex is not available
    void lock();
    /// Tries to lock the mutex, returns if the mutex is not available
    bool try_lock();
    /// Unlocks the mutex
    void unlock();

    // -- Implementation

    ~mutex() noexcept;

    // No copying or moving
    mutex& operator=(const mutex&) = delete;
    mutex& operator=(mutex&&) = delete;

private:

    // Only my friends can touch my privates
    friend class mutex_native_handle;

    /// Initializer for _impl
    static const char static_initializer[__sizeof_native_mutex];

    /// Opaque native mutex type
    using mux_impl_t = char[__sizeof_native_mutex];

    mux_impl_t  _impl;
};

template <class Mutex>
class lock_guard
{
public:

    using mutex_type = Mutex;

    explicit lock_guard(mutex_type& mux)
        : _mux(mux)
    {
        _mux.lock();
    }

    // No copying
    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

    ~lock_guard()
    {
        _mux.unlock();
    }

private:

    mutex_type&     _mux;
};

_SYS_END_NS

#endif // ifndef sys_mutex__included
