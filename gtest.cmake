include(ExternalProject)
set(GTest_version 1.7.0)
set(GTest_url "https://github.com/google/googletest/archive/release-${GTest_version}.zip")
set(GTest_md5 "ef5e700c8a0f3ee123e2e0209b8b4961")
set(GTest_BUILD_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest")
set(GTest_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download")

ExternalProject_Add(googletest
  URL ${GTest_url}
  URL_MD5 ${GTest_md5}
  PREFIX  ${GTest_BUILD_PREFIX}
  DOWNLOAD_DIR ${GTest_DOWNLOAD_DIR}
  INSTALL_COMMAND ""
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=ON
)

set(GTEST_ROOT ${GTest_BUILD_INSTALL_PREFIX} CACHE STRING "")

# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIR ${source_dir}/include)

# Library
ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_LIBRARY_PATH "${binary_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}gtest${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(GTEST_LIBRARY gtest)
add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
set_property(TARGET ${GTEST_LIBRARY} PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARY_PATH})
add_dependencies(${GTEST_LIBRARY} googletest)
