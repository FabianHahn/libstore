#include <ctype.h> // isspace isdigit
#include <stdarg.h> // va_list va_start
#include <stdbool.h> // bool true false
#include <stddef.h> // NULL
#include <stdlib.h> // atoi atof
#include <string.h> // strdup

#include "store/encoding.h"
#include "store/memory.h"
#include "store/parser.h"

typedef struct {
	char *key;
	Store *value;
} Entry;

static Store *parseStore(const char *input, StoreParseState *state);
static Store *parseValue(const char *input, StoreParseState *state);
static Store *parseString(const char *input, StoreParseState *state);
static Store *parseInt(const char *input, StoreParseState *state);
static Store *parseFloat(const char *input, StoreParseState *state);
static Store *parseList(const char *input, StoreParseState *state);
static Store *parseElements(const char *input, StoreParseState *state);
static Store *parseMap(const char *input, StoreParseState *state);
static Store *parseEntries(const char *input, StoreParseState *state);
static Entry *parseEntry(const char *input, StoreParseState *state);
static GString *parseDigits(const char *input, StoreParseState *state);
static GString *parseFloating(const char *input, StoreParseState *state);
static GString *parseExponential(const char *input, StoreParseState *state);
static GString *parseShortString(const char *input, StoreParseState *state);
static GString *parseLongString(const char *input, StoreParseState *state);
static GString *parseLongStringChunk(const char *input, StoreParseState *state);
static char parseTerminal(const char *input, StoreParseState *state);
static char parseHex(const char *input, StoreParseState *state);
static char parseDigit(const char *input, StoreParseState *state);
static char parseDelimiter(const char *input, StoreParseState *state);
static StoreParseState *createParseState(StoreParseStatePosition position);
static void freeParseState(StoreParseState *state);
static void reportAndFreeState(bool success, StoreParseState *parentState, StoreParseState *state, const char *type, const char *message, ...);
static void freeParseReportPointer(void *parseReportPointer);
static void freeParseReport(StoreParseReport *lastReport);
static bool isHex(char c);
static bool isSeparator(char c);

StoreParser *storeCreateParser()
{
	StoreParser *parser = storeAllocateMemoryType(StoreParser);
	parser->state.position.index = 0;
	parser->state.position.line = 1;
	parser->state.position.column = 1;
	parser->state.reports = g_queue_new();
	return parser;
}

void storeResetParser(StoreParser *parser)
{
	parser->state.position.index = 0;
	parser->state.position.line = 1;
	parser->state.position.column = 1;
	g_queue_free_full(parser->state.reports, freeParseReportPointer);
	parser->state.reports = g_queue_new();
}

void storeFreeParser(StoreParser *parser)
{
	g_queue_free_full(parser->state.reports, freeParseReportPointer);
	storeFreeMemory(parser);
}

Store *storeParse(StoreParser *parser, const char *input)
{
	storeResetParser(parser);
	return parseStore(input, &parser->state);
}

/**
 * store	: value '\0'
 * 			| entries '\0'
 */
