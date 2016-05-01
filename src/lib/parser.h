#ifndef LIBSTORE_PARSER_H
#define LIBSTORE_PARSER_H

#include "store.h"

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
	struct StoreParseReportStruct *lastSubReport;
	struct StoreParseReportStruct *previousReport;
} StoreParseReport;

typedef struct StoreParseStateStruct {
	StoreParseStatePosition position;
	StoreParseReport *lastReport;
} StoreParseState;

Store *StoreParse(const char *input, StoreParseState *state);

#endif
