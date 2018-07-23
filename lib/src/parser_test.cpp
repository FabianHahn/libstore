#include <gtest/gtest.h>

#include "encoding.c"
#include "memory.c"
#include "parser.c"
#include "store.c"
#include "types/stl/list.cpp"
#include "types/stl/map.cpp"
#include "types/stl/string.cpp"

class Parser: public ::testing::Test {
public:
	virtual void SetUp() {
		state.position.index = 0;
		state.position.line = 1;
		state.position.column = 1;
		state.lastReport = NULL;
	}

	virtual void TearDown() {
		freeParseReport(state.lastReport);
	}

protected:
	virtual void assertReportSuccess(const char *type) {
		ASSERT_TRUE(state.lastReport != NULL)<< "parse state's last report should not be NULL";
		ASSERT_TRUE(state.lastReport->previousReport == NULL) << "parse state should only have one report";
		ASSERT_TRUE(state.lastReport->success) << "parse state's report should be successful";
		ASSERT_STREQ(state.lastReport->type, type) << "parse state's report should have the correct type";
	}

	virtual void assertReportFailure(const char *type) {
		ASSERT_TRUE(state.lastReport != NULL)<< "parse state's last report should not be NULL";
		ASSERT_TRUE(state.lastReport->previousReport == NULL) << "parse state should only have one report";
		ASSERT_FALSE(state.lastReport->success) << "parse state's report should be a failure";
		ASSERT_STREQ(state.lastReport->type, type) << "parse state's report should have the correct type";
	}

	StoreParseState state;
};

#include "parser_test_parseStore.h"
#include "parser_test_parseValue.h"
#include "parser_test_parseString.h"
#include "parser_test_parseInt.h"
#include "parser_test_parseFloat.h"
#include "parser_test_parseList.h"
#include "parser_test_parseMap.h"
