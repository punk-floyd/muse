/**
 * @file    io_ofstream_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Defines sys::io::ofstream (output stream to a file)
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef sys_io_ofstream_included
#define sys_io_ofstream_included

#include <io_ostream_.h>
#include <io_file_.h>
#include <string_.h>

namespace sys { class string; class string_view; };

_SYS_IO_BEGIN_NS

class ofstream : public ostream
{
    static constexpr int _fmode = sys::io::open_mode::read;

public:

    // MOOMOO temp:
    constexpr explicit ofstream(native_file_type fd) : _file(fd) {}

    /// Default construction
    constexpr ofstream() { }
    /// Construct from destination file pathname
    explicit ofstream(const sys::string& dst) : _file(dst, _fmode) {}
    /// Construct from destination file pathname
    template <string_view_like T>
    constexpr explicit ofstream(const T& svl) : _file(svl, _fmode) {}

    // -- Implementation

    ~ofstream() override {}
    // No copying
    ofstream(const ofstream&) = delete;
    ofstream& operator=(const ofstream&) = delete;
    // Moving okay
    ofstream(ofstream&&) noexcept = default;
    ofstream& operator=(ofstream&&) noexcept = default;

protected:

    /// Output given buffer to sink
    bool sink(const char_t* data, size_t length) override
        { return _file.write_all(data, length) == length; }

private:

    sys::io::file       _file;
};

_SYS_IO_END_NS

#endif // ifndef sys_io_ofstream_included
