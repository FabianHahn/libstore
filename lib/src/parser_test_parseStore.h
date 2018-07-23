#include <glib.h>
#include <gtest/gtest.h>

#include "store/store.h"

TEST_F(Parser, parseStoreValue)
{
	const char *input = " 123 ";
	int solution = 123;

	Store *result = parseStore(input, &state);
	ASSERT_TRUE(result != NULL) << "StoreParse should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "StoreParse should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "StoreParse should parse the correct int value";
	ASSERT_EQ(state.position.index, 5) << "state position index should have moved to end of input";
	ASSERT_EQ(state.position.column, 6) << "state position column should have moved to end of input";
	storeFree(result);

	assertReportSuccess("store");
}

TEST_F(Parser, parseStoreEntries)
{
	const char *input = " foo = bar; baz = 3.14159265359 ";
	const char *solution_key1 = "foo";
	const char *solution_value1 = "bar";
	const char *solution_key2 = "baz";
	double solution_value2 = 3.14159265359;

	Store *result = parseStore(input, &state);
	ASSERT_TRUE(result != NULL) << "StoreParse should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "StoreParse should return a store of type map";
	ASSERT_EQ(g_hash_table_size(result->content.mapValue), 2) << "parsed map should have two entries";
	ASSERT_EQ(state.position.index, 32) << "state position index should have moved to end of input";
	ASSERT_EQ(state.position.column, 33) << "state position column should have moved to end of input";

	GHashTableIter iter;
	g_hash_table_iter_init(&iter, result->content.mapValue);

	int hits = 0;
	const char *key;
	Store *value;
	while(g_hash_table_iter_next(&iter, (gpointer *) &key, (gpointer *) &value)) {
		ASSERT_TRUE(key != NULL) << "iterator key should not be NULL when iterator is still valid";
		ASSERT_TRUE(value != NULL) << "iterator key should not be NULL when iterator is still valid";

		std::string stdStringKey(key);
		if(stdStringKey == solution_key1) {
			ASSERT_EQ(value->type, STORE_STRING) << "value of first map entry should be a store of type string";
			ASSERT_STREQ(value->content.stringValue, solution_value1) << "value of first map entry should have been parsed to the correct string value";
			hits++;
		} else if(stdStringKey == solution_key2) {
			ASSERT_EQ(value->type, STORE_FLOAT) << "value of second map entry should be a store of type float";
			ASSERT_EQ(value->content.floatValue, solution_value2) << "value of second map entry should have been parsed to the correct float value";
			hits++;
		} else {
			ASSERT_TRUE(false) << "invalid parsed entry with key: " << stdStringKey;
		}
	}

	ASSERT_EQ(hits, 2) << "loop body should have hit two times";

	storeFree(result);

	assertReportSuccess("store");
}

TEST_F(Parser, parseStoreInvalidElements)
{
	const char *input = " hello world ";
	Store *result = parseStore(input, &state);
	ASSERT_TRUE(result == NULL) << "StoreParse should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("store");
}
