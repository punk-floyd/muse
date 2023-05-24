/**
 * @file    sys_mutex.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Defines sys::mutex
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <mutex_.h>
#include <pthread.h>

#include <stdio.h>

_SYS_BEGIN_NS

/// Static initializer for a standard mutex
const char mutex::static_initializer[__sizeof_native_mutex] = {0};

class mutex_native_handle
{
    sys::mutex& _mux;
public:
    mutex_native_handle(sys::mutex& m) : _mux(m) {}
    operator pthread_mutex_t*() noexcept
        { return reinterpret_cast<pthread_mutex_t*>(&_mux._impl[0]); }
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