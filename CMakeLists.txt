cmake_minimum_required(VERSION 3.12)
project(muse                VERSION 1.0
        DESCRIPTION         "Mike's User Space Environment"
        LANGUAGES           CXX)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    set(CMAKE_BUILD_TYPE "Debug")
endif()

include_directories(SYSTEM sys/inc)

add_subdirectory(sys)
add_subdirectory(test)
add_subdirectory(util)
