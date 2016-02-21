#ifndef LIBSTORE_STORE_H
#define LIBSTORE_STORE_H

#include "types.h"

/**
 * Enumeration of the store value types
 */
typedef enum {
	/** A string value */
	STORE_STRING,
	/** An integer value */
	STORE_INTEGER,
	/** A floating point number value */
	STORE_FLOAT_NUMBER,
	/** A list value */
	STORE_LIST,
	/** An associative array value */
	STORE_ARRAY
} StoreValueType;

/**
 * Union to store a node value's content
 */
typedef union {
	/** A string value */
	char *string;
	/** An integer value */
	int integer;
	/** A floating point number value */
	double float_number;
	/** A list value */
	StoreList list;
	/** An associative array value */
	StoreMap array;
} StoreValueContent;

/**
 * Struct to represent a store, respectively a node value in the tree
 */
typedef struct {
	/** The node value's type */
	StoreValueType type;
	/** The node value's content */
	StoreValueContent content;
} Store;

/**
 * Struct to represent a store node
 * Note: This struct is only used internally to do the parsing, it is NOT used in the final parsed store's representation
 */
typedef struct {
	/** The node's key */
	char *key;
	/** The node's value, another store */
	Store *value;
} StoreNode;

#endif
