/**
 * @file    io_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Top level include for sys::io
 *
 * @copyright Copyright (c) 2022
 *
 */
 #ifndef sys_io__included
 #define sys_io__included

#include <_core_.h>
#include <memory_.h>

#define _SYS_IO_BEGIN_NS    namespace sys::io {
#define _SYS_IO_END_NS      }

_SYS_IO_BEGIN_NS

namespace open_mode {

    /// Open file with read access; file must exist if set
    inline constexpr int read           = 0x01;
    /// Open file for write access
    inline constexpr int write          = 0x02;

    /// Open file for non-blocking IO
    inline constexpr int non_block      = 0x04;
    /// Seek to end of file after opening
    inline constexpr int append         = 0x10;
    /// Truncate exising file
    inline constexpr int trunc          = 0x20;
    /// Create new file if it exists; fails if already exists
    inline constexpr int create_new     = 0x40;
}

class ostream;

//unique_ptr<ostream>  stin;
extern unique_ptr<ostream>  stout;
extern unique_ptr<ostream>  sterr;

_SYS_IO_END_NS

 #endif // ifndef sys_io__included
