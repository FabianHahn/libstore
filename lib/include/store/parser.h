#ifndef LIBSTORE_PARSER_H
#define LIBSTORE_PARSER_H

#include <store/store.h>

typedef struct {
	int index;
	int line;
	int column;
} StoreParseStatePosition;

typedef struct StoreParseReportStruct {
	bool success;
	StoreParseStatePosition position;
	const char *type;
	char *message;
	/** list of (StoreParseReport *) */
	GQueue *subreports;
} StoreParseReport;

typedef struct StoreParseStateStruct {
	StoreParseStatePosition position;
	int depth;
	/** list of (StoreParseReport *) */
	GQueue *reports;
} StoreParseState;

typedef struct {
	StoreParseState state;
} StoreParser;

StoreParser *storeCreateParser();
void storeResetParser(StoreParser *parser);
void storeFreeParser(StoreParser *parser);
Store *storeParse(StoreParser *parser, const char *input);

#endif
