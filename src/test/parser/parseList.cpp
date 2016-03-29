TEST_F(Parser, parseListEmpty)
{
	const char *input = "()";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result != NULL) << "parseList should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseList should return a store of type list";
	ASSERT_EQ(StoreGetListSize(result->content.listValue), 0) << "parsed list should be empty";

	StoreFree(result);
}

TEST_F(Parser, parseListEmptyBrackets)
{
	const char *input = "[]";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result != NULL) << "parseList should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseList should return a store of type list";
	ASSERT_EQ(StoreGetListSize(result->content.listValue), 0) << "parsed list should be empty";

	StoreFree(result);
}

TEST_F(Parser, parseListEmptyGap)
{
	const char *input = "(  , ; )";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result != NULL) << "parseList should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseList should return a store of type list";
	ASSERT_EQ(StoreGetListSize(result->content.listValue), 0) << "parsed list should be empty";

	StoreFree(result);
}

TEST_F(Parser, parseListOffset)
{
	const char *input = "  , ; ()  ";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result != NULL) << "parseList should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseList should return a store of type list";
	ASSERT_EQ(StoreGetListSize(result->content.listValue), 0) << "parsed list should be empty";
	ASSERT_EQ(state.position.index, 8) << "index should not have moved past suffix offset";
	ASSERT_EQ(state.position.column, 9) << "column should not have moved past suffix offset";

	StoreFree(result);
}

TEST_F(Parser, parseListMultipleElements)
{
	const char *input = "(the answer is 42)";
	const char *solution1 = "the";
	const char *solution2 = "answer";
	const char *solution3 = "is";
	int solution4 = 42;

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result != NULL) << "parseList should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseList should return a store of type list";
	ASSERT_EQ(StoreGetListSize(result->content.listValue), 4) << "parsed list should have correct size";

	Store *solutionPart1 = StoreGetListElement(result->content.listValue, 0);
	ASSERT_EQ(solutionPart1->type, STORE_STRING) << "first solution element should be a store of type string";
	ASSERT_STREQ(solutionPart1->content.stringValue, solution1) << "first solution value should be correct";

	Store *solutionPart2 = StoreGetListElement(result->content.listValue, 1);
	ASSERT_EQ(solutionPart2->type, STORE_STRING) << "second solution element should be a store of type string";
	ASSERT_STREQ(solutionPart2->content.stringValue, solution2) << "second solution value should be correct";

	Store *solutionPart3 = StoreGetListElement(result->content.listValue, 2);
	ASSERT_EQ(solutionPart3->type, STORE_STRING) << "third solution element should be a store of type string";
	ASSERT_STREQ(solutionPart3->content.stringValue, solution3) << "third solution value should be correct";

	Store *solutionPart4 = StoreGetListElement(result->content.listValue, 3);
	ASSERT_EQ(solutionPart4->type, STORE_INT) << "fourth solution element should be a store of type int";
	ASSERT_EQ(solutionPart4->content.intValue, solution4) << "fourth solution value should be correct";

	StoreFree(result);
}

TEST_F(Parser, parseListNested)
{
	const char *input = "( [ ] )";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result != NULL) << "parseList should not return NULL";
	ASSERT_EQ(result->type, STORE_LIST) << "parseList should return a store of type list";
	ASSERT_EQ(StoreGetListSize(result->content.listValue), 1) << "parsed list should be empty";

	Store *nestedResult = StoreGetListElement(result->content.listValue, 0);
	ASSERT_EQ(nestedResult->type, STORE_LIST) << "nested list element should be another store of type list";
	ASSERT_EQ(StoreGetListSize(nestedResult->content.listValue), 0) << "nested list should be empty";

	StoreFree(result);
}

TEST_F(Parser, parseListInvalid)
{
	const char *input = "asdf";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result == NULL) << "parseList should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseListInvalidEmpty)
{
	const char *input = "";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result == NULL) << "parseList should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseListInvalidUnclosed)
{
	const char *input = "(";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result == NULL) << "parseList should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseListInvalidElement)
{
	const char *input = "(\"\\uasdf\")";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result == NULL) << "parseList should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseListInvalidMixed)
{
	const char *input = "(]";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result == NULL) << "parseList should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}

TEST_F(Parser, parseListInvalidMixedInverse)
{
	const char *input = "[)";

	Store *result = parseList(input, &state);
	ASSERT_TRUE(result == NULL) << "parseList should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";
}
