/**
 * @file    sys_io_file.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Defines sys::io::file
 *
 * @copyright Copyright (c) 2022
 *
 */
 #include <io_file_.h>
 #include <string_.h>

#define _LARGEFILE64_SOURCE     1 // enables lseek64
#include <unistd.h>
#include <fcntl.h>

 using namespace sys::io;

/// Open file from NULL-terminated pathname
file::file(const char* pathname, int mode)
    : _fd(common_construct(pathname, mode))
{
    if (!valid())
        throw 1;    // MOOMOO
}

/// Construct and open the specified file
file::file(const sys::string& pathname, int mode)
    : _fd(common_construct(pathname.data(), mode))
{
    if (!valid())
        throw 1;    // MOOMOO
}

sys::native_file_type file::common_construct(const char* pathname, int mode)
{
    // MOOMOO:  Need a routine to take a syscall result
    // and set whatever our errno will be.
    int fd = ::open(pathname, get_open_flags(mode));
    if (fd < 0)
        return invalid_file;

    if (mode & open_mode::append)
        lseek64(fd, 0, SEEK_END);

    return fd;
}

/// Returns native file open flags for given open_mode
int file::get_open_flags(int mode) const noexcept
{
    int flags = 0;

    if ((mode & open_mode::read) && (mode & open_mode::write))
        flags |= O_RDWR;
    else if (mode & open_mode::write)
        flags |= O_WRONLY | O_CREAT;
    else if (mode & open_mode::write)
        flags |= O_RDONLY;

    if (mode & open_mode::non_block)
        flags |= O_NONBLOCK;
    // open_mode::append does not correspond to an O_* flag; it is not
    // O_APPEND: O_APPEND appends *all* writes, our append just jumps
    // to the end of file after opening, like std::ate.
    if (mode & open_mode::trunc)
        flags |= O_TRUNC;
    if (mode & open_mode::create_new)
        flags |= O_CREAT | O_EXCL;

    return flags;
}

void file::close()
{
    if (invalid_file != fd()) {
        // Don't close stdin(0), stout(1), or stedrr(2)
        if (_fd > 2)
            ::close(_fd);
        _fd = invalid_file;
    }
}

/// Returns file open mode flags (sys::io::open_mode::*)
int file::get_open_mode() const noexcept
{
    int open_mode = 0;

    if (valid()) {
        int flags = fcntl(fd(), F_GETFL);
        switch (auto acc = (flags & O_ACCMODE)) {
            case O_RDWR:
            case O_RDONLY: open_mode |= io::open_mode::read;
            if (acc == O_RDONLY) break;
            [[fallthrough]];
            case O_WRONLY: open_mode |= io::open_mode::read; break;
        }
        if (flags & O_NONBLOCK)
            open_mode |= sys::io::open_mode::non_block;
        if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
            open_mode |= sys::io::open_mode::create_new;
        if (flags & O_TRUNC)
            open_mode |= sys::io::open_mode::trunc;
    }

    return open_mode;
}

ssize_t file::read(void* dst, size_t count)
{
    return ::read(fd(), dst, count);
}

ssize_t file::write(const void* src, size_t count)
{
    return ::write(fd(), src, count);
}

ssize_t file::seek(ssize_t offset, seek_mode_t whence)
{
    return ::lseek64(fd(), offset, static_cast<int>(whence));
}
