include_directories(${GTEST_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)

set(LIBSTORE_LOCATION $<TARGET_FILE_DIR:store>)
include_directories(${LIBSTORE_LOCATION})

set(LIBSTORE_TEST_SRC
	libstoretest.cpp
	parser.cpp
	../lib/encoding.c
	../lib/memory.c
	../lib/parser.c
	../lib/store.c
	../lib/types/stl/list.cpp
	../lib/types/stl/map.cpp
	../lib/types/stl/string.cpp
)

add_executable(libstoretest ${LIBSTORE_TEST_SRC})
target_link_libraries(libstoretest ${GTEST_LIBRARY})

add_test(LibStoreTests libstoretest)