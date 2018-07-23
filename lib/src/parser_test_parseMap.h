#include <glib.h>
#include <gtest/gtest.h>

#include "store/store.h"

TEST_F(Parser, parseMapEmpty)
{
	const char *input = "{}";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result != NULL) << "parseMap should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "parseMap should return a store of type map";
	ASSERT_EQ(g_hash_table_size(result->content.mapValue), 0) << "parsed map should be empty";
	storeFree(result);

	assertReportSuccess("map");
}

TEST_F(Parser, parseMapEmptyGap)
{
	const char *input = "{  , ; }";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result != NULL) << "parseMap should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "parseMap should return a store of type map";
	ASSERT_EQ(g_hash_table_size(result->content.mapValue), 0) << "parsed map should be empty";
	storeFree(result);

	assertReportSuccess("map");
}

TEST_F(Parser, parseMapSimple)
{
	const char *input = "{hello = world}";
	const char *solution_key = "hello";
	const char *solution_value = "world";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result != NULL) << "parseMap should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "parseMap should return a store of type map";
	ASSERT_EQ(g_hash_table_size(result->content.mapValue), 1) << "parsed map should have one entry";

	GHashTableIter iter;
	g_hash_table_iter_init(&iter, result->content.mapValue);

	const char *key;
	Store *value;
	bool firstIteration = g_hash_table_iter_next(&iter, (gpointer *) &key, (gpointer *) &value);
	ASSERT_TRUE(firstIteration) << "store map iterator should be valid";
	ASSERT_TRUE(key != NULL) << "StoreGetCurrentMapIteratorKey should not return NULL when iterator is still valid";
	ASSERT_STREQ(key, solution_key) << "key of map entry should have been parsed correctly";
	ASSERT_TRUE(value != NULL) << "StoreGetCurrentMapIteratorValue should not return NULL when iterator is still valid";
	ASSERT_EQ(value->type, STORE_STRING) << "value of map entry should be a store of type string";
	ASSERT_STREQ(value->content.stringValue, solution_value) << "value of map entry should have been parsed to the correct string value";

	bool secondIteration = g_hash_table_iter_next(&iter, (gpointer *) &key, (gpointer *) &value);
	ASSERT_FALSE(secondIteration) << "forwarding map iterator should reach the end";

	storeFree(result);

	assertReportSuccess("map");
}

TEST_F(Parser, parseMapMultiple)
{
	const char *input = "{\"the answer\" = 42, foo = \"bar\"\nnested: {}}";
	const char *solution_key1 = "the answer";
	int solution_value1 = 42;
	const char *solution_key2 = "foo";
	const char *solution_value2 = "bar";
	const char *solution_key3 = "nested";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result != NULL) << "parseMap should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "parseMap should return a store of type map";
	ASSERT_EQ(g_hash_table_size(result->content.mapValue), 3) << "parsed map should have three entries";

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
			ASSERT_EQ(value->type, STORE_INT) << "value of first map entry should be a store of type int";
			ASSERT_EQ(value->content.intValue, solution_value1) << "value of first map entry should have been parsed to the correct int value";
			hits++;
		} else if(stdStringKey == solution_key2) {
			ASSERT_EQ(value->type, STORE_STRING) << "value of second map entry should be a store of type string";
			ASSERT_STREQ(value->content.stringValue, solution_value2) << "value of second map entry should have been parsed to the correct string value";
			hits++;
		} else if(stdStringKey == solution_key3) {
			ASSERT_EQ(value->type, STORE_MAP) << "value of third map entry should be a store of type map";
			ASSERT_EQ(g_hash_table_size(value->content.mapValue), 0) << "value of third map entry should have been parsed to an empty map value";
			hits++;
		} else {
			ASSERT_TRUE(false) << "invalid parsed entry with key: " << stdStringKey;
		}
	}

	ASSERT_EQ(hits, 3) << "loop body should have hit three times";

	storeFree(result);

	assertReportSuccess("map");
}

TEST_F(Parser, parseMapInvalid)
{
	const char *input = "asdf";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result == NULL) << "parseMap should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("map");
}

TEST_F(Parser, parseMapInvalidEmpty)
{
	const char *input = "";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result == NULL) << "parseMap should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("map");
}

TEST_F(Parser, parseMapInvalidOffset)
{
	const char *input = "  , ; {}  ";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result == NULL) << "parseMap should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("map");
}

TEST_F(Parser, parseMapInvalidUnclosed)
{
	const char *input = "{";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result == NULL) << "parseMap should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("map");
}

TEST_F(Parser, parseMapInvalidEntry)
{
	const char *input = "{asdf}";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result == NULL) << "parseMap should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("map");
}

TEST_F(Parser, parseMapInvalidEntryKey)
{
	const char *input = "{\"\\uasdf\" = 42}";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result == NULL) << "parseMap should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("map");
}

TEST_F(Parser, parseMapInvalidEntryValue)
{
	const char *input = "{foo = \"starting but not terminating...}";

	Store *result = parseMap(input, &state);
	ASSERT_TRUE(result == NULL) << "parseMap should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("map");
}
