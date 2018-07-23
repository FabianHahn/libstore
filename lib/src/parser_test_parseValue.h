#include <glib.h>
#include <gtest/gtest.h>

#include "store/store.h"

TEST_F(Parser, parseValuePrefixInt)
{
	const char *input = "123";
	int solution = 123;

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "parseValue should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "parseValue should parse the correct int value";
	storeFree(result);

	assertReportSuccess("value");
}

TEST_F(Parser, parseValuePrefixFloat)
{
	const char *input = "123.";
	double solution = 123.;

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseValue should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseValue should parse the correct float value";
	storeFree(result);

	assertReportSuccess("value");
}

TEST_F(Parser, parseValuePrefixString)
{
	const char *input = "123.e";
	const char *solution = "123.e";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseValue should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseValue should parse the correct string value";
	storeFree(result);

	assertReportSuccess("value");
}

TEST_F(Parser, parseValuePrefixFloatExp)
{
	const char *input = "123.e4";
	double solution = 123.e4;

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseValue should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseValue should parse the correct float value";
	storeFree(result);

	assertReportSuccess("value");
}

TEST_F(Parser, parseValueStringOffset)
{
	const char *input = "  ,;  \"hello world\" ;,  ";
	const char *solution = "hello world";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseValue should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseValue should parse the correct string value";
	ASSERT_EQ(state.position.index, 19) << "index should not have moved past suffix offset";
	ASSERT_EQ(state.position.column, 20) << "column should not have moved past suffix offset";
	storeFree(result);

	assertReportSuccess("value");
}


TEST_F(Parser, parseValueEmptyList)
{
	const char *input = "()";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseValue should return a store of type list";
	ASSERT_EQ(g_queue_get_length(result->content.listValue), 0) << "parseValue list should be empty";
	storeFree(result);

	assertReportSuccess("value");
}

TEST_F(Parser, parseValueEmptyMap)
{
	const char *input = "{}";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "parseValue should return a store of type map";
	ASSERT_EQ(g_hash_table_size(result->content.mapValue), 0) << "parseValue map should be empty";
	storeFree(result);

	assertReportSuccess("value");
}

TEST_F(Parser, parseValueInvalidEmptyListContinued)
{
	const char *input = "()asdf";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result == NULL) << "parseValue should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("value");
}

TEST_F(Parser, parseValueInvalidEmptyMapContinued)
{
	const char *input = "{}asdf";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result == NULL) << "parseValue should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("value");
}
