#include <ctype.h> // isspace isdigit
#include <stdarg.h> // va_list va_start
#include <stdbool.h> // bool true false
#include <stddef.h> // NULL
#include <stdlib.h> // atoi atof
#include <string.h> // strdup

#include "memory.h"
#include "parser.h"

typedef struct {
	const char *key;
	Store *value;
} Entry;

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
static Store *parseMap(const char *input, ParseState *state);
static Store *parseEntries(const char *input, ParseState *state);
static Entry *parseEntry(const char *input, ParseState *state);
static StoreDynamicString parseDigits(const char *input, ParseState *state);
static StoreDynamicString parseFloating(const char *input, ParseState *state);
static StoreDynamicString parseExponential(const char *input, ParseState *state);
static char parseDigit(const char *input, ParseState *state);
static char parseDelimiter(const char *input, ParseState *state);
static char parseNonWhitespace(const char *input, ParseState *state);
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
	Store *mapStore = parseMap(input, &valueState);
	if(mapStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		return mapStore;
	}

	appendParseError(state, "value", "%s", StoreReadDynamicString(valueState.error));
	StoreFreeDynamicString(valueState.error);
	state->position = valueState.position;
	return NULL;
}

static Store *parseString(const char *input, ParseState *state)
{

}

/**
 * int	: '-'? digits
 */
static Store *parseInt(const char *input, ParseState *state)
{
	ParseState intState;
	intState.position = state->position;
	intState.error = StoreCreateDynamicString();
	intState.level = state->level + 1;

	StoreDynamicString intString = StoreCreateDynamicString();

	char c = parseNonWhitespace(input, &intState);
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

	StoreDynamicString digitsString = parseDigits(input, &intState);
	if(digitsString == NULL) {
		appendParseError(state, "int", "%s", StoreReadDynamicString(intState.error));
		StoreFreeDynamicString(intState.error);
		StoreFreeDynamicString(intString);
		return NULL;
	}

	StoreAppendDynamicString(intString, "%s", StoreReadDynamicString(digitsString));
	StoreFreeDynamicString(digitsString);

	StoreFreeDynamicString(intState.error);
	state->position = intState.position;
	int intValue = atoi(StoreReadDynamicString(intString));
	return StoreCreateIntValue(intValue);
}

/**
 * float	: '-'? digits floating? exponential?
 */
