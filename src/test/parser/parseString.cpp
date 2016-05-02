TEST_F(Parser, parseStringSimple)
{
	const char *input = "simplestring";
	const char *solution = "simplestring";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseStringSimpleUnicode)
{
	const char *input = "@äच€☃☆✈ð";
	const char *solution = "@äच€☃☆✈ð";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseStringSimpleInterrupted)
{
	const char *input = "not\"simple";
	const char *solution = "not";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";
	ASSERT_EQ(state.position.index, 3) << "state position index should have moved to delimiter";
	ASSERT_EQ(state.position.column, 4) << "state position column should have moved to delimiter";

	StoreFree(result);
}


TEST_F(Parser, parseStringSimpleGap)
{
	const char *input = "three simple strings";
	const char *solution = "three";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";
	ASSERT_EQ(state.position.index, 5) << "state position index should have moved to delimiter";
	ASSERT_EQ(state.position.column, 6) << "state position column should have moved to delimiter";

	StoreFree(result);
}

TEST_F(Parser, parseStringLongButSimple)
{
	const char *input = "\"longstring\"";
	const char *solution = "longstring";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseStringLongNotSimple)
{
	const char *input = "\" this is,a;long\tstring\"";
	const char *solution = " this is,a;long\tstring";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseStringLongEscaped)
{
	const char *input = "\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t \"";
	const char *solution = " \" \\ / \b \f \n \r \t ";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseStringLongEncodedUnicode)
{
	const char *input = "\"\\u0040\\u00e4\\u091A\\ue02E\"";
	const char *solution = "@äच";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseStringLongWithNewlines)
{
	const char *input = "\"this is a\nlong string over\nseveral lines\"";
	const char *solution = "this is a\nlong string over\nseveral lines";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";
	ASSERT_EQ(state.position.index, 42) << "index should not have wrapped around over newlines";
	ASSERT_EQ(state.position.column, 15) << "column should have wrapped around over newlines";
	ASSERT_EQ(state.position.line, 3) << "line should have increased over newlines";

	StoreFree(result);
}

TEST_F(Parser, parseStringLongEmpty)
{
	const char *input = "\"\"";
	const char *solution = "";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result != NULL) << "parseString should not return NULL";
	ASSERT_EQ(result->type, STORE_STRING) << "parseString should return a store of type string";
	ASSERT_STREQ(result->content.stringValue, solution) << "parseString should parse the correct string value";

	StoreFree(result);
}

TEST_F(Parser, parseStringInvalid)
{
	const char *input = "=";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result == NULL) << "parseString should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseStringInvalidEmpty)
{
	const char *input = "";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result == NULL) << "parseString should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseStringInvalidOffset)
{
	const char *input = "  ,;  asdf ";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result == NULL) << "parseString should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseStringInvalidUnclosed)
{
	const char *input = "\"";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result == NULL) << "parseString should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseStringInvalidEscaped)
{
	const char *input = "\"\\yolo\"";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result == NULL) << "parseString should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseStringInvalidEncoded)
{
	const char *input = "\"\\uasdf\"";

	Store *result = parseString(input, &state);
	ASSERT_TRUE(result == NULL) << "parseString should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}
