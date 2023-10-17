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
#include <mutex_.h>
#include <io_ofstream_.h>

using namespace sys;

//unique_ptr<io::istream> io::stin;
unique_ptr<io::ostream> io::stout;
unique_ptr<io::ostream> io::sterr;

static void init_runtime_work()
{
    //io::stin  = make_unique<io::ifstream>(0);
    io::stout = make_unique<io::ofstream>(1);
    io::sterr = make_unique<io::ofstream>(2);
}

namespace sys::nonpublic {
    void init_runtime()
    {
        // Only main should be calling this, but just in case...
        static once_flag of;
        call_once(of, init_runtime_work);
    }
}
