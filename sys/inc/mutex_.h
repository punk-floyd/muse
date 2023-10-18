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
#include <atomic_.h>
#include <functional_.h>
#include <array_.h>

_SYS_BEGIN_NS

/// Provides basic mutual exclusion facility; non recursive
class mutex
{
public:

    // -- Construction

    constexpr mutex() noexcept
    {
        _impl.fill(0);
    }

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
    mutex(const mutex&) = delete;
    mutex(mutex&&) = delete;

private:

    // Only my friends can touch my privates
    friend class mutex_native_handle;

    /// Opaque native mutex type
    sys::array<char, nonpublic::sizeof_native_mutex> _impl;
};

template <class Mutex>
class lock_guard
{
public:

    using mutex_type = Mutex;

    explicit lock_guard(mutex_type& mux)
        : _mux(mux)
    {
        _mux.get().lock();
    }

    // No copying; moving okay
    lock_guard(const lock_guard&)                = delete;
    lock_guard& operator=(const lock_guard&)     = delete;
    lock_guard(lock_guard&&) noexcept            = default;
    lock_guard& operator=(lock_guard&&) noexcept = default;

    ~lock_guard()
    {
        _mux.get().unlock();
    }

private:

    sys::ref_wrap<mutex_type>     _mux;
};

/// Helper object to ensure that call_once invokes the function only once
class once_flag
{
public:

    constexpr once_flag() noexcept = default;

private:

    // Only call_once<> can fiddle with our private parts
    template <class Callable, class... Args>
        requires is_invocable_v<Callable, Args...>
    friend void call_once(once_flag& flag, Callable&& f, Args&&... args)
        noexcept(is_nothrow_invocable_v<Callable, Args...>);

    /// Returns true the first time this called or false otherwise
    bool try_once() noexcept { return _flag.exchange(1) == 0; }

    /** @brief Reset flag back to initial state
     *
     * This routine should only be called when a call_once invocation
     * throws an exception.
    */
    void reset_from_exception() noexcept
    {
        _flag.store(0);
    }

    sys::atomic<int>    _flag{0};
};

/// Invokes a callable only once even if called from multiple threads
template <class Callable, class... Args>
    requires is_invocable_v<Callable, Args...>
inline void call_once(once_flag& flag, Callable&& f, Args&&... args)
    noexcept(is_nothrow_invocable_v<Callable, Args...>)
{
    if constexpr (is_nothrow_invocable_v<Callable, Args...>) {
        if (flag.try_once())
            sys::invoke(sys::forward<Callable>(f), sys::forward<Args>(args)...);
    }
    else {
        if (flag.try_once()) {
            try { sys::invoke(sys::forward<Callable>(f), sys::forward<Args>(args)...); }
            catch (...) {
                // The callable did not successfully execute. Reset the
                // once_flag so someone else can make an attempt. This is a
                // safe operation because we have the "lock" here.
                flag.reset_from_exception();
                throw;
            }
        }
    }
}

_SYS_END_NS

#endif // ifndef sys_mutex__included