static Store *parseStore(const char *input, StoreParseState *state)
{
	StoreParseState *storeState = createParseState(state->position);

	Store *valueStore = parseValue(input, storeState);
	if(valueStore != NULL) {
		char c = parseTerminal(input, storeState);
		if(c == '\0') {
			// make sure it's and actual EOF, not just a parseTerminal failure
			if(input[storeState->position.index] == '\0') {
				state->position = storeState->position;
				reportAndFreeState(true, state, storeState, "store", "parsed value store of type %s", storeGetTypeName(valueStore));
				return valueStore;
			}
		}

		StoreParseState *valueState = createParseState(storeState->position);
		reportAndFreeState(false, storeState, valueState, "value", "expected termination by end of input, but got '%c'", c);
		storeFree(valueStore);
	}

	storeState->position = state->position;
	Store *entriesStore = parseEntries(input, storeState);
	if(entriesStore != NULL) {
		char c = parseTerminal(input, storeState);
		if(c == '\0') {
			// make sure it's and actual EOF, not just a parseTerminal failure
			if(input[storeState->position.index] == '\0') {
				state->position = storeState->position;
				reportAndFreeState(true, state, storeState, "store", "parsed entries store");
				return entriesStore;
			}
		}

		StoreParseState *entriesState = createParseState(storeState->position);
		reportAndFreeState(false, storeState, entriesState, "entries", "expected termination by end of input, but got '%c'", c);
		storeFree(entriesStore);
	}

	reportAndFreeState(false, state, storeState, "store", "expected value or entries");
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
	StoreParseState *valueState = createParseState(state->position);

	char c = parseTerminal(input, valueState);
	if(c == '\0') {
		reportAndFreeState(false, state, valueState, "value", "expected terminal");
		return NULL;
	}
	StoreParseStatePosition terminalPosition = valueState->position;

	valueState->position = terminalPosition;
	Store *intStore = parseInt(input, valueState);
	if(intStore != NULL) {
		char c = input[valueState->position.index];
		if(isSeparator(c)) {
			state->position = valueState->position;
			reportAndFreeState(true, state, valueState, "value", "parsed int");
			return intStore;
		} else {
			StoreParseState *intState = createParseState(valueState->position);
			reportAndFreeState(false, valueState, intState, "int", "expected termination by separator but got '%c'", c);
			storeFree(intStore);
		}
	}

	valueState->position = terminalPosition;
	Store *floatStore = parseFloat(input, valueState);
	if(floatStore != NULL) {
		char c = input[valueState->position.index];
		if(isSeparator(c)) {
			state->position = valueState->position;
			reportAndFreeState(true, state, valueState, "value", "parsed float");
			return floatStore;
		} else {
			StoreParseState *floatState = createParseState(valueState->position);
			reportAndFreeState(false, valueState, floatState, "float", "expected termination by separator but got '%c'", c);
			storeFree(floatStore);
		}
	}

	valueState->position = terminalPosition;
	Store *stringStore = parseString(input, valueState);
	if(stringStore != NULL) {
		char c = input[valueState->position.index];
		if(isSeparator(c)) {
			state->position = valueState->position;
			reportAndFreeState(true, state, valueState, "value", "parsed string");
			return stringStore;
		} else {
			StoreParseState *stringState = createParseState(valueState->position);
			reportAndFreeState(false, valueState, stringState, "string", "expected termination by separator but got '%c'", c);
			storeFree(stringStore);
		}
	}

	valueState->position = terminalPosition;
	Store *listStore = parseList(input, valueState);
	if(listStore != NULL) {
		char c = input[valueState->position.index];
		if(isSeparator(c)) {
			state->position = valueState->position;
			reportAndFreeState(true, state, valueState, "value", "parsed list");
			return listStore;
		} else {
			StoreParseState *listState = createParseState(valueState->position);
			reportAndFreeState(false, valueState, listState, "list", "expected termination by separator but got '%c'", c);
			storeFree(listStore);
		}
	}

	valueState->position = terminalPosition;
	Store *mapStore = parseMap(input, valueState);
	if(mapStore != NULL) {
		char c = input[valueState->position.index];
		if(isSeparator(c)) {
			state->position = valueState->position;
			reportAndFreeState(true, state, valueState, "value", "parsed map");
			return mapStore;
		} else {
			StoreParseState *mapState = createParseState(valueState->position);
			reportAndFreeState(false, valueState, mapState, "map", "expected termination by separator but got '%c'", c);
			storeFree(mapStore);
		}
	}

	reportAndFreeState(false, state, valueState, "value", "expected int, float, string, list, or map");
	return NULL;
}

/**
 * string	: simplestring
 * 			| '"' longstring '"'
 */
