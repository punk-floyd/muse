/**
 * @file    _init.cpp
 * @author  Mike Dekoker (dekoker.mike@gmail.com)
 * @brief   General system startup
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <_core_.h>
#include <memory_.h>
#include <io_ofstream_.h>

using namespace sys;

//unique_ptr<io::istream> io::stin;
unique_ptr<io::ostream> io::stout;
unique_ptr<io::ostream> io::sterr;

void _sys_init_runtime()
{
  //io::stin  = sys::move(make_unique<io::ifstream>(0));
    io::stout = sys::move(make_unique<io::ofstream>(1));
    io::sterr = sys::move(make_unique<io::ofstream>(2));
}
