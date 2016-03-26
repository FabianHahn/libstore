cmake_minimum_required(VERSION 3.1)
project(libstore)

include(${CMAKE_CURRENT_SOURCE_DIR}/gtest.cmake)
enable_testing()

add_subdirectory(src)