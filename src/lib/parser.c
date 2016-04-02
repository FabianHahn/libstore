#include <ctype.h> // isspace isdigit
#include <stdarg.h> // va_list va_start
#include <stdbool.h> // bool true false
#include <stddef.h> // NULL
#include <stdlib.h> // atoi atof
#include <string.h> // strdup

#include "encoding.h"
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
static StoreDynamicString parseShortString(const char *input, ParseState *state);
static StoreDynamicString parseLongString(const char *input, ParseState *state);
static StoreDynamicString parseLongStringChar(const char *input, ParseState *state);
static char parseTerminal(const char *input, ParseState *state);
static char parseShortStringChar(const char *input, ParseState *state);
static char parseHex(const char *input, ParseState *state);
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
 * value	: int
 * 			| float
 * 			| string
 * 			| list
 * 			| struct
 */
static Store *parseValue(const char *input, ParseState *state)
{
	ParseState valueState;
	valueState.error = StoreCreateDynamicString();
	valueState.level = state->level + 1;

	valueState.position = state->position;
	Store *intStore = parseInt(input, &valueState);
	if(intStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		state->position = valueState.position;
		return intStore;
	}

	valueState.position = state->position;
	Store *floatStore = parseFloat(input, &valueState);
	if(floatStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		state->position = valueState.position;
		return floatStore;
	}

	valueState.position = state->position;
	Store *stringStore = parseString(input, &valueState);
	if(stringStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		state->position = valueState.position;
		return stringStore;
	}

	valueState.position = state->position;
	Store *listStore = parseList(input, &valueState);
	if(listStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		state->position = valueState.position;
		return listStore;
	}

	valueState.position = state->position;
	Store *mapStore = parseMap(input, &valueState);
	if(mapStore != NULL) {
		StoreFreeDynamicString(valueState.error);
		state->position = valueState.position;
		return mapStore;
	}

	appendParseError(state, "value", "%s", StoreReadDynamicString(valueState.error));
	StoreFreeDynamicString(valueState.error);
	return NULL;
}

/**
 * string	: simplestring
 * 			| '"' longstring '"'
 */
