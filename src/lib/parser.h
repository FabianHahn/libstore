#ifndef LIBSTORE_PARSER_H
#define LIBSTORE_PARSER_H

#include "store.h"

typedef enum {
	STORE_PARSE_SUCCESS,
	STORE_PARSE_ERROR
} StoreParseResultStatus;

typedef struct {
	StoreParseResultStatus status;
	Store *store;
	StoreDynamicString *error;
	int line;
	int column;
} StoreParseResult;

StoreParseResult *StoreParse(const char *input);

#endif
