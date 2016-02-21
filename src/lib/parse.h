#ifndef LIBSTORE_PARSE_H
#define LIBSTORE_PARSE_H

#include <stdio.h>
#include "store.h"

/**
 * Parser union for bison
 */
typedef union
{
	char *string;
	int integer;
	double float_number;
	Store *value;
	StoreNode *node;
} YYSTYPE;

/**
 * A store reader to retrieve characters from a source
 * Note: The first param has only type void * and not StoreParser to get around C's single pass compilation restrictions
 */
typedef char (StoreReader)(void *parser);

/**
 * A store unreader to push back characters into a source
 * Note: The first param has only type void * and not StoreParser to get around C's single pass compilation restrictions
 */
typedef void (StoreUnreader)(void *parser, int c);

/**
 * Struct to represent a store
 */
typedef struct
{
	union
	{
		/** The store parser's resource */
		void *resource;
		/** The store parser's const resource */
		const void *const_resource;
	};
	/** The store's reader */
	StoreReader *read;
	/** The store's unreader */
	StoreUnreader *unread;
	/** The store to parse to */
	Store *store;
} StoreParser;

#endif
