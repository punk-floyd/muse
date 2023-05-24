
set(sys_src
    _init_.cpp
    new.cpp
    mutex_.cpp
    io_file_.cpp
    string_helper.cpp
    error.cpp
)

# Main C++ runtime library
add_library               (sysrt SHARED ${sys_src})
target_compile_features   (sysrt PUBLIC  cxx_std_20)
target_include_directories(sysrt PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(sysrt PRIVATE Threads::Threads)

add_subdirectory(_startup)