static Store *parseFloat(const char *input, ParseState *state)
{
	ParseState floatState;
	floatState.position = state->position;
	floatState.error = StoreCreateDynamicString();
	floatState.level = state->level + 1;

	StoreDynamicString floatString = StoreCreateDynamicString();

	char c = parseNonWhitespace(input, &floatState);
	if(c == '\0') {
		appendParseError(state, "float", "%s", StoreReadDynamicString(floatState.error));
		StoreFreeDynamicString(floatState.error);
		StoreFreeDynamicString(floatString);
		return NULL;
	} else if(c == '-') {
		StoreAppendDynamicString(floatString, "-");
	} else {
		floatState.position = state->position; // reset position to reread that character
	}

	StoreDynamicString digitsString = parseDigits(input, &floatState);
	if(digitsString == NULL) {
		appendParseError(state, "float", "%s", StoreReadDynamicString(floatState.error));
		StoreFreeDynamicString(floatState.error);
		StoreFreeDynamicString(floatString);
		return NULL;
	}

	StoreAppendDynamicString(floatString, "%s", StoreReadDynamicString(digitsString));
	StoreFreeDynamicString(digitsString);

	StoreDynamicString floatingString = parseFloating(input, &floatState);
	if(floatingString != NULL) {
		StoreAppendDynamicString(floatString, "%s", StoreReadDynamicString(floatingString));
		StoreFreeDynamicString(floatingString);
	}

	StoreDynamicString exponentialString = parseExponential(input, &floatState);
	if(exponentialString != NULL) {
		StoreAppendDynamicString(floatString, "%s", StoreReadDynamicString(exponentialString));
		StoreFreeDynamicString(exponentialString);
	}

	StoreFreeDynamicString(floatState.error);
	state->position = floatState.position;
	double floatValue = atof(StoreReadDynamicString(floatString));
	return StoreCreateFloatValue(floatValue);
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

	char c = parseNonWhitespace(input, &listState);
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

	c = parseNonWhitespace(input, &listState);
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

	StoreFreeDynamicString(listState.error);
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
 * map	: '{' entries '}'
 */
static Store *parseMap(const char *input, ParseState *state)
{
	ParseState mapState;
	mapState.position = state->position;
	mapState.error = StoreCreateDynamicString();
	mapState.level = state->level + 1;

	char c = parseNonWhitespace(input, &mapState);
	if(c == '\0') {
		appendParseError(state, "map", "%s", StoreReadDynamicString(mapState.error));
		StoreFreeDynamicString(mapState.error);
		return NULL;
	} else if(c != '{') {
		appendParseError(state, "map", "opening character must be '{'");
		StoreFreeDynamicString(mapState.error);
		return NULL;
	}

	Store *mapStore = parseEntries(input, &mapState);
	if(mapStore == NULL) {
		appendParseError(state, "map", "%s", StoreReadDynamicString(mapState.error));
		StoreFreeDynamicString(mapState.error);
		return NULL;
	}

	c = parseNonWhitespace(input, &mapState);
	if(c == '\0') {
		appendParseError(state, "map", "%s", StoreReadDynamicString(mapState.error));
		StoreFreeDynamicString(mapState.error);
		StoreFree(mapStore);
		return NULL;
	} else if(c != '}') {
		appendParseError(state, "map", "ending character must be '}'");
		StoreFreeDynamicString(mapState.error);
		StoreFree(mapStore);
		return NULL;
	}

	StoreFreeDynamicString(mapState.error);
	state->position = mapState.position;
	return mapStore;
}

/**
 * entries	: entry*
 */
static Store *parseEntries(const char *input, ParseState *state)
{
	Store *mapStore = StoreCreateMapValue();
	while(true) {
		Entry *entry = parseEntry(input, state);
		if(entry == NULL) {
			break;
		}

		StoreInsertMap(mapStore->content.mapValue, entry->key, entry->value);
		StoreFreeMemory(entry);
	}

	return mapStore;
}

/**
 * entry	: string '=' value
 * 			| string ':' value
 */
static Entry *parseEntry(const char *input, ParseState *state)
{
	ParseState entryState;
	entryState.position = state->position;
	entryState.error = StoreCreateDynamicString();
	entryState.level = state->level + 1;

	Store *stringStore = parseString(input, &entryState);
	if(stringStore == NULL) {
		appendParseError(state, "entry", "%s", StoreReadDynamicString(entryState.error));
		return NULL;
	}

	char c = parseNonWhitespace(input, &entryState);
	if(c == '\0') {
		appendParseError(state, "entry", "%s", StoreReadDynamicString(entryState.error));
		StoreFreeDynamicString(entryState.error);
		StoreFree(stringStore);
		return NULL;
	} else if(c != ':' && c != '=') {
		appendParseError(state, "entry", "entry separating character must be ':' or '='");
		StoreFreeDynamicString(entryState.error);
		StoreFree(stringStore);
		return NULL;
	}

	Store *valueStore = parseValue(input, &entryState);
	if(valueStore == NULL) {
		appendParseError(state, "entry", "%s", StoreReadDynamicString(entryState.error));
		StoreFreeDynamicString(entryState.error);
		StoreFree(stringStore);
		return NULL;
	}

	Entry *entry = StoreAllocateMemoryType(Entry);
	entry->key = strdup(stringStore->content.stringValue);
	entry->value = valueStore;

	StoreFree(stringStore);
	return entry;
}

/**
 * digits	: digit+
 */
static StoreDynamicString parseDigits(const char *input, ParseState *state)
{
	ParseState digitsState;
	digitsState.position = state->position;
	digitsState.error = StoreCreateDynamicString();
	digitsState.level = state->level + 1;

	StoreDynamicString digitsString = StoreCreateDynamicString();

	int len = 0;
	while(true) {
		char d = parseDigit(input, &digitsState);
		if(d == '\0') {
			break;
		}

		StoreAppendDynamicString(digitsString, "%c", d);
		len++;
	}

	if(len == 0) {
		appendParseError(state, "digits", "failed to parse any digits");
		StoreFreeDynamicString(digitsState.error);
		StoreFreeDynamicString(digitsString);
		return NULL;
	}

	StoreFreeDynamicString(digitsState.error);
	state->position = digitsState.position;
	return digitsString;
}

/**
 * floating	: '.' digits
 */
static StoreDynamicString parseFloating(const char *input, ParseState *state)
{
	ParseState floatingState;
	floatingState.position = state->position;
	floatingState.error = StoreCreateDynamicString();
	floatingState.level = state->level + 1;

	StoreDynamicString floatingString = StoreCreateDynamicString();

	char c = parseNonWhitespace(input, &floatingState);
	if(c == '\0') {
		appendParseError(state, "floating", "%s", StoreReadDynamicString(floatingState.error));
		StoreFreeDynamicString(floatingState.error);
		StoreFreeDynamicString(floatingString);
		return NULL;
	} else if(c == '.') {
		StoreAppendDynamicString(floatingString, ".");
	} else {
		floatingState.position = state->position; // reset position to reread that character
	}

	StoreDynamicString digitsString = parseDigits(input, &floatingState);
	if(digitsString == NULL) {
		appendParseError(state, "floating", "%s", StoreReadDynamicString(floatingState.error));
		StoreFreeDynamicString(floatingState.error);
		StoreFreeDynamicString(floatingString);
		return NULL;
	}

	StoreFreeDynamicString(floatingState.error);
	state->position = floatingState.position;
	return floatingString;
}

/**
 * exponential : ('e'|'E') ('+'|'-')? digits
 */
static StoreDynamicString parseExponential(const char *input, ParseState *state)
{
	ParseState exponentialState;
	exponentialState.position = state->position;
	exponentialState.error = StoreCreateDynamicString();
	exponentialState.level = state->level + 1;

	StoreDynamicString exponentialString = StoreCreateDynamicString();

	char c = parseNonWhitespace(input, &exponentialState);
	if(c == '\0') {
		appendParseError(state, "exponential", "%s", StoreReadDynamicString(exponentialState.error));
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	} else if(c == 'e' || c == 'E') {
		StoreAppendDynamicString(exponentialString, "%c", c);
	} else {
		appendParseError(state, "exponential", "expected 'e' or 'E'");
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	}

	c = parseNonWhitespace(input, &exponentialState);
	if(c == '\0') {
		appendParseError(state, "exponential", "%s", StoreReadDynamicString(exponentialState.error));
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	} else if(c == '+' || c == '-') {
		StoreAppendDynamicString(exponentialString, "%c", c);
	} else {
		exponentialState.position = state->position; // reset position to reread that character
	}

	StoreDynamicString digitsString = parseDigits(input, &exponentialState);
	if(digitsString == NULL) {
		appendParseError(state, "exponential", "%s", StoreReadDynamicString(exponentialState.error));
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	}

	StoreFreeDynamicString(exponentialState.error);
	state->position = exponentialState.position;
	return exponentialString;
}

static char parseDigit(const char *input, ParseState *state)
{
	char c = input[state->position.index];

	if(isdigit(c)) {
		state->position.index++;
		state->position.column++;
		return c;
	}

	appendParseError(state, "digit", "encountered non-digit '%c'", c);
	return '\0';
}

static char parseDelimiter(const char *input, ParseState *state)
{
	char c = input[state->position.index];

	if(isspace(c) || c == ',' || c == ';') {
		state->position.index++;
		state->position.column++;

		if(c == '\n') {
			state->position.line++;
			state->position.column = 1;
		}

		return c;
	}

	appendParseError(state, "delimiter", "encountered non-delimiter '%c'", c);
	return '\0';
}

static char parseNonWhitespace(const char *input, ParseState *state)
{
	char c = input[state->position.index];

	if(!isspace(c)) {
		state->position.index++;
		state->position.column++;
		return c;
	}

	appendParseError(state, "non-whitespace", "encountered whitespace character");
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
