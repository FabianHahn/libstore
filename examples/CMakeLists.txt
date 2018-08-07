set(LIBSTORE_EXAMPLE_STORE_PARSER_SRC
		store_parser.c
)

if(LIBSTORE_BUILD_EXAMPLES)
	add_executable(store_parser ${LIBSTORE_EXAMPLE_STORE_PARSER_SRC})
	set_property(TARGET store_parser PROPERTY C_STANDARD 99)
	target_link_libraries(store_parser storestatic)
endif()

