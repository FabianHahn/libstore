#include <gtest/gtest.h>

#include <memory.c>
#include <parser.c>
#include <store.c>
#include <types/stl/list.cpp>
#include <types/stl/map.cpp>
#include <types/stl/string.cpp>

class Parser : public ::testing::Test {
	public:
	  virtual void SetUp() {
		state.position.index = 0;
		state.position.line = 1;
		state.position.column = 1;
		state.error = StoreCreateDynamicString();
		state.level = 0;
	  }

	  virtual void TearDown() {
		  StoreFreeDynamicString(state.error);
	  }

	  ParseState state;
};

#include "parser/parseInt.cpp"