static Store *parseString(const char *input, StoreParseState *state)
{
	StoreParseState *stringState = createParseState(state->position);

	Store *stringStore = NULL;

	bool isShort = true;
	char c = input[stringState->position.index];
	if(c == '"') {
		isShort = false;

		// eat that character
		stringState->position.index++;
		stringState->position.column++;

		GString *longString = parseLongString(input, stringState);
		if(longString == NULL) {
			reportAndFreeState(false, state, stringState, "string", "expected long string");
			return NULL;
		}

		c = input[stringState->position.index];
		if(c != '"') {
			reportAndFreeState(false, state, stringState, "string", "expected '\"' delimiter after long string, but got '%c'", c);
			g_string_free(longString, true);
			return NULL;
		}

		// eat that character
		stringState->position.index++;
		stringState->position.column++;

		stringStore = storeCreateStringValue(longString->str);
		g_string_free(longString, true);
	} else {
		GString *shortString = parseShortString(input, stringState);
		if(shortString == NULL) {
			reportAndFreeState(false, state, stringState, "string", "expected short string");
			return NULL;
		}

		stringStore = storeCreateStringValue(shortString->str);
		g_string_free(shortString, true);
	}

	state->position = stringState->position;
	reportAndFreeState(true, state, stringState, "string", "parsed %s string", isShort ? "short" : "long");
	return stringStore;
}

/**
 * int	: '-'? digits
 */
static Store *parseInt(const char *input, StoreParseState *state)
{
	StoreParseState *intState = createParseState(state->position);

	GString *intString = g_string_new("");

	bool isNegative = false;
	char c = input[intState->position.index];
	if(c == '-') {
		g_string_append_printf(intString, "-");

		// eat that character
		intState->position.index++;
		intState->position.column++;

		isNegative = true;
	}

	GString *digitsString = parseDigits(input, intState);
	if(digitsString == NULL) {
		reportAndFreeState(false, state, intState, "int", "expected digits");
		g_string_free(intString, true);
		return NULL;
	}

	g_string_append_printf(intString, "%s", digitsString->str);
	g_string_free(digitsString, true);

	state->position = intState->position;
	reportAndFreeState(true, state, intState, "int", "parsed %s int", isNegative ? "negative" : "positive");
	int intValue = atoi(intString->str);
	g_string_free(intString, true);
	return storeCreateIntValue(intValue);
}

/**
 * float	: '-'? digits floating? exponential?
 */
static Store *parseFloat(const char *input, StoreParseState *state)
{
	StoreParseState *floatState = createParseState(state->position);

	GString *floatString = g_string_new("");

	bool isNegative = false;
	char c = input[floatState->position.index];
	if(c == '-') {
		g_string_append_printf(floatString, "-");

		// eat that character
		floatState->position.index++;
		floatState->position.column++;

		isNegative = true;
	}

	GString *digitsString = parseDigits(input, floatState);
	if(digitsString == NULL) {
		reportAndFreeState(false, state, floatState, "float", "expected digits");
		g_string_free(floatString, true);
		return NULL;
	}

	g_string_append_printf(floatString, "%s", digitsString->str);
	g_string_free(digitsString, true);

	bool hasFloating = false;
	GString *floatingString = parseFloating(input, floatState);
	if(floatingString != NULL) {
		g_string_append_printf(floatString, "%s", floatingString->str);
		g_string_free(floatingString, true);
		hasFloating = true;
	}

	bool hasExponential = false;
	GString *exponentialString = parseExponential(input, floatState);
	if(exponentialString != NULL) {
		g_string_append_printf(floatString, "%s", exponentialString->str);
		g_string_free(exponentialString, true);
		hasExponential = true;
	}

	state->position = floatState->position;
	reportAndFreeState(true, state, floatState, "float", "parsed %s float %s floating part and %s exponential part", isNegative ? "negative" : "positive", hasFloating ? "with" : "without", hasExponential ? "with" : "without");
	double floatValue = atof(floatString->str);
	g_string_free(floatString, true);
	return storeCreateFloatValue(floatValue);
}

/**
 * list	: '(' elements ')'
 * 		| '[' elements ']'
 */
