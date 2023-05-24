/**
 * @file    app.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   sys::app : Base class for applications
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_app__included
#define sys_app__included

#include <io_.h>
#include <io_ostream_.h>

namespace sys {

namespace io { class ostream; }

class app
{
public:

    app() = default;

    // No copying or moving
    app(const app& that) = delete;
    app& operator=(const app& that) = delete;
    app(app&& that) = delete;
    app& operator=(app&& that) = delete;

    virtual ~app() = default;

    virtual int Run() = 0;

protected:

  //constexpr const auto& stin()  const noexcept { return sys::io::stin; }
    constexpr const auto& stout() const noexcept { return sys::io::stout; }
    constexpr const auto& sterr() const noexcept { return sys::io::sterr; }
};

}   // End namespace sys

#endif // ifndef sys_app__included
