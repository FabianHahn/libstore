TEST_F(Parser, parseFloatSimple)
{
	const char *input = "123.456";
	const double solution = 123.456;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatNegative)
{
	const char *input = "-123.456";
	const double solution = -123.456;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatExponential)
{
	const char *input = "123.456e123";
	const double solution = 123.456e123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatPositiveExponential)
{
	const char *input = "123.456e+123";
	const double solution = 123.456e+123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatNegativeExponential)
{
	const char *input = "123.456e-123";
	const double solution = 123.456e-123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatCapitalExponential)
{
	const char *input = "123.456E123";
	const double solution = 123.456e123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatCapitalPositiveExponential)
{
	const char *input = "123.456E+123";
	const double solution = 123.456e+123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatCapitalNegativeExponential)
{
	const char *input = "123.456E-123";
	const double solution = 123.456e-123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatInt)
{
	const char *input = "123";
	const double solution = 123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatIntDot)
{
	const char *input = "123.";
	const double solution = 123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	ASSERT_EQ(state.position.index, 4) << "state position index should have moved past dot character";
	ASSERT_EQ(state.position.column, 5) << "state position column should have moved past dot character";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatZeroPoint)
{
	const char *input = "0.123";
	const double solution = 0.123;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatInterrupted)
{
	const char *input = "1.2x";
	const double solution = 1.2;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	ASSERT_EQ(state.position.index, 3) << "state position index should have moved to interrupting character";
	ASSERT_EQ(state.position.column, 4) << "state position column should have moved to interrupting character";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatInterruptedExponential)
{
	const char *input = "1.2e";
	const double solution = 1.2;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	ASSERT_EQ(state.position.index, 3) << "state position index should have moved to interrupting character";
	ASSERT_EQ(state.position.column, 4) << "state position column should have moved to interrupting character";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatGap)
{
	const char *input = "1.2 3.4";
	const double solution = 1.2;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result != NULL) << "parseFloat should not return NULL";
	ASSERT_EQ(result->type, STORE_FLOAT) << "parseFloat should return a store of type float";
	ASSERT_EQ(result->content.floatValue, solution) << "parseFloat should parse the correct float value";
	ASSERT_EQ(state.position.index, 3) << "state position index should have moved to delimiter";
	ASSERT_EQ(state.position.column, 4) << "state position column should have moved to delimiter";
	StoreFree(result);

	assertReportSuccess("float");
}

TEST_F(Parser, parseFloatInvalid)
{
	const char *input = "asdf";

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result == NULL) << "parseInt should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("float");
}

TEST_F(Parser, parseFloatInvalidEmpty)
{
	const char *input = "";

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result == NULL) << "parseFloat should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("float");
}

TEST_F(Parser, parseFloatInvalidOffset)
{
	const char *input = "  ,;  1.2";
	const double solution = 1.2;

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result == NULL) << "parseFloat should return NULL";
	ASSERT_EQ(state.position.index, 0) << "parse state index should not have changed";
	ASSERT_EQ(state.position.column, 1) << "parse state column should not have changed";

	assertReportFailure("float");
}

TEST_F(Parser, parseFloatInvalidPureFractional)
{
	const char *input = ".456";

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result == NULL) << "parseFloat should return NULL";
	ASSERT_EQ(state.position.index, 0) << "state position index should be unchanged";
	ASSERT_EQ(state.position.column, 1) << "state position column should be unchanged";
	ASSERT_EQ(state.position.line, 1) << "state position column should be unchanged";

	assertReportFailure("float");
}

TEST_F(Parser, parseFloatInvalidPureExponential)
{
	const char *input = "e123";

	Store *result = parseFloat(input, &state);
	ASSERT_TRUE(result == NULL) << "parseFloat should return NULL";
	ASSERT_EQ(state.position.index, 0) << "state position index should be unchanged";
	ASSERT_EQ(state.position.column, 1) << "state position column should be unchanged";
	ASSERT_EQ(state.position.line, 1) << "state position column should be unchanged";

	assertReportFailure("float");
}