static Store *parseList(const char *input, StoreParseState *state)
{
	StoreParseState *listState = createParseState(state->position);

	Store *listStore = NULL;

	bool isSquare = false;
	char c = input[listState->position.index];
	if(c == '(') {
		// eat that character
		listState->position.index++;
		listState->position.column++;

		listStore = parseElements(input, listState);
		if(listStore == NULL) {
			reportAndFreeState(false, state, listState, "list", "expected elements");
			return NULL;
		}

		c = parseTerminal(input, listState);
		if(c == '\0') {
			reportAndFreeState(false, state, listState, "list", "expected terminal");
			storeFree(listStore);
			return NULL;
		} else if(c != ')') {
			reportAndFreeState(false, state, listState, "list", "ending character must be ')', but got '%c'", c);
			storeFree(listStore);
			return NULL;
		}

		// eat that character
		listState->position.index++;
		listState->position.column++;
	} else if(c == '[') {
		// eat that character
		listState->position.index++;
		listState->position.column++;

		listStore = parseElements(input, listState);
		if(listStore == NULL) {
			reportAndFreeState(false, state, listState, "list", "expected elements");
			return NULL;
		}

		c = parseTerminal(input, listState);
		if(c == '\0') {
			reportAndFreeState(false, state, listState, "list", "expected terminal");
			storeFree(listStore);
			return NULL;
		} else if(c != ']') {
			reportAndFreeState(false, state, listState, "list", "ending character must be ']', but got '%c'", c);
			storeFree(listStore);
			return NULL;
		}

		// eat that character
		listState->position.index++;
		listState->position.column++;

		isSquare = true;
	} else {
		reportAndFreeState(false, state, listState, "list", "opening character must be '(' or '[', but got '%c'", c);
		return NULL;
	}

	state->position = listState->position;
	reportAndFreeState(true, state, listState, "list", "parsed list with %s brackets", isSquare ? "square" : "round");
	return listStore;
}

/**
 * elements	: value*
 */
static Store *parseElements(const char *input, StoreParseState *state)
{
	StoreParseState *elementsState = createParseState(state->position);

	int numElements = 0;
	Store *listStore = storeCreateListValue();
	while(true) {
		Store *valueStore = parseValue(input, elementsState);
		if(valueStore == NULL) {
			break;
		}

		g_queue_push_tail(listStore->content.listValue, valueStore);
		numElements++;
	}

	state->position = elementsState->position;
	reportAndFreeState(true, state, elementsState, "elements", "parsed %d elements", numElements);
	return listStore;
}

/**
 * map	: '{' entries '}'
 */
static Store *parseMap(const char *input, StoreParseState *state)
{
	StoreParseState *mapState = createParseState(state->position);

	char c = input[mapState->position.index];
	if(c != '{') {
		reportAndFreeState(false, state, mapState, "map", "opening character must be '{', but got '%c'", c);
		return NULL;
	}

	// eat that character
	mapState->position.index++;
	mapState->position.column++;

	Store *mapStore = parseEntries(input, mapState);
	if(mapStore == NULL) {
		reportAndFreeState(false, state, mapState, "map", "expected entries");
		return NULL;
	}

	c = parseTerminal(input, mapState);
	if(c == '\0') {
		reportAndFreeState(false, state, mapState, "map", "expected terminal");
		storeFree(mapStore);
		return NULL;
	} else if(c != '}') {
		reportAndFreeState(false, state, mapState, "map", "ending character must be '}', but got '%c'", c);
		storeFree(mapStore);
		return NULL;
	}

	// eat that character
	mapState->position.index++;
	mapState->position.column++;

	state->position = mapState->position;
	reportAndFreeState(true, state, mapState, "map", "parsed map");
	return mapStore;
}

/**
 * entries	: entry*
 */
static Store *parseEntries(const char *input, StoreParseState *state)
{
	StoreParseState *entriesState = createParseState(state->position);

	int numEntries = 0;
	Store *entriesStore = storeCreateMapValue();
	while(true) {
		Entry *entry = parseEntry(input, entriesState);
		if(entry == NULL) {
			break;
		}

		g_hash_table_insert(entriesStore->content.mapValue, entry->key, entry->value);
		storeFreeMemory(entry);
		numEntries++;
	}

	state->position = entriesState->position;
	reportAndFreeState(true, state, entriesState, "entries", "parsed %d entries", numEntries);
	return entriesStore;
}

