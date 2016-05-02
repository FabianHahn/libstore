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

static Store *parseValue(const char *input, StoreParseState *state);
static Store *parseString(const char *input, StoreParseState *state);
static Store *parseInt(const char *input, StoreParseState *state);
static Store *parseFloat(const char *input, StoreParseState *state);
static Store *parseList(const char *input, StoreParseState *state);
static Store *parseElements(const char *input, StoreParseState *state);
static Store *parseMap(const char *input, StoreParseState *state);
static Store *parseEntries(const char *input, StoreParseState *state);
static Entry *parseEntry(const char *input, StoreParseState *state);
static StoreDynamicString parseDigits(const char *input, StoreParseState *state);
static StoreDynamicString parseFloating(const char *input, StoreParseState *state);
static StoreDynamicString parseExponential(const char *input, StoreParseState *state);
static StoreDynamicString parseShortString(const char *input, StoreParseState *state);
static StoreDynamicString parseLongString(const char *input, StoreParseState *state);
static StoreDynamicString parseLongStringChunk(const char *input, StoreParseState *state);
static char parseTerminal(const char *input, StoreParseState *state);
static char parseHex(const char *input, StoreParseState *state);
static char parseDigit(const char *input, StoreParseState *state);
static char parseDelimiter(const char *input, StoreParseState *state);
static void report(bool success, StoreParseState *parentState, StoreParseState *currentState, const char *type, const char *message, ...);
static void freeParseReport(StoreParseReport *lastReport);
static bool isHex(char c);
static bool isSeparator(char c);

/**
 * store	: value '\0'
 * 			| entries '\0'
 */
Store *StoreParse(const char *input, StoreParseState *state)
{
	StoreParseState storeState;
	storeState.position = state->position;
	storeState.lastReport = NULL;

	Store *valueStore = parseValue(input, &storeState);
	if(valueStore != NULL) {
		char c = parseTerminal(input, &storeState);
		if(c == '\0') {
			// make sure it's and actual EOF, not just a parseTerminal failure
			if(input[storeState.position.index] == '\0') {
				report(true, state, &storeState, "store", "parsed value store of type %s", StoreGetTypeName(valueStore));
				state->position = storeState.position;
				return valueStore;
			}
		}

		StoreParseState valueState;
		valueState.position = storeState.position;
		valueState.lastReport = NULL;
		report(false, &storeState, &valueState, "value", "expected termination by end of input, but got '%c'", c);
		StoreFree(valueStore);
	}

	storeState.position = state->position;
	Store *entriesStore = parseEntries(input, &storeState);
	if(entriesStore != NULL) {
		char c = parseTerminal(input, &storeState);
		if(c == '\0') {
			// make sure it's and actual EOF, not just a parseTerminal failure
			if(input[storeState.position.index] == '\0') {
				report(true, state, &storeState, "entries", "parsed entries store");
				state->position = storeState.position;
				return valueStore;
			}
		}

		StoreParseState entriesState;
		entriesState.position = storeState.position;
		entriesState.lastReport = NULL;
		report(false, &storeState, &entriesState, "entries", "expected termination by end of input, but got '%c'", c);
		StoreFree(valueStore);
	}

	report(false, state, &storeState, "store", "expected value or entries");
	return NULL;
}

/**
 * value	: nonterminal* int separator
 * 			| nonterminal* float separator
 * 			| nonterminal* string separator
 * 			| nonterminal* list separator
 * 			| nonterminal* struct separator
 */
