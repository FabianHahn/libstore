TEST_F(Parser, parseValuePrefixInt)
{
	const char *input = "123";
	int solution = 123;

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "parseValue should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "parseValue should parse the correct int value";

	StoreFree(result);
}

TEST_F(Parser, parseValuePrefixFloat)
{
	const char *input = "123.";
	double solution = 123.;

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseValue should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseValue should parse the correct float value";

	StoreFree(result);
}

TEST_F(Parser, parseValuePrefixString)
{
	const char *input = "123.e";
	const char *solution = "123.e";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseValue should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseValue should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseValuePrefixFloatExp)
{
	const char *input = "123.e4";
	double solution = 123.e4;

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseValue should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseValue should parse the correct float value";

	StoreFree(result);
}

TEST_F(Parser, parseValueEmptyList)
{
	const char *input = "()";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseValue should return a store of type list";
	ASSERT_EQ(StoreGetListSize(result->content.listValue), 0) << "parseValue list should be empty";

	StoreFree(result);
}

TEST_F(Parser, parseValueEmptyMap)
{
	const char *input = "{}";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result != NULL) << "parseValue should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "parseValue should return a store of type map";
	ASSERT_EQ(StoreGetMapSize(result->content.listValue), 0) << "parseValue map should be empty";

	StoreFree(result);
}

TEST_F(Parser, parseValueInvalidEmptyListContinued)
{
	const char *input = "()asdf";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result == NULL) << "parseValue should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseValueInvalidEmptyMapContinued)
{
	const char *input = "{}asdf";

	Store *result = parseValue(input, &state);
	ASSERT_TRUE(result == NULL) << "parseValue should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}
