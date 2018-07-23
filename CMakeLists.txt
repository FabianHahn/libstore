cmake_minimum_required(VERSION 3.1)
project(libstore)

enable_testing()

option(LIBSTORE_BUILD_SHARED "Build shared libstore." ON)
option(LIBSTORE_BUILD_STATIC "Build static libstore." ON)
option(LIBSTORE_BUILD_TESTS "Build libstore unit tests." ON)
option(LIBSTORE_INCLUDE_FAKEGLIB "Include the vendored thirdparty fakeglib library." ON)
option(LIBSTORE_INCLUDE_GOOGLETEST "Include the vendored thirdparty googletest library." ON)

set(FAKEGLIB_BUILD_TESTS OFF CACHE BOOL "Disable building fakeglib tests")

if(NOT LIBSTORE_BUILD_STATIC)
	set(FAKEGLIB_BUILD_STATIC OFF CACHE BOOL "Disable building static fakeglib")
endif()

if(NOT LIBSTORE_BUILD_SHARED)
	set(FAKEGLIB_BUILD_SHARED OFF CACHE BOOL "Disable building shared fakeglib")
endif()

add_subdirectory(lib)
add_subdirectory(thirdparty)
