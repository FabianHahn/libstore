#ifndef LIBSTORE_STORE_H
#define LIBSTORE_STORE_H

#include "api.h"
#include "types.h"

/**
 * Enumeration of the store value types
 */
typedef enum {
	/** A string value */
	STORE_STRING,
	/** An integer value */
	STORE_INT,
	/** A floating point number value */
	STORE_FLOAT,
	/** A list value */
	STORE_LIST,
	/** A struct value */
	STORE_STRUCT
} StoreType;

/**
 * Union to store a node value's content
 */
typedef union {
	/** A string value */
	char *stringValue;
	/** An integer value */
	int intValue;
	/** A floating point number value */
	double floatValue;
	/** A list value */
	StoreList listValue;
	/** An struct value */
	StoreMap structValue;
} StoreContent;

/**
 * Struct to represent a store, respectively a node value in the tree
 */
typedef struct {
	/** The store's type */
	StoreType type;
	/** The store's content */
	StoreContent content;
} Store;

/**
 * Creates a string store
 *
 * @param stringValue	the content string
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *StoreCreateString(const char *stringValue);

/**
 * Creates an int store
 *
 * @param intValue		the content int
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *StoreCreateInt(int intValue);

/**
 * Creates a float store
 *
 * @param floatValue	the content float
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *StoreCreateFloat(double floatValue);

/**
 * Creates an empty list store
 *
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *StoreCreateList();

/**
 * Creates an empty struct store
 *
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *StoreCreateStruct();


/**
 * A GDestroyNotify function to free a store node value
 *
 * @param store		the store to free
 */
LIBSTORE_API void StoreFree(Store *store);

#endif
