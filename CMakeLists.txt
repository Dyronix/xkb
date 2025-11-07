cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

project(xkb)

set(XKB_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include)
set(XKB_SOURCE_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/src)

add_library(xkb STATIC)

# Sources
target_sources(xkb PRIVATE
    ${XKB_SOURCE_DIRECTORY}/xkbX11KeyboardBackend.cpp
    ${XKB_SOURCE_DIRECTORY}/xkbEvDevKeyboardBackend.cpp
    ${XKB_SOURCE_DIRECTORY}/xkbInputDetection.cpp
)

target_sources(xkb PUBLIC
    ${XKB_INCLUDE_DIRECTORY}/xkbKeyboardBackend.h
    ${XKB_INCLUDE_DIRECTORY}/xkbX11KeyboardBackend.h
    ${XKB_INCLUDE_DIRECTORY}/xkbInputDetection.h
)

target_include_directories(xkb PUBLIC ${XKB_INCLUDE_DIRECTORY})

# Core Xlib
find_package(X11 REQUIRED)
target_link_libraries(xkb PRIVATE X11::X11)