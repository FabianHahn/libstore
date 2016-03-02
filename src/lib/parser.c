#include <ctype.h> // isspace isdigit
#include <stdarg.h> // va_list va_start
#include <stdbool.h> // bool true false
#include <stddef.h> // NULL
#include <stdlib.h> // atoi
#include <string.h> // strdup

#include "memory.h"
#include "parser.h"

typedef struct {
	const char *key;
	Store *value;
} Field;

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
static Store *parseElements(const char *input, ParseState *state);
static Store *parseStruct(const char *input, ParseState *state);
static Store *parseFields(const char *input, ParseState *state);
static Field *parseField(const char *input, ParseState *state);
static char parseNextNonWhitespace(const char *input, ParseState *state);
static char parseDigit(const char *input, ParseState *state);
static char parseDelimiter(const char *input, ParseState *state);
static void appendParseError(ParseState *state, const char *what, const char *reason, ...);

StoreParseResult *StoreParse(const char *input)
{
	ParseState state;
	state.position.index = 0;
	state.position.line = 1;
	state.position.column = 1;
	state.error = StoreCreateDynamicString();
	state.level = 0;

	StoreParseResult *result = StoreAllocateMemoryType(StoreParseResult);

	// Try to parse the store as a value
	StoreDynamicString valueStoreError = StoreCreateDynamicString();
	Store *valueStore = parseValue(input, &state);
	if(valueStore != NULL) {
		result->status = STORE_PARSE_SUCCESS;
		result->store = valueStore;
	} else {
		result->status = STORE_PARSE_ERROR;
	}

	StoreFreeDynamicString(state.error);

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
	valueState.error = StoreCreateDynamicString();
	valueState.level = state->level + 1;

	valueState.position = state->position;
	Store *stringStore = parseString(input, &valueState);
	if(stringStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		return stringStore;
	}

	valueState.position = state->position;
	Store *intStore = parseInt(input, &valueState);
	if(intStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		return intStore;
	}

	valueState.position = state->position;
	Store *floatStore = parseFloat(input, &valueState);
	if(floatStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		return floatStore;
	}

	valueState.position = state->position;
	Store *listStore = parseList(input, &valueState);
	if(listStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		return listStore;
	}

	valueState.position = state->position;
	Store *structStore = parseStruct(input, &valueState);
	if(structStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		return structStore;
	}

	appendParseError(state, "value", "%s", StoreReadDynamicString(valueState.error));
	StoreFreeDynamicString(valueState.error);

	return NULL;
}

static Store *parseString(const char *input, ParseState *state)
{

}

/**
 * int	: '-'? digit+ delimiter
 */
static Store *parseInt(const char *input, ParseState *state)
{
	ParseState intState;
	intState.position = state->position;
	intState.error = StoreCreateDynamicString();
	intState.level = state->level + 1;

	StoreDynamicString intString = StoreCreateDynamicString();

	char c = parseNextNonWhitespace(input, &intState);
	if(c == '\0') {
		appendParseError(state, "int", "%s", StoreReadDynamicString(intState.error));
		StoreFreeDynamicString(intState.error);
		StoreFreeDynamicString(intString);
		return NULL;
	} else if(c == '-') {
		StoreAppendDynamicString(intString, "-");
	} else {
		intState.position = state->position; // reset position to reread that character
	}

	while(true) {
		char d = parseDigit(input, &intState);
		if(d == '\0') {
			intState.position = state->position; // reset position to reread that character
			break;
		}

		StoreAppendDynamicString(intString, "%c", d);
	}

	c = parseDelimiter(input, &intState);
	if(c == '\0') {
		appendParseError(state, "int", "%s", StoreReadDynamicString(intState.error));
		StoreFreeDynamicString(intState.error);
		StoreFreeDynamicString(intString);
		return NULL;
	}

	int intValue = atoi(StoreReadDynamicString(intString));
	return StoreCreateIntValue(intValue);
}

static Store *parseFloat(const char *input, ParseState *state)
{

}

/**
 * list	: '(' elements ')'
 */
static Store *parseList(const char *input, ParseState *state)
{
	ParseState listState;
	listState.position = state->position;
	listState.error = StoreCreateDynamicString();
	listState.level = state->level + 1;

	char c = parseNextNonWhitespace(input, &listState);
	if(c == '\0') {
		appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
		StoreFreeDynamicString(listState.error);
		return NULL;
	} else if(c != '{') {
		appendParseError(state, "list", "opening character must be '('");
		StoreFreeDynamicString(listState.error);
		return NULL;
	}

	Store *listStore = parseElements(input, &listState);
	if(listStore == NULL) {
		appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
		StoreFreeDynamicString(listState.error);
		return NULL;
	}

	c = parseNextNonWhitespace(input, &listState);
	if(c == '\0') {
		appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
		StoreFreeDynamicString(listState.error);
		StoreFree(listStore);
		return NULL;
	} else if(c != '}') {
		appendParseError(state, "list", "ending character must be ')'");
		StoreFreeDynamicString(listState.error);
		StoreFree(listStore);
		return NULL;
	}

	state->position = listState.position;
	return listStore;
}

