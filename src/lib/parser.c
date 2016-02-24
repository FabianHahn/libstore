#include <stddef.h> // NULL

#include "memory.h"
#include "parser.h"

typedef struct {
	StoreDynamicString message;
	int level;
} ParseError;

static Store *parseValue(const char *input, ParseError *error);
static Store *parseString(const char *input, ParseError *error);
static Store *parseInt(const char *input, ParseError *error);
static Store *parseFloat(const char *input, ParseError *error);
static Store *parseList(const char *input, ParseError *error);
static Store *parseStruct(const char *input, ParseError *error);

StoreParseResult *StoreParse(const char *input)
{
	ParseError error;
	error.message = StoreCreateDynamicStringType();
	error.level = 0;

	StoreParseResult *result = StoreAllocateMemoryType(StoreParseResult);

	// Try to parse the store as a value
	StoreDynamicString valueStoreError = StoreCreateDynamicStringType();
	Store *valueStore = parseValue(input, &error);
	if(valueStore != NULL) {
		result->status = STORE_PARSE_SUCCESS;
		result->store = valueStore;
	} else {
		result->status = STORE_PARSE_ERROR;
	}

	StoreFreeDynamicStringType(valueStoreError);

	return result;
}

static Store *parseValue(const char *input, ParseError *error)
{
	ParseError valueError;
	valueError.message = StoreCreateDynamicStringType();
	valueError.level = error->level + 1;

	Store *stringStore = parseString(input, &valueError);
	if(stringStore != NULL) {
		StoreFreeDynamicStringType(valueError.message);
		return stringStore;
	}

	Store *intStore = parseInt(input, &valueError);
	if(intStore != NULL) {
		StoreFreeDynamicStringType(valueError.message);
		return intStore;
	}

	Store *floatStore = parseFloat(input, &valueError);
	if(floatStore != NULL) {
		StoreFreeDynamicStringType(valueError.message);
		return floatStore;
	}

	Store *listStore = parseList(input, &valueError);
	if(listStore != NULL) {
		StoreFreeDynamicStringType(valueError.message);
		return listStore;
	}

	Store *structStore = parseStruct(input, &valueError);
	if(structStore != NULL) {
		StoreFreeDynamicStringType(valueError.message);
		return structStore;
	}

	for(int i = 0; i < error->level; i++) {
		StoreAppendDynamicString(error->message, "\t");
	}
	StoreAppendDynamicString(error->message, "Failed to parse value:\n%s", valueError.message);
	StoreFreeDynamicStringType(valueError.message);

	return NULL;
}

static Store *parseString(const char *input, ParseError *error)
{

}

static Store *parseInt(const char *input, ParseError *error)
{

}

static Store *parseFloat(const char *input, ParseError *error)
{

}

static Store *parseList(const char *input, ParseError *error)
{

}

static Store *parseStruct(const char *input, ParseError *error)
{

}
