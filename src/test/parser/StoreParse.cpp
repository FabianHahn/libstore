TEST_F(Parser, StoreParseValue)
{
	const char *input = " 123 ";
	int solution = 123;

	Store *result = StoreParse(input, &state);
	ASSERT_TRUE(result != NULL) << "StoreParse should not return NULL";
	ASSERT_EQ(result->type, STORE_INT) << "StoreParse should return a store of type int";
	ASSERT_EQ(result->content.intValue, solution) << "StoreParse should parse the correct int value";
	ASSERT_EQ(state.position.index, 5) << "state position index should have moved to end of input";
	ASSERT_EQ(state.position.column, 6) << "state position column should have moved to end of input";

	StoreFree(result);
}

TEST_F(Parser, StoreParseEntries)
{
	const char *input = " foo = bar; baz = 3.14159265359 ";
	const char *solution_key1 = "foo";
	const char *solution_value1 = "bar";
	const char *solution_key2 = "baz";
	double solution_value2 = 3.14159265359;

	Store *result = StoreParse(input, &state);
	ASSERT_TRUE(result != NULL) << "StoreParse should not return NULL";
	ASSERT_EQ(result->type, STORE_MAP) << "StoreParse should return a store of type map";
	ASSERT_EQ(StoreGetMapSize(result->content.mapValue), 2) << "parsed map should have two entries";
	ASSERT_EQ(state.position.index, 32) << "state position index should have moved to end of input";
	ASSERT_EQ(state.position.column, 33) << "state position column should have moved to end of input";

	StoreMapIterator iter = StoreCreateMapIterator(result->content.mapValue);
	ASSERT_TRUE(StoreIsMapIteratorValid(iter)) << "store map iterator should be valid";

	int hits = 0;
	for(; StoreIsMapIteratorValid(iter); StoreForwardMapIterator(iter)) {
		const char *key = StoreGetCurrentMapIteratorKey(iter);
		ASSERT_TRUE(key != NULL) << "StoreGetCurrentMapIteratorKey should not return NULL when iterator is still valid";
		Store *value = StoreGetCurrentMapIteratorValue(iter);
		ASSERT_TRUE(value != NULL) << "StoreGetCurrentMapIteratorValue should not return NULL when iterator is still valid";

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
	StoreFreeMapIterator(iter);

	ASSERT_EQ(hits, 2) << "loop body should have hit two times";

	StoreFree(result);
}
