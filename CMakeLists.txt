cmake_minimum_required(VERSION 3.12)
project(muse                VERSION 1.0
        DESCRIPTION         "Mike's User Space Environment"
        LANGUAGES           CXX)

set(CMAKE_CXX_STANDARD              20)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if (MSVC)
    add_compile_options(/permissive- /W4)
else()
    add_compile_options(
        -Wall -Wextra -Wshadow -Wold-style-cast -Wunused -Wpedantic
        -Wconversion -Wsign-conversion -Wnon-virtual-dtor
    )
    set(CMAKE_CXX_FLAGS_RELEASE -O3)
    set(CMAKE_CXX_FLAGS_DEBUG   -Og)
endif()

# Only do these if this is the main project, and not if it is included through add_subdirectory
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()

endif()

include_directories(SYSTEM sys/inc)

add_subdirectory(sys)
add_subdirectory(test)
add_subdirectory(util)