/**
 * entry	: nonterminal* string '=' value
 * 			| nonterminal* string ':' value
 */
static Entry *parseEntry(const char *input, StoreParseState *state)
{
	StoreParseState *entryState = createParseState(state->position);

	char c = parseTerminal(input, entryState);
	if(c == '\0') {
		reportAndFreeState(false, state, entryState, "entry", "expected terminal");
		return NULL;
	}

	Store *stringStore = parseString(input, entryState);
	if(stringStore == NULL) {
		reportAndFreeState(false, state, entryState, "entry", "expected key string");
		return NULL;
	}

	c = parseTerminal(input, entryState);
	if(c == '\0') {
		reportAndFreeState(false, state, entryState, "entry", "expected terminal");
		storeFree(stringStore);
		return NULL;
	} else if(c != ':' && c != '=') {
		reportAndFreeState(false, state, entryState, "entry", "entry separating character must be ':' or '=', but got '%c'", c);
		storeFree(stringStore);
		return NULL;
	}

	// eat that character
	entryState->position.index++;
	entryState->position.column++;

	Store *valueStore = parseValue(input, entryState);
	if(valueStore == NULL) {
		reportAndFreeState(false, state, entryState, "entry", "expected value");
		storeFree(stringStore);
		return NULL;
	}

	Entry *entry = storeAllocateMemoryType(Entry);
	entry->key = strdup(stringStore->content.stringValue);
	entry->value = valueStore;
	storeFree(stringStore);

	state->position = entryState->position;
	reportAndFreeState(true, state, entryState, "entry", "parsed entry with %s value", storeGetTypeName(valueStore));
	return entry;
}

/**
 * digits	: digit+
 */
static GString *parseDigits(const char *input, StoreParseState *state)
{
	StoreParseState *digitsState = createParseState(state->position);

	GString *digitsString = g_string_new("");

	int numDigits = 0;
	while(true) {
		char c = input[digitsState->position.index];

		if(!isdigit(c)) {
			break;
		}

		digitsState->position.index++;
		digitsState->position.column++;
		g_string_append_printf(digitsString, "%c", c);

		numDigits++;
	}

	if(numDigits == 0) {
		reportAndFreeState(false, state, digitsState, "digits", "no digits parsed");
		g_string_free(digitsString, true);
		return NULL;
	}

	state->position = digitsState->position;
	reportAndFreeState(true, state, digitsState, "digits", "parsed %d digits", numDigits);
	return digitsString;
}

/**
 * floating	: '.' digits?
 */
static GString *parseFloating(const char *input, StoreParseState *state)
{
	StoreParseState *floatingState = createParseState(state->position);

	GString *floatingString = g_string_new("");

	char c = input[floatingState->position.index];
	if(c == '\0') {
		reportAndFreeState(false, state, floatingState, "floating", "unexpected end of input");
		g_string_free(floatingString, true);
		return NULL;
	} else if(c == '.') {
		g_string_append_printf(floatingString, ".");

		// eat that character
		floatingState->position.index++;
		floatingState->position.column++;
	}

	bool hasDigits = false;
	GString *digitsString = parseDigits(input, floatingState);
	if(digitsString != NULL) {
		g_string_append_printf(floatingString, "%s", digitsString->str);
		g_string_free(digitsString, true);
		hasDigits = true;
	}

	state->position = floatingState->position;
	reportAndFreeState(true, state, floatingState, "floating", "parsed floating %s digits", hasDigits ? "with" : "without");
	return floatingString;
}

/**
 * exponential : ('e'|'E') ('+'|'-')? digits
 */
