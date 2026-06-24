# Oregon Trail port build.
#
#   cmake -B build/port -S port                       # auto-detects SDL2
#   cmake -B build/port -S port -DOTRAIL_BACKEND=file # force the headless backend
#   cmake --build build/port
#
# Backends:
#   sdl   - interactive SDL2 window (desktop/web/mobile/Switch/Wii U)
#   file  - headless; pal_present writes a PNG (no GUI deps), for CI / proof
#
# Cross-compiling (web/console) is a matter of pointing cmake at the right
# toolchain file (Emscripten, devkitPro, Android NDK, iOS); the sources don't
# change.
cmake_minimum_required(VERSION 3.16)
project(oregon_trail_port C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CORE core/main.c core/pcx.c)

set(OTRAIL_BACKEND "auto" CACHE STRING "PAL backend: auto | sdl | file")

if(OTRAIL_BACKEND STREQUAL "auto")
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        set(OTRAIL_BACKEND "sdl")
    else()
        set(OTRAIL_BACKEND "file")
    endif()
endif()

message(STATUS "Oregon Trail port backend: ${OTRAIL_BACKEND}")

if(OTRAIL_BACKEND STREQUAL "sdl")
    find_package(SDL2 REQUIRED)
    add_executable(oregon_trail ${CORE} platform/sdl/pal_sdl.c)
    target_include_directories(oregon_trail PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(oregon_trail PRIVATE ${SDL2_LIBRARIES})
else()
    add_executable(oregon_trail ${CORE} platform/file/pal_file.c)
endif()