static Store *parseValue(const char *input, StoreParseState *state)
{
	StoreParseState valueState;
	valueState.lastReport = NULL;
	valueState.position = state->position;

	char c = parseTerminal(input, &valueState);
	if(c == '\0') {
		report(false, state, &valueState, "value", "expected terminal");
		return NULL;
	}
	StoreParseStatePosition terminalPosition = valueState.position;

	valueState.position = terminalPosition;
	Store *intStore = parseInt(input, &valueState);
	if(intStore != NULL) {
		char c = input[valueState.position.index];
		if(isSeparator(c)) {
			report(true, state, &valueState, "value", "parsed int");
			state->position = valueState.position;
			return intStore;
		} else {
			StoreParseState intState;
			intState.position = valueState.position;
			intState.lastReport = NULL;
			report(false, &valueState, &intState, "int", "expected termination by separator but got '%c'", c);
			StoreFree(intStore);
		}
	}

	valueState.position = terminalPosition;
	Store *floatStore = parseFloat(input, &valueState);
	if(floatStore != NULL) {
		char c = input[valueState.position.index];
		if(isSeparator(c)) {
			report(true, state, &valueState, "value", "parsed float");
			state->position = valueState.position;
			return floatStore;
		} else {
			StoreParseState floatState;
			floatState.position = valueState.position;
			floatState.lastReport = NULL;
			report(false, &valueState, &floatState, "float", "expected termination by separator but got '%c'", c);
			StoreFree(floatStore);
		}
	}

	valueState.position = terminalPosition;
	Store *stringStore = parseString(input, &valueState);
	if(stringStore != NULL) {
		char c = input[valueState.position.index];
		if(isSeparator(c)) {
			report(true, state, &valueState, "value", "parsed string");
			state->position = valueState.position;
			return stringStore;
		} else {
			StoreParseState stringState;
			stringState.position = valueState.position;
			stringState.lastReport = NULL;
			report(false, &valueState, &stringState, "string", "expected termination by separator but got '%c'", c);
			StoreFree(stringStore);
		}
	}

	valueState.position = terminalPosition;
	Store *listStore = parseList(input, &valueState);
	if(listStore != NULL) {
		char c = input[valueState.position.index];
		if(isSeparator(c)) {
			report(true, state, &valueState, "value", "parsed list");
			state->position = valueState.position;
			return listStore;
		} else {
			StoreParseState listState;
			listState.position = valueState.position;
			listState.lastReport = NULL;
			report(false, &valueState, &listState, "list", "expected termination by separator but got '%c'", c);
			StoreFree(listStore);
		}
	}

	valueState.position = terminalPosition;
	Store *mapStore = parseMap(input, &valueState);
	if(mapStore != NULL) {
		char c = input[valueState.position.index];
		if(isSeparator(c)) {
			report(true, state, &valueState, "value", "parsed map");
			state->position = valueState.position;
			return mapStore;
		} else {
			StoreParseState mapState;
			mapState.position = valueState.position;
			mapState.lastReport = NULL;
			report(false, &valueState, &mapState, "map", "expected termination by separator but got '%c'", c);
			StoreFree(mapStore);
		}
	}

	report(false, state, &valueState, "value", "expected int, float, string, list, or map");
	return NULL;
}

/**
 * string	: simplestring
 * 			| '"' longstring '"'
 */
static Store *parseString(const char *input, StoreParseState *state)
{
	StoreParseState stringState;
	stringState.position = state->position;
	stringState.lastReport = NULL;

	Store *stringStore = NULL;

	bool isShort = true;
	char c = input[stringState.position.index];
	if(c == '"') {
		isShort = false;

		// eat that character
		stringState.position.index++;
		stringState.position.column++;

		StoreDynamicString longString = parseLongString(input, &stringState);
		if(longString == NULL) {
			report(false, state, &stringState, "string", "expected long string");
			return NULL;
		}

		c = input[stringState.position.index];
		if(c != '"') {
			report(false, state, &stringState, "string", "expected '\"' delimiter after long string, but got '%c'", c);
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
			report(false, state, &stringState, "string", "expected short string");
			return NULL;
		}

		stringStore = StoreCreateStringValue(StoreReadDynamicString(shortString));
		StoreFreeDynamicString(shortString);
	}

	report(true, state, &stringState, "string", "parsed %s string", isShort ? "short" : "long");
	state->position = stringState.position;
	return stringStore;
}