static GString *parseExponential(const char *input, StoreParseState *state)
{
	StoreParseState *exponentialState = createParseState(state->position);

	GString *exponentialString = g_string_new("");

	char c = input[exponentialState->position.index];
	if(c == '\0') {
		reportAndFreeState(false, state, exponentialState, "exponential", "unexpected end of input");
		g_string_free(exponentialString, true);
		return NULL;
	} else if(c != 'e' && c != 'E') {
		reportAndFreeState(false, state, exponentialState, "exponential", "expected 'e' or 'E'");
		g_string_free(exponentialString, true);
		return NULL;
	}

	g_string_append_printf(exponentialString, "%c", c);

	// eat that character
	exponentialState->position.index++;
	exponentialState->position.column++;

	bool isNegative = false;
	c = input[exponentialState->position.index];
	if(c == '\0') {
		reportAndFreeState(false, state, exponentialState, "exponential", "unexpected end of input");
		g_string_free(exponentialString, true);
		return NULL;
	} else if(c == '+' || c == '-') {
		g_string_append_printf(exponentialString, "%c", c);

		// eat that character
		exponentialState->position.index++;
		exponentialState->position.column++;

		if(c == '-') {
			isNegative = true;
		}
	}

	GString *digitsString = parseDigits(input, exponentialState);
	if(digitsString == NULL) {
		reportAndFreeState(false, state, exponentialState, "exponential", "expected digits");
		g_string_free(exponentialString, true);
		return NULL;
	}

	g_string_append_printf(exponentialString, "%s", digitsString->str);
	g_string_free(digitsString, true);

	state->position = exponentialState->position;
	reportAndFreeState(true, state, exponentialState, "exponential", "parsed %s exponential", isNegative ? "negative" : "positive");
	return exponentialString;
}

/**
 * shortstring		: shortstringchar+
 *
 * where
 * shortstringchar	: nonseparator
 */
