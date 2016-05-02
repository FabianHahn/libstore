#include <gtest/gtest.h>

#include <encoding.c>
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
		state.lastReport = NULL;
	  }

	  virtual void TearDown() {

	  }

	  StoreParseState state;
};

#include "parser/StoreParse.cpp"
#include "parser/parseValue.cpp"
#include "parser/parseString.cpp"
#include "parser/parseInt.cpp"
#include "parser/parseFloat.cpp"
#include "parser/parseList.cpp"
#include "parser/parseMap.cpp"
