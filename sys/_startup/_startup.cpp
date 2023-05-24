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
extern void _sys_init_runtime();

int main (int argc, char* argv[])
{
    _sys_init_runtime();

    sys::unique_ptr<sys::app> app(CreateApp());
    auto res = app->Run();
    return res;
}