/**
 * elements	: value*
 */
static Store *parseElements(const char *input, ParseState *state)
{
	Store *listStore = StoreCreateListValue();
	while(true) {
		Store *valueStore = parseValue(input, state);
		if(valueStore == NULL) {
			break;
		}

		StoreAppendList(listStore->content.listValue, valueStore);
	}

	return listStore;
}

/**
 * struct	: '{' fields '}'
 */
static Store *parseStruct(const char *input, ParseState *state)
{
	ParseState structState;
	structState.position = state->position;
	structState.error = StoreCreateDynamicString();
	structState.level = state->level + 1;

	char c = parseNextNonWhitespace(input, &structState);
	if(c == '\0') {
		appendParseError(state, "struct", "%s", StoreReadDynamicString(structState.error));
		StoreFreeDynamicString(structState.error);
		return NULL;
	} else if(c != '{') {
		appendParseError(state, "struct", "opening character must be '{'");
		StoreFreeDynamicString(structState.error);
		return NULL;
	}

	Store *structStore = parseFields(input, &structState);
	if(structStore == NULL) {
		appendParseError(state, "struct", "%s", StoreReadDynamicString(structState.error));
		StoreFreeDynamicString(structState.error);
		return NULL;
	}

	c = parseNextNonWhitespace(input, &structState);
	if(c == '\0') {
		appendParseError(state, "struct", "%s", StoreReadDynamicString(structState.error));
		StoreFreeDynamicString(structState.error);
		StoreFree(structStore);
		return NULL;
	} else if(c != '}') {
		appendParseError(state, "struct", "ending character must be '}'");
		StoreFreeDynamicString(structState.error);
		StoreFree(structStore);
		return NULL;
	}

	state->position = structState.position;
	return structStore;
}

/**
 * fields	: field*
 */
static Store *parseFields(const char *input, ParseState *state)
{
	Store *structStore = StoreCreateStructValue();
	while(true) {
		Field *field = parseField(input, state);
		if(field == NULL) {
			break;
		}

		StoreInsertMap(structStore->content.structValue, field->key, field->value);
		StoreFreeMemory(field);
	}

	return structStore;
}

/**
 * field	: string '=' value
 * 			| string ':' value
 */
static Field *parseField(const char *input, ParseState *state)
{
	ParseState fieldState;
	fieldState.position = state->position;
	fieldState.error = StoreCreateDynamicString();
	fieldState.level = state->level + 1;

	Store *stringStore = parseString(input, &fieldState);
	if(stringStore == NULL) {
		appendParseError(state, "field", "%s", StoreReadDynamicString(fieldState.error));
		return NULL;
	}

	char c = parseNextNonWhitespace(input, &fieldState);
	if(c == '\0') {
		appendParseError(state, "field", "%s", StoreReadDynamicString(fieldState.error));
		StoreFreeDynamicString(fieldState.error);
		StoreFree(stringStore);
		return NULL;
	} else if(c != ':' && c != '=') {
		appendParseError(state, "field", "field separating character must be ':' or '='");
		StoreFreeDynamicString(fieldState.error);
		StoreFree(stringStore);
		return NULL;
	}

	Store *valueStore = parseValue(input, &fieldState);
	if(valueStore == NULL) {
		appendParseError(state, "field", "%s", StoreReadDynamicString(fieldState.error));
		StoreFreeDynamicString(fieldState.error);
		StoreFree(stringStore);
		return NULL;
	}

	Field *field = StoreAllocateMemoryType(Field);
	field->key = strdup(stringStore->content.stringValue);
	field->value = valueStore;

	StoreFree(stringStore);
	return field;
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

static char parseDigit(const char *input, ParseState *state)
{
	char c = input[state->position.index];
	state->position.index++;
	state->position.column++;

	if(isdigit(c)) {
		return c;
	}

	appendParseError(state, "digit", "encountered non-digit '%c'", c);
	return '\0';
}

static char parseDelimiter(const char *input, ParseState *state)
{
	char c = input[state->position.index];
	state->position.index++;
	state->position.column++;

	if(isspace(c) || c == ',' || c == ';') {
		return c;
	}

	appendParseError(state, "delimiter", "encountered non-delimiter '%c'", c);
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
