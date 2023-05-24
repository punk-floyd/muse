/**
 * @file    string_helper.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Implements some helper routines for string related classes
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <error_.h>
#include <imp/string_helper.h>

_SYS_BEGIN_NS

/// Throws error_bounds
void throw_error_bounds(size_t bad_val, size_t max_val)
{
    // sys::string_view uses this. It can't throw directly because it needs
    // sys::exception which needs sys::string_view which needs
    // sys::exception...

    throw error_bounds_ex(bad_val, max_val);
}

_SYS_END_NS
