/**
 * @file    error.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Implements some helper routines for string related classes
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <error_.h>
#include <string_.h>

_SYS_BEGIN_NS

exception::exception(string&& s)
    : _msg(sys::move(s._sbuf))
{
}

_SYS_END_NS
