/**
 * @file    io_file_.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Declares sys::io::file
 *
 * @copyright Copyright (c) 2022
 *
 */
 #ifndef sys_io_file__included
 #define sys_io_file__included

#include <io_.h>
#include <utility_.h>
#include <string_.h>

_SYS_IO_BEGIN_NS

class file
{
public:

    /// Default construction (no file)
    constexpr file() = default;
    /// Open file from NULL-terminated pathname
    explicit file(const char* pathname, int mode);
    /// Construct and open the specified file
    explicit file(const sys::string& pathname, int mode);
    /// Construct from a string_view; note this allocates a temp string for \0
    template <string_view_like T>
    constexpr explicit file(const T& svl, int mode)
        : _fd(common_construct(sys::string(svl).data(), mode)) { }
    /// Construct from an existing native file handle; we take control
    constexpr explicit file(__file_type fd) : _fd(fd) {}

    /// Returns true if we manage a valid file object
    constexpr bool valid() const noexcept { return fd() != invalid_file; }

    /// Returns file open mode flags (sys::io::open_mode::*)
    int get_open_mode() const noexcept;

    ssize_t read(void* dst, size_t count);
    ssize_t write(const void* src, size_t count);

    /**
     * @brief   Read all count bytes into dst
     *
     * This routine is intended for reading from devices that may break up
     * the data in transit such as a COM port or socket.
     *
     * @param dst   Destination for read data
     * @param count Total number of bytes to read
     *
     * @return Returns total number of bytes read, which will be count if
     *  there were no errors. Check MOOMOO for error details.
     */
    size_t read_all(void* dst, size_t count)
    {
        auto p = reinterpret_cast<uint8_t*>(dst);

        while (count) {
            ssize_t chunk_bytes = read(p, count);
            if ((0 == chunk_bytes) || (chunk_bytes == -1))
                break;

            p     += chunk_bytes;
            count -= chunk_bytes;
        }

        return size_t(p - reinterpret_cast<uint8_t*>(dst));
    }

    size_t write_all(const void* src, size_t count)
    {
        auto p = reinterpret_cast<const uint8_t*>(src);

        while (count) {
            ssize_t chunk_bytes = write(p, count);
            if ((0 == chunk_bytes) || (chunk_bytes == -1))
                break;

            p     += chunk_bytes;
            count -= chunk_bytes;
        }

        return size_t(p - reinterpret_cast<const uint8_t*>(src));
    }

    enum class seek_mode_t : int {
        set,    // Seek to absolute file position
        cur,    // Seek relative to current file position
        end     // Seek relative to end of file
    };
    ssize_t seek(ssize_t offset, seek_mode_t whence = seek_mode_t::set);

    void close();

    // No copying
    file(const file&) = delete;
    file& operator=(const file&) = delete;

    // Moving okay
    constexpr file(file&& other) noexcept
        : _fd(sys::move(other._fd))
    {
        other._fd = invalid_file;     // Invalide other side
    }
    constexpr file& operator=(file&& other) noexcept
    {
        if (this != &other) {
            close();
            _fd = sys::move(other._fd);
            other._fd = invalid_file;

        }
        return *this;
    }

    ~file() { close(); }

    /// Returns true if we manage a valid file object
    constexpr explicit operator bool() const noexcept { return valid(); }

    /// An invalid native file handle
    static constexpr __file_type invalid_file = __invalid_file;

protected:

    /// Returns underlying file handle
    constexpr __file_type fd() const noexcept { return _fd; }

private:

    __file_type common_construct(const char* pathname, int mode);
    /// Returns native file open flags for given open_mode
    int get_open_flags(int open_mode) const noexcept;

    __file_type     _fd{invalid_file};
};

_SYS_IO_END_NS

 #endif // ifndef sys_io_file__included
