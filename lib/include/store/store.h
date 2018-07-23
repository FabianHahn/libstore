#ifndef LIBSTORE_STORE_H
#define LIBSTORE_STORE_H

#include <glib.h>

#include <store/api.h>

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
	/** A map value */
	STORE_MAP
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
	GQueue *listValue;
	/** An map value */
	GHashTable *mapValue;
} StoreContent;

/**
 * Struct to represent a store, respectively a node value in the tree
 */
typedef struct Store {
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
LIBSTORE_API Store *storeCreateStringValue(const char *stringValue);

/**
 * Creates an int store
 *
 * @param intValue		the content int
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *storeCreateIntValue(int intValue);

/**
 * Creates a float store
 *
 * @param floatValue	the content float
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *storeCreateFloatValue(double floatValue);

/**
 * Creates an empty list store
 *
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *storeCreateListValue();

/**
 * Creates an empty map store
 *
 * @result				the created store, must be freed with StoreFree
 */
LIBSTORE_API Store *storeCreateMapValue();

/**
 * Returns the type name of a store
 *
 * @param store	the store to retrieve the name of
 * @result		the type name of the store, doesn't need to be freed
 */
LIBSTORE_API const char *storeGetTypeName(Store *store);

/**
 * Frees a store
 *
 * @param store		the store to free
 */
LIBSTORE_API void storeFree(Store *store);

#endif
