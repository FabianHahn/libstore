#include <iostream>

#include <glib.h>
#include <gtest/gtest.h>

extern "C" {
	#include <store/report.h>
}

#include "encoding.c"
#include "memory.c"
#include "parser.c"
#include "store.c"

class Parser: public ::testing::Test {
public:
	virtual void SetUp() {
		state.position.index = 0;
		state.position.line = 1;
		state.position.column = 1;
		state.depth = 0;
		state.reports = g_queue_new();
	}

	virtual void TearDown() {
		StoreParser parser{state};
		char *report = storeGenerateParseReport(&parser);
		std::cout << report << std::endl;
		free(report);

		g_queue_free_full(state.reports, freeParseReportPointer);
	}

protected:
	virtual void assertReportSuccess(const char *type) {
		ASSERT_EQ(g_queue_get_length(state.reports), 1) << "parse state should contain a single report";

		StoreParseReport *report = (StoreParseReport *) state.reports->head->data;
		ASSERT_TRUE(report->success) << "parse state's report should be successful";
		ASSERT_STREQ(report->type, type) << "parse state's report should have the correct type";
	}

	virtual void assertReportFailure(const char *type) {
		ASSERT_EQ(g_queue_get_length(state.reports), 1) << "parse state should contain a single report";

		StoreParseReport *report = (StoreParseReport *) state.reports->head->data;
		ASSERT_FALSE(report->success) << "parse state's report should be a failure";
		ASSERT_STREQ(report->type, type) << "parse state's report should have the correct type";
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
