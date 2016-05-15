TEST_F(Parser, parseIntSimple)
{
	const char *input = "1234567890";
	const int solution = 1234567890;

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result != NULL) << "parseInt should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "parseInt should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "parseInt should parse the correct integer value";
	StoreFree(result);

	assertReportSuccess("int");
}

TEST_F(Parser, parseIntNegative)
{
	const char *input = "-1234567890";
	const int solution = -1234567890;

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result != NULL) << "parseInt should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "parseInt should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "parseInt should parse the correct integer value";
	StoreFree(result);

	assertReportSuccess("int");
}

TEST_F(Parser, parseIntInterrupted)
{
	const char *input = "1x";
	const int solution = 1;

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result != NULL) << "parseInt should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "parseInt should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "parseInt should parse the correct integer value";
	ASSERT_EQ(state.position.index, 1) << "state position index should have moved to interrupting character";
	ASSERT_EQ(state.position.column, 2) << "state position column should have moved to interrupting character";
	StoreFree(result);

	assertReportSuccess("int");
}

TEST_F(Parser, parseIntGap)
{
	const char *input = "1 2";
	const int solution = 1;

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result != NULL) << "parseInt should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "parseInt should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "parseInt should parse the correct integer value";
	ASSERT_EQ(state.position.index, 1) << "state position index should have moved to delimiter";
	ASSERT_EQ(state.position.column, 2) << "state position column should have moved to delimiter";
	StoreFree(result);

	assertReportSuccess("int");
}

TEST_F(Parser, parseIntInvalid)
{
	const char *input = "asdf";

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result == NULL) << "parseInt should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("int");
}

TEST_F(Parser, parseIntInvalidEmpty)
{
	const char *input = "";

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result == NULL) << "parseInt should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("int");
}

TEST_F(Parser, parseIntInvalidOffset)
{
	const char *input = "  ,;  1";

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result == NULL) << "parseInt should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("int");
}

TEST_F(Parser, parseIntInvalidNegative)
{
	const char *input = "-asdf";

	Store *result = parseInt(input, &state);
	ASSERT_TRUE(result == NULL) << "parseInt should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("int");
}