static Store *parseString(const char *input, ParseState *state)
{
	ParseState stringState;
	stringState.position = state->position;
	stringState.error = StoreCreateDynamicString();
	stringState.level = state->level + 1;

	Store *stringStore = NULL;

	char c = parseTerminal(input, &stringState);
	if(c == '\0') {
		appendParseError(state, "string", "%s", StoreReadDynamicString(stringState.error));
		StoreFreeDynamicString(stringState.error);
		return NULL;
	} else if(c == '"') {
		// eat that character
		stringState.position.index++;
		stringState.position.column++;

		StoreDynamicString longString = parseLongString(input, &stringState);
		if(longString == NULL) {
			appendParseError(state, "string", "%s", StoreReadDynamicString(stringState.error));
			StoreFreeDynamicString(stringState.error);
			return NULL;
		}

		c = input[stringState.position.index];
		if(c != '"') {
			appendParseError(state, "string", "expected '\"' delimiter after longstring");
			StoreFreeDynamicString(stringState.error);
			StoreFreeDynamicString(longString);
			return NULL;
		}

		// eat that character
		stringState.position.index++;
		stringState.position.column++;

		stringStore = StoreCreateStringValue(StoreReadDynamicString(longString));
		StoreFreeDynamicString(longString);
	} else {
		StoreDynamicString shortString = parseShortString(input, &stringState);
		if(shortString == NULL) {
			appendParseError(state, "string", "%s", StoreReadDynamicString(stringState.error));
			StoreFreeDynamicString(stringState.error);
			return NULL;
		}

		stringStore = StoreCreateStringValue(StoreReadDynamicString(shortString));
		StoreFreeDynamicString(shortString);
	}

	StoreFreeDynamicString(stringState.error);
	state->position = stringState.position;
	return stringStore;
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

	char c = parseTerminal(input, &intState);
	if(c == '\0') {
		appendParseError(state, "int", "%s", StoreReadDynamicString(intState.error));
		StoreFreeDynamicString(intState.error);
		StoreFreeDynamicString(intString);
		return NULL;
	} else if(c == '-') {
		StoreAppendDynamicString(intString, "-");

		// eat that character
		intState.position.index++;
		intState.position.column++;
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

	char c = parseTerminal(input, &floatState);
	if(c == '\0') {
		appendParseError(state, "float", "%s", StoreReadDynamicString(floatState.error));
		StoreFreeDynamicString(floatState.error);
		StoreFreeDynamicString(floatString);
		return NULL;
	} else if(c == '-') {
		StoreAppendDynamicString(floatString, "-");

		// eat that character
		floatState.position.index++;
		floatState.position.column++;
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
 * 		| '[' elements ']'
 */
static Store *parseList(const char *input, ParseState *state)
{
	ParseState listState;
	listState.position = state->position;
	listState.error = StoreCreateDynamicString();
	listState.level = state->level + 1;

	Store *listStore = NULL;

	char c = parseTerminal(input, &listState);
	if(c == '\0') {
		appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
		StoreFreeDynamicString(listState.error);
		return NULL;
	} else if(c == '(') {
		// eat that character
		listState.position.index++;
		listState.position.column++;

		listStore = parseElements(input, &listState);
		if(listStore == NULL) {
			appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
			StoreFreeDynamicString(listState.error);
			return NULL;
		}

		c = parseTerminal(input, &listState);
		if(c == '\0') {
			appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
			StoreFreeDynamicString(listState.error);
			StoreFree(listStore);
			return NULL;
		} else if(c != ')') {
			appendParseError(state, "list", "ending character must be ')'");
			StoreFreeDynamicString(listState.error);
			StoreFree(listStore);
			return NULL;
		}

		// eat that character
		listState.position.index++;
		listState.position.column++;
	} else if(c == '[') {
		// eat that character
		listState.position.index++;
		listState.position.column++;

		listStore = parseElements(input, &listState);
		if(listStore == NULL) {
			appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
			StoreFreeDynamicString(listState.error);
			return NULL;
		}

		c = parseTerminal(input, &listState);
		if(c == '\0') {
			appendParseError(state, "list", "%s", StoreReadDynamicString(listState.error));
			StoreFreeDynamicString(listState.error);
			StoreFree(listStore);
			return NULL;
		} else if(c != ']') {
			appendParseError(state, "list", "ending character must be ']'");
			StoreFreeDynamicString(listState.error);
			StoreFree(listStore);
			return NULL;
		}

		// eat that character
		listState.position.index++;
		listState.position.column++;
	} else {
		appendParseError(state, "list", "opening character must be '(' or '['");
		StoreFreeDynamicString(listState.error);
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
	ParseState elementsState;
	elementsState.position = state->position;
	elementsState.error = StoreCreateDynamicString();
	elementsState.level = state->level + 1;

	Store *listStore = StoreCreateListValue();
	while(true) {
		Store *valueStore = parseValue(input, &elementsState);
		if(valueStore == NULL) {
			break;
		}

		StoreAppendList(listStore->content.listValue, valueStore);
	}

	StoreFreeDynamicString(elementsState.error);
	state->position = elementsState.position;
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

	char c = parseTerminal(input, &mapState);
	if(c == '\0') {
		appendParseError(state, "map", "%s", StoreReadDynamicString(mapState.error));
		StoreFreeDynamicString(mapState.error);
		return NULL;
	} else if(c != '{') {
		appendParseError(state, "map", "opening character must be '{'");
		StoreFreeDynamicString(mapState.error);
		return NULL;
	}

	// eat that character
	mapState.position.index++;
	mapState.position.column++;

	Store *mapStore = parseEntries(input, &mapState);
	if(mapStore == NULL) {
		appendParseError(state, "map", "%s", StoreReadDynamicString(mapState.error));
		StoreFreeDynamicString(mapState.error);
		return NULL;
	}

	c = parseTerminal(input, &mapState);
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

	// eat that character
	mapState.position.index++;
	mapState.position.column++;

	StoreFreeDynamicString(mapState.error);
	state->position = mapState.position;
	return mapStore;
}

/**
 * entries	: entry*
 */
static Store *parseEntries(const char *input, ParseState *state)
{
	ParseState entriesState;
	entriesState.position = state->position;
	entriesState.error = StoreCreateDynamicString();
	entriesState.level = state->level + 1;

	Store *entriesStore = StoreCreateMapValue();
	while(true) {
		Entry *entry = parseEntry(input, &entriesState);
		if(entry == NULL) {
			break;
		}

		StoreInsertMap(entriesStore->content.mapValue, entry->key, entry->value);
		StoreFreeMemory(entry);
	}

	StoreFreeDynamicString(entriesState.error);
	state->position = entriesState.position;
	return entriesStore;
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

	char c = parseTerminal(input, &entryState);
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

	// eat that character
	entryState.position.index++;
	entryState.position.column++;

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

	StoreFreeDynamicString(entryState.error);
	state->position = entryState.position;
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
 * floating	: '.' digits?
 */
static StoreDynamicString parseFloating(const char *input, ParseState *state)
{
	ParseState floatingState;
	floatingState.position = state->position;
	floatingState.error = StoreCreateDynamicString();
	floatingState.level = state->level + 1;

	StoreDynamicString floatingString = StoreCreateDynamicString();

	char c = input[floatingState.position.index];
	if(c == '\0') {
		appendParseError(state, "floating", "unexpected end of input");
		StoreFreeDynamicString(floatingState.error);
		StoreFreeDynamicString(floatingString);
		return NULL;
	} else if(c == '.') {
		StoreAppendDynamicString(floatingString, ".");

		// eat that character
		floatingState.position.index++;
		floatingState.position.column++;
	}

	StoreDynamicString digitsString = parseDigits(input, &floatingState);
	if(digitsString != NULL) {
		StoreAppendDynamicString(floatingString, "%s", StoreReadDynamicString(digitsString));
		StoreFreeDynamicString(digitsString);
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

	char c = input[exponentialState.position.index];
	if(c == '\0') {
		appendParseError(state, "exponential", "%s", StoreReadDynamicString(exponentialState.error));
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	} else if(c != 'e' && c != 'E') {
		appendParseError(state, "exponential", "expected 'e' or 'E'");
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	}

	StoreAppendDynamicString(exponentialString, "%c", c);

	// eat that character
	exponentialState.position.index++;
	exponentialState.position.column++;

	c = input[exponentialState.position.index];
	if(c == '\0') {
		appendParseError(state, "exponential", "unexpected end of input");
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	} else if(c == '+' || c == '-') {
		StoreAppendDynamicString(exponentialString, "%c", c);

		// eat that character
		exponentialState.position.index++;
		exponentialState.position.column++;
	}

	StoreDynamicString digitsString = parseDigits(input, &exponentialState);
	if(digitsString == NULL) {
		appendParseError(state, "exponential", "%s", StoreReadDynamicString(exponentialState.error));
		StoreFreeDynamicString(exponentialState.error);
		StoreFreeDynamicString(exponentialString);
		return NULL;
	}

	StoreAppendDynamicString(exponentialString, "%s", StoreReadDynamicString(digitsString));
	StoreFreeDynamicString(digitsString);

	StoreFreeDynamicString(exponentialState.error);
	state->position = exponentialState.position;
	return exponentialString;
}

/**
 * shortstring	: shortstringchar+
 */
static StoreDynamicString parseShortString(const char *input, ParseState *state)
{
	ParseState shortStringState;
	shortStringState.position = state->position;
	shortStringState.error = StoreCreateDynamicString();
	shortStringState.level = state->level + 1;

	char c = parseShortStringChar(input, &shortStringState);
	if(c == '\0') {
		appendParseError(state, "shortstring", "%s", StoreReadDynamicString(shortStringState.error));
		StoreFreeDynamicString(shortStringState.error);
		return NULL;
	}

	StoreDynamicString shortString = StoreCreateDynamicString();
	do {
		StoreAppendDynamicString(shortString, "%c", c);
		c = parseShortStringChar(input, &shortStringState);
	} while(c != '\0');

	StoreFreeDynamicString(shortStringState.error);
	state->position = shortStringState.position;
	return shortString;
}

/**
 * longstring	: longstringchar*
 */
static StoreDynamicString parseLongString(const char *input, ParseState *state)
{
	ParseState longStringState;
	longStringState.position = state->position;
	longStringState.error = StoreCreateDynamicString();
	longStringState.level = state->level + 1;

	StoreDynamicString longString = StoreCreateDynamicString();
	while(true) {
		StoreDynamicString longStringChar = parseLongStringChar(input, &longStringState);
		if(longStringChar == NULL) {
			break;
		}

		StoreAppendDynamicString(longString, "%s", StoreReadDynamicString(longStringChar));
		StoreFreeDynamicString(longStringChar);
	}

	StoreFreeDynamicString(longStringState.error);
	state->position = longStringState.position;
	return longString;
}

/**
 * longstringchar	: nonspecial+
 * 					| '\' escaped
 * 					| '\' 'u' hex hex hex hex
 */
static StoreDynamicString parseLongStringChar(const char *input, ParseState *state)
{
	StoreDynamicString longStringChar = StoreCreateDynamicString();

	char c = input[state->position.index];

	if(c == '\\') {
		state->position.index++;
		state->position.column++;

		c = input[state->position.index];
		state->position.index++;
		state->position.column++;

		switch(c) {
			case '"':
				StoreAppendDynamicString(longStringChar, "\"");
			break;
			case '\\':
				StoreAppendDynamicString(longStringChar, "\\");
			break;
			case '/':
				StoreAppendDynamicString(longStringChar, "/");
			break;
			case 'b':
				StoreAppendDynamicString(longStringChar, "\b");
			break;
			case 'f':
				StoreAppendDynamicString(longStringChar, "\f");
			break;
			case 'n':
				StoreAppendDynamicString(longStringChar, "\n");
			break;
			case 'r':
				StoreAppendDynamicString(longStringChar, "\r");
			break;
			case 't':
				StoreAppendDynamicString(longStringChar, "\t");
			break;
			case 'u':
			{
				char u1 = parseHex(input, state);
				if(u1 == '\0') {
					state->position.index -= 2;
					state->position.column -= 2;
					appendParseError(state, "longstringchar", "expected first hex number of escaped unicode character");
					StoreFreeDynamicString(longStringChar);
					return NULL;
				}

				char u2 = parseHex(input, state);
				if(u2 == '\0') {
					state->position.index -= 3;
					state->position.column -= 3;
					appendParseError(state, "longstringchar", "expected second hex number of escaped unicode character");
					StoreFreeDynamicString(longStringChar);
					return NULL;
				}

				char u3 = parseHex(input, state);
				if(u3 == '\0') {
					state->position.index -= 4;
					state->position.column -= 4;
					appendParseError(state, "longstringchar", "expected third hex number of escaped unicode character");
					StoreFreeDynamicString(longStringChar);
					return NULL;
				}

				char u4 = parseHex(input, state);
				if(u4 == '\0') {
					state->position.index -= 5;
					state->position.column -= 5;
					appendParseError(state, "longstringchar", "expected fourth hex number of escaped unicode character");
					StoreFreeDynamicString(longStringChar);
					return NULL;
				}

				char conversion[5] = {u1, u2, u3, u4, '\0'};
				uint32_t codepoint = strtol(conversion, NULL, 16);
				StoreDynamicString utf8 = StoreConvertUnicodeToUtf8(codepoint);

				StoreAppendDynamicString(longStringChar, "%s", StoreReadDynamicString(utf8));
				StoreFreeDynamicString(utf8);
			}
			break;
			default:
				state->position.index -= 2;
				state->position.column -= 2;
				appendParseError(state, "longstringchar", "expected escaped character, but encountered %c", c);
				StoreFreeDynamicString(longStringChar);
				return NULL;
			break;
		}
	} else if(c == '"' || c == '\0') {
		StoreFreeDynamicString(longStringChar);
		return NULL;
	} else {
		while(c != '\\' && c != '"' && c != '\0') {
			StoreAppendDynamicString(longStringChar, "%c", c);

			state->position.index++;
			state->position.column++;

			if(c == '\n') {
				state->position.line++;
				state->position.column = 1;
			}

			c = input[state->position.index];
		}
	}

	return longStringChar;
}

static char parseTerminal(const char *input, ParseState *state)
{
	ParseState terminalState;
	terminalState.position = state->position;
	terminalState.error = StoreCreateDynamicString();
	terminalState.level = state->level + 1;

	char c;

	do {
		c = parseDelimiter(input, &terminalState);
	} while(c != '\0');

	c = input[terminalState.position.index];
	if(c == '\0') {
		appendParseError(state, "terminal", "encountered end of input");
		StoreFreeDynamicString(terminalState.error);
		return '\0';
	}

	state->position = terminalState.position;
	StoreFreeDynamicString(terminalState.error);
	return c;
}

static char parseShortStringChar(const char *input, ParseState *state)
{
	char c = input[state->position.index];

	if(!isspace(c) && c != ',' && c != ';' && c != '"' && c != '(' && c != '[' && c != '{' && c != ')' && c != ']' && c != '}' && c != ':' && c != '=') {
		state->position.index++;
		state->position.column++;
		return c;
	}

	appendParseError(state, "shortstringchar", "encountered non-shortstringchar '%c'", c);
	return '\0';
}

static char parseHex(const char *input, ParseState *state)
{
	char c = input[state->position.index];

	if(isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
		state->position.index++;
		state->position.column++;
		return c;
	}

	appendParseError(state, "hex", "encountered non-hex '%c'", c);
	return '\0';
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