/**
 * int	: '-'? digits
 */
static Store *parseInt(const char *input, StoreParseState *state)
{
	StoreParseState intState;
	intState.position = state->position;
	intState.lastReport = NULL;

	StoreDynamicString intString = StoreCreateDynamicString();

	bool isNegative = false;
	char c = input[intState.position.index];
	if(c == '-') {
		StoreAppendDynamicString(intString, "-");

		// eat that character
		intState.position.index++;
		intState.position.column++;

		isNegative = true;
	}

	StoreDynamicString digitsString = parseDigits(input, &intState);
	if(digitsString == NULL) {
		report(false, state, &intState, "int", "expected digits");
		StoreFreeDynamicString(intString);
		return NULL;
	}

	StoreAppendDynamicString(intString, "%s", StoreReadDynamicString(digitsString));
	StoreFreeDynamicString(digitsString);

	report(true, state, &intState, "int", "parsed %s int", isNegative ? "negative" : "positive");
	state->position = intState.position;
	int intValue = atoi(StoreReadDynamicString(intString));
	StoreFreeDynamicString(intString);
	return StoreCreateIntValue(intValue);
}

/**
 * float	: '-'? digits floating? exponential?
 */
static Store *parseFloat(const char *input, StoreParseState *state)
{
	StoreParseState floatState;
	floatState.position = state->position;
	floatState.lastReport = NULL;

	StoreDynamicString floatString = StoreCreateDynamicString();

	bool isNegative = false;
	char c = input[floatState.position.index];
	if(c == '-') {
		StoreAppendDynamicString(floatString, "-");

		// eat that character
		floatState.position.index++;
		floatState.position.column++;

		isNegative = true;
	}

	StoreDynamicString digitsString = parseDigits(input, &floatState);
	if(digitsString == NULL) {
		report(false, state, &floatState, "float", "expected digits");
		StoreFreeDynamicString(floatString);
		return NULL;
	}

	StoreAppendDynamicString(floatString, "%s", StoreReadDynamicString(digitsString));
	StoreFreeDynamicString(digitsString);

	bool hasFloating = false;
	StoreDynamicString floatingString = parseFloating(input, &floatState);
	if(floatingString != NULL) {
		StoreAppendDynamicString(floatString, "%s", StoreReadDynamicString(floatingString));
		StoreFreeDynamicString(floatingString);
		hasFloating = true;
	}

	bool hasExponential = false;
	StoreDynamicString exponentialString = parseExponential(input, &floatState);
	if(exponentialString != NULL) {
		StoreAppendDynamicString(floatString, "%s", StoreReadDynamicString(exponentialString));
		StoreFreeDynamicString(exponentialString);
		hasExponential = true;
	}

	report(false, state, &floatState, "float", "parsed %s float %s floating part and %s exponential part", isNegative ? "negative" : "positive", hasFloating ? "with" : "without", hasExponential ? "with" : "without");
	state->position = floatState.position;
	double floatValue = atof(StoreReadDynamicString(floatString));
	StoreFreeDynamicString(floatString);
	return StoreCreateFloatValue(floatValue);
}

/**
 * list	: '(' elements ')'
 * 		| '[' elements ']'
 */
