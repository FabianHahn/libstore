include_directories(${GTEST_INCLUDE_DIR})

set(LIBSTORE_TEST_SRC
	libstoretest.cpp
	parser.cpp
)

add_executable(libstoretests ${LIBSTORE_TEST_SRC})
target_link_libraries(libstoretests ${GTEST_LIBRARY})
target_link_libraries(libstoretests store)

add_test(LibStoreTests libstoretests)