static GString *parseShortString(const char *input, StoreParseState *state)
{
	StoreParseState *shortStringState = createParseState(state->position);

	int numChars = 0;
	GString *shortString = g_string_new("");
	while(true) {
		char c = input[shortStringState->position.index];
		if(isSeparator(c)) {
			break;
		}

		shortStringState->position.index++;
		shortStringState->position.column++;
		g_string_append_printf(shortString, "%c", c);

		numChars++;
	}

	if(numChars == 0) {
		reportAndFreeState(false, state, shortStringState, "short string", "no short string characters parsed");
		g_string_free(shortString, true);
		return NULL;
	}

	state->position = shortStringState->position;
	reportAndFreeState(true, state, shortStringState, "short string", "parsed short string with %d characters", numChars);
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
static GString *parseLongString(const char *input, StoreParseState *state)
{
	StoreParseState *longStringState = createParseState(state->position);

	int numChars = 0;
	GString *longString = g_string_new("");
	while(true) {
		char c = input[longStringState->position.index];
		if(c == '\\') {
			longStringState->position.index++;
			longStringState->position.column++;

			c = input[longStringState->position.index];
			longStringState->position.index++;
			longStringState->position.column++;

			switch(c) {
				case '"':
					g_string_append_printf(longString, "\"");
				break;
				case '\\':
					g_string_append_printf(longString, "\\");
				break;
				case '/':
					g_string_append_printf(longString, "/");
				break;
				case 'b':
					g_string_append_printf(longString, "\b");
				break;
				case 'f':
					g_string_append_printf(longString, "\f");
				break;
				case 'n':
					g_string_append_printf(longString, "\n");
				break;
				case 'r':
					g_string_append_printf(longString, "\r");
				break;
				case 't':
					g_string_append_printf(longString, "\t");
				break;
				case 'u':
				{
					char u1 = input[longStringState->position.index];
					if(!isHex(u1)) {
						reportAndFreeState(false, state, longStringState, "long string", "expected first hex number of escaped unicode character, but got '%c'", u1);
						g_string_free(longString, true);
						return NULL;
					}
					longStringState->position.index++;
					longStringState->position.column++;

					char u2 = input[longStringState->position.index];
					if(!isHex(u2)) {
						reportAndFreeState(false, state, longStringState, "long string", "expected second hex number of escaped unicode character, but got '%c'", u2);
						g_string_free(longString, true);
						return NULL;
					}
					longStringState->position.index++;
					longStringState->position.column++;

					char u3 = input[longStringState->position.index];
					if(!isHex(u3)) {
						reportAndFreeState(false, state, longStringState, "long string", "expected third hex number of escaped unicode character, but got '%c'", u3);
						g_string_free(longString, true);
						return NULL;
					}
					longStringState->position.index++;
					longStringState->position.column++;

					char u4 = input[longStringState->position.index];
					if(!isHex(u4)) {
						reportAndFreeState(false, state, longStringState, "long string", "expected fourth hex number of escaped unicode character, but got '%c'", u4);
						g_string_free(longString, true);
						return NULL;
					}
					longStringState->position.index++;
					longStringState->position.column++;

					char conversion[5] = {u1, u2, u3, u4, '\0'};
					uint32_t codepoint = strtol(conversion, NULL, 16);
					GString *utf8 = storeConvertUnicodeToUtf8(codepoint);

					g_string_append_printf(longString, "%s", utf8->str);
					g_string_free(utf8, true);
				}
				break;
				default:
					reportAndFreeState(false, state, longStringState, "long string", "expected escaped character, but got '%c'", c);
					g_string_free(longString, true);
					return NULL;
				break;
			}
		} else if(c == '"' || c == '\0') {
			break;
		} else {
			g_string_append_printf(longString, "%c", c);

			longStringState->position.index++;
			longStringState->position.column++;

			if(c == '\n') {
				longStringState->position.line++;
				longStringState->position.column = 1;
			}
		}

		numChars++;
	}

	state->position = longStringState->position;
	reportAndFreeState(true, state, longStringState, "long string", "parsed long string with %d characters", numChars);
	return longString;
}

static char parseTerminal(const char *input, StoreParseState *state)
{
	StoreParseState *terminalState = createParseState(state->position);

	int numDelimiters = 0;
	while(true) {
		char c = input[terminalState->position.index];

		if(!isspace(c) && c != ',' && c != ';') {
			break;
		}

		terminalState->position.index++;
		terminalState->position.column++;

		if(c == '\n') {
			terminalState->position.line++;
			terminalState->position.column = 1;
		}

		numDelimiters++;
	}

	state->position = terminalState->position;
	char terminal = input[terminalState->position.index];
	reportAndFreeState(true, state, terminalState, "terminal", "parsed terminal after %d delimiters", numDelimiters);
	return terminal;
}

static StoreParseState *createParseState(StoreParseStatePosition position)
{
	StoreParseState *state = storeAllocateMemoryType(StoreParseState);
	state->position = position;
	state->reports = g_queue_new();
	return state;
}

static void freeParseState(StoreParseState *state)
{
	g_queue_free_full(state->reports, freeParseReportPointer);
	storeFreeMemory(state);
}

static void reportAndFreeState(bool success, StoreParseState *parentState, StoreParseState *state, const char *type, const char *message, ...)
{
	va_list va;
	va_start(va, message);

	GString *messageString = g_string_new("");
	g_string_append_vprintf(messageString, message, va);

	StoreParseReport *report = storeAllocateMemoryType(StoreParseReport);
	report->success = success;
	report->position = state->position;
	report->type = type;
	report->message = strdup(messageString->str);
	report->subreports = g_queue_new();
	for(GList *iter = state->reports->head; iter != NULL; iter = iter->next) {
		g_queue_push_tail(report->subreports, iter->data);
	}
	g_queue_clear(state->reports);

	g_queue_push_tail(parentState->reports, report);

	g_string_free(messageString, true);

	freeParseState(state);
}

static void freeParseReportPointer(void *parseReportPointer)
{
	StoreParseReport *parseReport = (StoreParseReport *) parseReportPointer;
	freeParseReport(parseReport);
}

static void freeParseReport(StoreParseReport *parseReport)
{
	if(parseReport == NULL) {
		return;
	}

	free(parseReport->message);
	g_queue_free_full(parseReport->subreports, freeParseReportPointer);
	storeFreeMemory(parseReport);
}

static bool isHex(char c)
{
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool isSeparator(char c)
{
	return isspace(c) || c == ',' || c == ';' || c == '"' || c == '(' || c == '[' || c == '{' || c == ')' || c == ']' || c == '}' || c == ':' || c == '=' || c == '\0';
}
