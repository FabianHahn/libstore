#include <ctype.h> // isspace
#include <stdarg.h> // va_list va_start
#include <stdbool.h> // bool true false
#include <stddef.h> // NULL

#include "memory.h"
#include "parser.h"

typedef struct {
	struct {
		int index;
		int line;
		int column;
	} position;
	StoreDynamicString error;
	int level;
} ParseState;

static Store *parseValue(const char *input, ParseState *state);
static Store *parseString(const char *input, ParseState *state);
static Store *parseInt(const char *input, ParseState *state);
static Store *parseFloat(const char *input, ParseState *state);
static Store *parseList(const char *input, ParseState *state);
static Store *parseStruct(const char *input, ParseState *state);
static Store *parseFields(const char *input, ParseState *state);
static char parseNextNonWhitespace(const char *input, ParseState *state);
static void appendParseError(ParseState *state, const char *what, const char *reason, ...);

StoreParseResult *StoreParse(const char *input)
{
	ParseState state;
	state.position.index = 0;
	state.position.line = 1;
	state.position.column = 1;
	state.error = StoreCreateDynamicStringType();
	state.level = 0;

	StoreParseResult *result = StoreAllocateMemoryType(StoreParseResult);

	// Try to parse the store as a value
	StoreDynamicString valueStoreError = StoreCreateDynamicStringType();
	Store *valueStore = parseValue(input, &state);
	if(valueStore != NULL) {
		result->status = STORE_PARSE_SUCCESS;
		result->store = valueStore;
	} else {
		result->status = STORE_PARSE_ERROR;
	}

	StoreFreeDynamicStringType(state.error);

	return result;
}

/**
 * value	: string
 * 			| int
 * 			| float
 * 			| list
 * 			| struct
 */
static Store *parseValue(const char *input, ParseState *state)
{
	ParseState valueState;
	valueState.error = StoreCreateDynamicStringType();
	valueState.level = state->level + 1;

	valueState.position = state->position;
	Store *stringStore = parseString(input, &valueState);
	if(stringStore != NULL) {
		StoreFreeDynamicStringType(valueState.error);
		return stringStore;
	}

	valueState.position = state->position;
	Store *intStore = parseInt(input, &valueState);
	if(intStore != NULL) {
		StoreFreeDynamicStringType(valueState.error);
		return intStore;
	}

	valueState.position = state->position;
	Store *floatStore = parseFloat(input, &valueState);
	if(floatStore != NULL) {
		StoreFreeDynamicStringType(valueState.error);
		return floatStore;
	}

	valueState.position = state->position;
	Store *listStore = parseList(input, &valueState);
	if(listStore != NULL) {
		StoreFreeDynamicStringType(valueState.error);
		return listStore;
	}

	valueState.position = state->position;
	Store *structStore = parseStruct(input, &valueState);
	if(structStore != NULL) {
		StoreFreeDynamicStringType(valueState.error);
		return structStore;
	}

	appendParseError(state, "value", "%s", StoreReadDynamicString(valueState.error));
	StoreFreeDynamicStringType(valueState.error);

	return NULL;
}

static Store *parseString(const char *input, ParseState *state)
{

}

static Store *parseInt(const char *input, ParseState *state)
{

}

static Store *parseFloat(const char *input, ParseState *state)
{

}

static Store *parseList(const char *input, ParseState *state)
{

}

/**
 * struct	: '{' fields '}'
 */
static Store *parseStruct(const char *input, ParseState *state)
{
	ParseState structState;
	structState.position = state->position;
	structState.error = StoreCreateDynamicStringType();
	structState.level = state->level + 1;

	char c = parseNextNonWhitespace(input, &structState);
	if(c == '\0') {
		appendParseError(state, "struct", "%s", StoreReadDynamicString(structState.error));
		StoreFreeDynamicStringType(structState.error);
		return NULL;
	} else if(c != '{') {
		appendParseError(state, "struct", "opening character must be '{'");
		StoreFreeDynamicStringType(structState.error);
		return NULL;
	}

	Store *structStore = parseFields(input, &structState);
	if(structStore == NULL) {
		appendParseError(state, "struct", "%s", StoreReadDynamicString(structState.error));
		StoreFreeDynamicStringType(structState.error);
		return NULL;
	}

	c = parseNextNonWhitespace(input, &structState);
	if(c == '\0') {
		appendParseError(state, "struct", "%s", StoreReadDynamicString(structState.error));
		StoreFreeDynamicStringType(structState.error);
		StoreFree(structStore);
		return NULL;
	} else if(c != '}') {
		appendParseError(state, "struct", "ending character must be '}'");
		StoreFreeDynamicStringType(structState.error);
		StoreFree(structStore);
		return NULL;
	}

	state->position = structState.position;
	return structStore;
}

static Store *parseFields(const char *input, ParseState *state)
{

}

static char parseNextNonWhitespace(const char *input, ParseState *state)
{
	while(true) {
		char c = input[state->position.index];
		state->position.index++;
		state->position.column++;

		if(c == '\0') {
			appendParseError(state, "non-whitespace", "premature end of input");
			return c;
		} else if(!isspace(c)) {
			return c;
		} else if(c == '\n') {
			state->position.line++;
			state->position.column = 1;
		}
	}

	return '\0';
}

static void appendParseError(ParseState *state, const char *what, const char *reason, ...)
{
	va_list va;
	va_start(va, reason);

	for(int i = 0; i < state->level; i++) {
		StoreAppendDynamicString(state->error, "\t");
	}
	StoreAppendDynamicString(state->error, "failed to parse %s at line %d column %d:\n", what, state->position.line, state->position.column);
	StoreAppendDynamicStringV(state->error, reason, va);
}