static Store *parseList(const char *input, StoreParseState *state)
{
	StoreParseState listState;
	listState.position = state->position;
	listState.lastReport = NULL;

	Store *listStore = NULL;

	bool isSquare = false;
	char c = input[listState.position.index];
	if(c == '(') {
		// eat that character
		listState.position.index++;
		listState.position.column++;

		listStore = parseElements(input, &listState);
		if(listStore == NULL) {
			report(false, state, &listState, "list", "expected elements");
			return NULL;
		}

		c = parseTerminal(input, &listState);
		if(c == '\0') {
			report(false, state, &listState, "list", "expected terminal");
			StoreFree(listStore);
			return NULL;
		} else if(c != ')') {
			report(false, state, &listState, "list", "ending character must be ')', but got '%c'", c);
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
			report(false, state, &listState, "list", "expected elements");
			return NULL;
		}

		c = parseTerminal(input, &listState);
		if(c == '\0') {
			report(false, state, &listState, "list", "expected terminal");
			StoreFree(listStore);
			return NULL;
		} else if(c != ']') {
			report(false, state, &listState, "list", "ending character must be ']', but got '%c'", c);
			StoreFree(listStore);
			return NULL;
		}

		// eat that character
		listState.position.index++;
		listState.position.column++;

		isSquare = true;
	} else {
		report(false, state, &listState, "list", "opening character must be '(' or '[', but got '%c'", c);
		return NULL;
	}

	report(true, state, &listState, "list", "parsed list with %s brackets", isSquare ? "square" : "round");
	state->position = listState.position;
	return listStore;
}

/**
 * elements	: value*
 */
static Store *parseElements(const char *input, StoreParseState *state)
{
	StoreParseState elementsState;
	elementsState.position = state->position;
	elementsState.lastReport = NULL;

	int numElements = 0;
	Store *listStore = StoreCreateListValue();
	while(true) {
		Store *valueStore = parseValue(input, &elementsState);
		if(valueStore == NULL) {
			break;
		}

		StoreAppendList(listStore->content.listValue, valueStore);
		numElements++;
	}

	report(true, state, &elementsState, "elements", "parsed %d elements", numElements);
	state->position = elementsState.position;
	return listStore;
}

/**
 * map	: '{' entries '}'
 */
static Store *parseMap(const char *input, StoreParseState *state)
{
	StoreParseState mapState;
	mapState.position = state->position;
	mapState.lastReport = NULL;

	char c = input[mapState.position.index];
	if(c != '{') {
		report(false, state, &mapState, "map", "opening character must be '{', but got '%c'", c);
		return NULL;
	}

	// eat that character
	mapState.position.index++;
	mapState.position.column++;

	Store *mapStore = parseEntries(input, &mapState);
	if(mapStore == NULL) {
		report(false, state, &mapState, "map", "expected entries");
		return NULL;
	}

	c = parseTerminal(input, &mapState);
	if(c == '\0') {
		report(false, state, &mapState, "map", "expected terminal");
		StoreFree(mapStore);
		return NULL;
	} else if(c != '}') {
		report(false, state, &mapState, "map", "ending character must be '}', but got '%c'", c);
		StoreFree(mapStore);
		return NULL;
	}

	// eat that character
	mapState.position.index++;
	mapState.position.column++;

	report(true, state, &mapState, "map", "parsed map");
	state->position = mapState.position;
	return mapStore;
}

/**
 * entries	: entry*
 */
static Store *parseEntries(const char *input, StoreParseState *state)
{
	StoreParseState entriesState;
	entriesState.position = state->position;
	entriesState.lastReport = NULL;

	int numEntries = 0;
	Store *entriesStore = StoreCreateMapValue();
	while(true) {
		Entry *entry = parseEntry(input, &entriesState);
		if(entry == NULL) {
			break;
		}

		StoreInsertMap(entriesStore->content.mapValue, entry->key, entry->value);
		StoreFreeMemory(entry);
		numEntries++;
	}

	report(true, state, &entriesState, "entries", "parsed %d entries", numEntries);
	state->position = entriesState.position;
	return entriesStore;
}

/**
 * entry	: nonterminal* string '=' value
 * 			| nonterminal* string ':' value
 */
