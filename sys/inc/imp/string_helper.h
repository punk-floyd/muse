/**
 * @file    string_helper.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Some helper routines for string related classes
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef sys_imp_string_helper__included
#define sys_imp_string_helper__included

#include <_core_.h>

_SYS_BEGIN_NS

/// Throws error_bounds
void throw_error_bounds(size_t bad_val, size_t max_val = size_t(-1));

_SYS_END_NS

#endif // ifndef sys_imp_string_helper__included
