/**
 * @file    _startup.cpp
 * @author  Mike Dekoker (dekoker.mike@gmail.com)
 * @brief   Startup code for all applications
 * @date    2022-11-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <memory_.h>
#include <app.h>

extern sys::app* CreateApp();

namespace sys::nonpublic {
    extern void init_runtime();
}

int main (int argc, char* argv[])
{
    sys::nonpublic::init_runtime();

    // Until we're ready to do something with them
    (void)argc; (void)argv;

    sys::unique_ptr<sys::app> app(CreateApp());
    auto res = app->Run();
    return res;
}
