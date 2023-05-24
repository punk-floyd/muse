# About

Muse is Mike's User Space Environment. It's an experiment in creating a
completely isolated runtime environment developed entirely from scratch
by yours truly. It is written in C++20 using my own implementation of a
C++ runtime, which is not intended to be C++ standard compliant, but is
certainly influenced by it.

I have a couple of goals:
 - First and foremost, have fun nerding out.
 - I harbor the idea of someday running a system that is entirely written
   by me; kernel space, drivers, user space, the whole shebang. This code
   would be part of that.
 - This is a great exercise in exploring the nooks and crannies of the
   modern standard C++ library and its implementation as I use it as a
   guide for my own implementation.
 - This was a great way for me to do a deep dive on template
   metaprogramming, which is something I haven't generally dealt with in my
   day to day work.

My rules:
 - I can only use standard C++ language features. I do not use any C++
   standard library headers.
 - I have no qualms with using compiler intrinsics to implement certain
   things. I flat out have to in some cases.
 - I allow myself to use "possible implementations" from
   [cppreference](https://cppreference.com/) for certain low level type
   traits.
 - I am okay with using a wee bit of the C runtime in the implementation to
   interface with the operating system, which I'm assuming to be UNIX-like.
   This would include things like open/read/write/ioctl/close et al.
 - I will bring in some libraries with the idea that they'll eventually be
   replaced with my own implementation. For example, pthreads is used under
   the hood for mutex and (eventual) thread operations.

My assumptions:
 - I'm building on a GCC that supports C++20 (presently gcc-12)
 - I'm running on a UNIX like OS (presently Debian flavored Linux)

# The state of things (May 2023)

## C++ Runtime (sys)

Generally:
 - I have no plans for custom allocators.
 - Only char-based strings are used. wchar_t can go pound sand. The default
   text encoding will be utf-8.
 - I beleive in constexpr for all of the things.

Headers:
 - [app.h](sys/inc/app.h) - Base class for user space applications.
 - [array_.h](sys/inc/array_.h) - Generic array implementation.
 - [atomic_.h](sys/inc/atomic_.h) - Generic atomic operations.
   Atomic operations for "large" objects are still handled under the hood by
   glibc.
 - [bit_.h](sys/inc/bit_.h) - Implements sys::bit_cast, a constexpr-friendly
   "reinterpret_cast".
 - [char_traits_.h](sys/inc/char_traits_.h) - Generic char traits. It also
   supports some stuff that probably belongs in some kind of "text encoding"
   class.
 - [charconv_.h](sys/inc/charconv_.h) - to_chars and from_chars
   implementations. Presently only supports integral types.
 - [compare_.h](sys/inc/compare_.h) - Support for strong_ordering,
   partial_ordering, and weak ordering. The language itself requires this
   stuff to be defined for any code that makes use of the <=> operator.
 - [error_.h](sys/inc/error_.h) - General error and exception support:
    - sys::error_code - A fledgling error code object.
    - sys::exception and derived buddies defining an exception heirarchy.
 - [format_.h](sys/inc/format_.h) (sys::format and friends) - A mostly-complete
   implementation for a constexpr-friendly, type-safe formatted output akin to
   std::format. Does not yet support floating-point data output and isn't utf-8
   friendly yet.
 - [initializer_list_.h](sys/inc/initializer_list_.h) - Support for
   sys::initializer_list. This is another spot where the compiler assumes
   std::initializer_list to be present.
 - io_*.h - Elementary IO operations. Implemented just enough to give us a
   sys::ostream that we can use for program output.
 - [iterator_.h](sys/inc/iterator_.h) - Preliminary iterator support.
   Implements sys::it_contig for string/string_view/array iterators and a
   few back inserter objects.
 - [limits_.h](sys/inc/limits_.h) - Generic numeric limit support (integral
  only for now).
 - [memory_.h](sys/inc/memory_.h) - Implements sys::unique_ptr. Also contains
   some constexpr friendly memory operations akin to memcopy, memset, memmove,
   etc.
 - [mutex_.h](sys/inc/mutex_.h) (sys::mutex) - Mutex implementation. Presently
   implemented under the hood by pthreads.
 - [new_.h](sys/inc/new_.h) - Implements new and friends; backed by gcc
   intrinsic __builtin_malloc.
 - [shared_string_.h](sys/inc/shared_string_.h) - A shared read-only string
   object. Used by sys::exception and friends so we can have noexcept copy
   constructors.
 - [string_.h](sys/inc/string_.h) (sys::string) - Generic NULL terminated
   string akin to std::string. Supports short string optimization.
 - [string_view_.h](sys/inc/string_view_.h) (sys::string_view) - My stab at
   a std::string_view.
 - [type_list_.h](sys/inc/type_list_.h) (sys::type_list) - A utility class
   for manipulating arbitrary sized template parameter packs.
 - [type_traits_.h](sys/inc/type_traits_.h) - A mostly complete
   implementation of a type traits library based on the standard C++
   version.
 - [types_.h](sys/inc/types_.h) - Basic system types.
 - [utility_.h](sys/inc/utility_.h) - General utility classes:
    - sys::swap and collateral.
    - sys::min_v/sys::max_v: Type-safe min/max support.
 - [variant_.h](sys/inc/variant_.h) (sys::variant) - A mostly complete
   implementation of a constexpr friendly variant class. Not yet supported:
   converting constructors.