static Entry *parseEntry(const char *input, StoreParseState *state)
{
	StoreParseState entryState;
	entryState.position = state->position;
	entryState.lastReport = NULL;

	char c = parseTerminal(input, &entryState);
	if(c == '\0') {
		report(false, state, &entryState, "entry", "expected terminal");
		return NULL;
	}

	Store *stringStore = parseString(input, &entryState);
	if(stringStore == NULL) {
		report(false, state, &entryState, "entry", "expected key string");
		return NULL;
	}

	c = parseTerminal(input, &entryState);
	if(c == '\0') {
		report(false, state, &entryState, "entry", "expected terminal");
		StoreFree(stringStore);
		return NULL;
	} else if(c != ':' && c != '=') {
		report(false, state, &entryState, "entry", "entry separating character must be ':' or '=', but got '%c'", c);
		StoreFree(stringStore);
		return NULL;
	}

	// eat that character
	entryState.position.index++;
	entryState.position.column++;

	Store *valueStore = parseValue(input, &entryState);
	if(valueStore == NULL) {
		report(false, state, &entryState, "entry", "expected value");
		StoreFree(stringStore);
		return NULL;
	}

	Entry *entry = StoreAllocateMemoryType(Entry);
	entry->key = strdup(stringStore->content.stringValue);
	entry->value = valueStore;
	StoreFree(stringStore);

	report(true, state, &entryState, "entry", "parsed entry with %s value", StoreGetTypeName(valueStore));
	state->position = entryState.position;
	return entry;
}

/**
 * digits	: digit+
 */
static StoreDynamicString parseDigits(const char *input, StoreParseState *state)
{
	StoreParseState digitsState;
	digitsState.position = state->position;
	digitsState.lastReport = NULL;

	StoreDynamicString digitsString = StoreCreateDynamicString();

	int numDigits = 0;
	while(true) {
		char c = input[digitsState.position.index];

		if(!isdigit(c)) {
			break;
		}

		digitsState.position.index++;
		digitsState.position.column++;
		StoreAppendDynamicString(digitsString, "%c", c);

		numDigits++;
	}

	if(numDigits == 0) {
		report(false, state, &digitsState, "digits", "no digits parsed");
		StoreFreeDynamicString(digitsString);
		return NULL;
	}

	report(true, state, &digitsState, "digits", "parsed %d digits", numDigits);
	state->position = digitsState.position;
	return digitsString;
}

/**
 * floating	: '.' digits?
 */
static StoreDynamicString parseFloating(const char *input, StoreParseState *state)
{
	StoreParseState floatingState;
	floatingState.position = state->position;
	floatingState.lastReport = NULL;

	StoreDynamicString floatingString = StoreCreateDynamicString();

	char c = input[floatingState.position.index];
	if(c == '\0') {
		report(false, state, &floatingState, "floating", "unexpected end of input");
		StoreFreeDynamicString(floatingString);
		return NULL;
	} else if(c == '.') {
		StoreAppendDynamicString(floatingString, ".");

		// eat that character
		floatingState.position.index++;
		floatingState.position.column++;
	}

	bool hasDigits = false;
	StoreDynamicString digitsString = parseDigits(input, &floatingState);
	if(digitsString != NULL) {
		StoreAppendDynamicString(floatingString, "%s", StoreReadDynamicString(digitsString));
		StoreFreeDynamicString(digitsString);
		hasDigits = true;
	}

	report(true, state, &floatingState, "floating", "parsed floating %s digits", hasDigits ? "with" : "without");
	state->position = floatingState.position;
	return floatingString;
}

/**
 * exponential : ('e'|'E') ('+'|'-')? digits
 */
