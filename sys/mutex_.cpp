/**
 * @file    sys_mutex.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Defines sys::mutex
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <mutex_.h>
#include <bit_.h>

#include <pthread.h>
#include <stdio.h>

_SYS_BEGIN_NS

class mutex_native_handle
{
    sys::mutex& _mux;
public:
    constexpr mutex_native_handle(sys::mutex& m) : _mux(m) {}
    constexpr operator pthread_mutex_t*() noexcept
        { return sys::bit_cast<pthread_mutex_t*>(_mux._impl.data()); }
};

mutex::~mutex() noexcept
{
    pthread_mutex_destroy(mutex_native_handle(*this));
}

/// Locks the mutex, blocks if the mutex is not available
void mutex::lock()
{
    pthread_mutex_lock(mutex_native_handle(*this));
}

/// Tries to lock the mutex, returns if the mutex is not available
bool mutex::try_lock()
{
    return 0 == pthread_mutex_trylock(mutex_native_handle(*this));
}

/// Unlocks the mutex
void mutex::unlock()
{
    pthread_mutex_unlock(mutex_native_handle(*this));
}

_SYS_END_NS