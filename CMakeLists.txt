cmake_minimum_required(VERSION 3.1)
project(libstore)

enable_testing()

option(LIBSTORE_BUILD_SHARED "Build shared libstore." ON)
option(LIBSTORE_BUILD_STATIC "Build static libstore." ON)
option(LIBSTORE_BUILD_TESTS "Build libstore unit tests." ON)
option(LIBSTORE_INCLUDE_GOOGLETEST "Include the vendored thirdparty googletest library." ON)

add_subdirectory(lib)
add_subdirectory(thirdparty)