static StoreDynamicString parseExponential(const char *input, StoreParseState *state)
{
	StoreParseState exponentialState;
	exponentialState.position = state->position;
	exponentialState.lastReport = NULL;

	StoreDynamicString exponentialString = StoreCreateDynamicString();

	char c = input[exponentialState.position.index];
	if(c == '\0') {
		report(false, state, &exponentialState, "exponential", "unexpected end of input");
		StoreFreeDynamicString(exponentialString);
		return NULL;
	} else if(c != 'e' && c != 'E') {
		report(false, state, &exponentialState, "exponential", "expected 'e' or 'E'");
		StoreFreeDynamicString(exponentialString);
		return NULL;
	}

	StoreAppendDynamicString(exponentialString, "%c", c);

	// eat that character
	exponentialState.position.index++;
	exponentialState.position.column++;

	bool isNegative = false;
	c = input[exponentialState.position.index];
	if(c == '\0') {
		report(false, state, &exponentialState, "exponential", "unexpected end of input");
		StoreFreeDynamicString(exponentialString);
		return NULL;
	} else if(c == '+' || c == '-') {
		StoreAppendDynamicString(exponentialString, "%c", c);

		// eat that character
		exponentialState.position.index++;
		exponentialState.position.column++;

		if(c == '-') {
			isNegative = true;
		}
	}

	StoreDynamicString digitsString = parseDigits(input, &exponentialState);
	if(digitsString == NULL) {
		report(false, state, &exponentialState, "exponential", "expected digits");
		StoreFreeDynamicString(exponentialString);
		return NULL;
	}

	StoreAppendDynamicString(exponentialString, "%s", StoreReadDynamicString(digitsString));
	StoreFreeDynamicString(digitsString);

	report(true, state, &exponentialState, "exponential", "parsed %s exponential", isNegative ? "negative" : "positive");
	state->position = exponentialState.position;
	return exponentialString;
}

/**
 * shortstring		: shortstringchar+
 *
 * where
 * shortstringchar	: nonseparator
 */
static StoreDynamicString parseShortString(const char *input, StoreParseState *state)
{
	StoreParseState shortStringState;
	shortStringState.position = state->position;
	shortStringState.lastReport = NULL;

	int numChars = 0;
	StoreDynamicString shortString = StoreCreateDynamicString();
	while(true) {
		char c = input[shortStringState.position.index];
		if(isSeparator(c)) {
			break;
		}

		shortStringState.position.index++;
		shortStringState.position.column++;
		StoreAppendDynamicString(shortString, "%c", c);

		numChars++;
	}

	if(numChars == 0) {
		report(false, state, &shortStringState, "short string", "no short string characters parsed");
		StoreFreeDynamicString(shortString);
		return NULL;
	}

	report(true, state, &shortStringState, "short string", "parsed short string with %d characters", numChars);
	state->position = shortStringState.position;
	return shortString;
}

/**
 * longstring		: longstringchar*
 *
 * where
 * longstringchar	: nonspecial
 * 					| '\' escaped
 * 					| '\' 'u' hex hex hex hex
 */
