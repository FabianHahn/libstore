include(GenerateExportHeader)

set(LIBSTORE_LIB_SRC
	src/types/stl/list.cpp
	src/types/stl/map.cpp
	src/types/stl/string.cpp
	src/encoding.c
	src/memory.c
	src/parser.c
	src/report.c
	src/store.c
	include/store/encoding.h
	include/store/memory.h
	include/store/parser.h
	include/store/report.h
	include/store/store.h
	include/store/types.h
)

set(LIBSTORE_LIB_TEST_SRC
	src/parser_test.cpp
	src/parser_test_parseFloat.h
	src/parser_test_parseInt.h
	src/parser_test_parseList.h
	src/parser_test_parseMap.h
	src/parser_test_parseStore.h
	src/parser_test_parseString.h
	src/parser_test_parseValue.h
	src/test.cpp
)

if(LIBSTORE_BUILD_SHARED)
	add_library(store SHARED ${LIBSTORE_LIB_SRC})
	set_property(TARGET store PROPERTY CXX_STANDARD 11)
	
	generate_export_header(store
		EXPORT_MACRO_NAME LIBSTORE_API
		EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/store/api.h
		DEPRECATED_MACRO_NAME LIBSTORE_DEPRECATED
		NO_EXPORT_MACRO_NAME LIBSTORE_NO_EXPORT
		STATIC_DEFINE LIBSTORE_STATIC
		NO_DEPRECATED_MACRO_NAME LIBSTORE_NO_DEPRECATED
	)
	
	target_include_directories(store
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	    PUBLIC $<INSTALL_INTERFACE:include>
	    PRIVATE src)
endif()

if(LIBSTORE_BUILD_STATIC)
	add_library(storestatic STATIC ${LIBSTORE_LIB_SRC})
	set_property(TARGET storestatic PROPERTY CXX_STANDARD 11)
	target_compile_definitions(storestatic PUBLIC LIBSTORE_STATIC)
	
	generate_export_header(storestatic
		EXPORT_MACRO_NAME LIBSTORE_API
		EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/store/api.h
		DEPRECATED_MACRO_NAME LIBSTORE_DEPRECATED
		NO_EXPORT_MACRO_NAME LIBSTORE_NO_EXPORT
		STATIC_DEFINE LIBSTORE_STATIC
		NO_DEPRECATED_MACRO_NAME LIBSTORE_NO_DEPRECATED
	)
	
	target_include_directories(storestatic
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	    PUBLIC $<INSTALL_INTERFACE:include>
	    PRIVATE src)

	if(LIBSTORE_BUILD_TESTS)
		add_executable(store_test ${LIBSTORE_LIB_TEST_SRC})
		target_link_libraries(store_test storestatic gtest)
		set_property(TARGET store_test PROPERTY CXX_STANDARD 11)
		add_test(store store_test)
	endif()
endif()