static StoreDynamicString parseLongString(const char *input, StoreParseState *state)
{
	StoreParseState longStringState;
	longStringState.position = state->position;
	longStringState.lastReport = NULL;

	int numChars = 0;
	StoreDynamicString longString = StoreCreateDynamicString();
	while(true) {
		char c = input[longStringState.position.index];
		if(c == '\\') {
			longStringState.position.index++;
			longStringState.position.column++;

			c = input[longStringState.position.index];
			longStringState.position.index++;
			longStringState.position.column++;

			switch(c) {
				case '"':
					StoreAppendDynamicString(longString, "\"");
				break;
				case '\\':
					StoreAppendDynamicString(longString, "\\");
				break;
				case '/':
					StoreAppendDynamicString(longString, "/");
				break;
				case 'b':
					StoreAppendDynamicString(longString, "\b");
				break;
				case 'f':
					StoreAppendDynamicString(longString, "\f");
				break;
				case 'n':
					StoreAppendDynamicString(longString, "\n");
				break;
				case 'r':
					StoreAppendDynamicString(longString, "\r");
				break;
				case 't':
					StoreAppendDynamicString(longString, "\t");
				break;
				case 'u':
				{
					char u1 = input[longStringState.position.index];
					if(!isHex(u1)) {
						report(false, state, &longStringState, "long string", "expected first hex number of escaped unicode character, but got '%c'", u1);
						StoreFreeDynamicString(longString);
						return NULL;
					}
					longStringState.position.index++;
					longStringState.position.column++;

					char u2 = input[longStringState.position.index];
					if(!isHex(u2)) {
						report(false, state, &longStringState, "long string", "expected second hex number of escaped unicode character, but got '%c'", u2);
						StoreFreeDynamicString(longString);
						return NULL;
					}
					longStringState.position.index++;
					longStringState.position.column++;

					char u3 = input[longStringState.position.index];
					if(!isHex(u3)) {
						report(false, state, &longStringState, "long string", "expected third hex number of escaped unicode character, but got '%c'", u3);
						StoreFreeDynamicString(longString);
						return NULL;
					}
					longStringState.position.index++;
					longStringState.position.column++;

					char u4 = input[longStringState.position.index];
					if(!isHex(u4)) {
						report(false, state, &longStringState, "long string", "expected fourth hex number of escaped unicode character, but got '%c'", u4);
						StoreFreeDynamicString(longString);
						return NULL;
					}
					longStringState.position.index++;
					longStringState.position.column++;

					char conversion[5] = {u1, u2, u3, u4, '\0'};
					uint32_t codepoint = strtol(conversion, NULL, 16);
					StoreDynamicString utf8 = StoreConvertUnicodeToUtf8(codepoint);

					StoreAppendDynamicString(longString, "%s", StoreReadDynamicString(utf8));
					StoreFreeDynamicString(utf8);
				}
				break;
				default:
					report(false, state, &longStringState, "long string", "expected escaped character, but got '%c'", c);
					StoreFreeDynamicString(longString);
					return NULL;
				break;
			}
		} else if(c == '"' || c == '\0') {
			break;
		} else {
			StoreAppendDynamicString(longString, "%c", c);

			longStringState.position.index++;
			longStringState.position.column++;

			if(c == '\n') {
				longStringState.position.line++;
				longStringState.position.column = 1;
			}
		}

		numChars++;
	}

	report(true, state, &longStringState, "long string", "parsed long string with %d characters", numChars);
	state->position = longStringState.position;
	return longString;
}

static char parseTerminal(const char *input, StoreParseState *state)
{
	StoreParseState terminalState;
	terminalState.position = state->position;
	terminalState.lastReport = NULL;

	int numDelimiters = 0;
	while(true) {
		char c = input[terminalState.position.index];

		if(!isspace(c) && c != ',' && c != ';') {
			break;
		}

		terminalState.position.index++;
		terminalState.position.column++;

		if(c == '\n') {
			terminalState.position.line++;
			terminalState.position.column = 1;
		}

		numDelimiters++;
	}

	report(true, state, &terminalState, "terminal", "parsed terminal after %d delimiters", numDelimiters);
	state->position = terminalState.position;
	return input[terminalState.position.index];
}

static void report(bool success, StoreParseState *parentState, StoreParseState *currentState, const char *type, const char *message, ...)
{
	va_list va;
	va_start(va, message);

	StoreDynamicString messageString = StoreCreateDynamicString();
	StoreAppendDynamicStringV(messageString, message, va);

	StoreParseReport *report = StoreAllocateMemoryType(StoreParseReport);
	report->success = success;
	report->position = currentState->position;
	report->type = type;
	report->message = strdup(StoreReadDynamicString(messageString));
	report->lastSubReport = currentState->lastReport;
	report->previousReport = parentState->lastReport;
	parentState->lastReport = report;

	StoreFreeDynamicString(messageString);
}

static void freeParseReport(StoreParseReport *lastReport)
{
	if(lastReport != NULL) {
		freeParseReport(lastReport->lastSubReport);
		freeParseReport(lastReport->previousReport);
		free(lastReport->message);
	}
}

static bool isHex(char c)
{
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool isSeparator(char c)
{
	return isspace(c) || c == ',' || c == ';' || c == '"' || c == '(' || c == '[' || c == '{' || c == ')' || c == ']' || c == '}' || c == ':' || c == '=' || c == '\0';
}
