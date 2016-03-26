#ifndef LIBSTORE_TYPES
#define LIBSTORE_TYPES

#include <stdbool.h> // bool
#include <stdarg.h> // va_list

#include "api.h"

struct Store; // forward declaration

typedef void * StoreDynamicString;
typedef void * StoreList;
typedef void * StoreMap;

LIBSTORE_NO_EXPORT StoreDynamicString StoreCreateDynamicString();
LIBSTORE_NO_EXPORT void StoreAppendDynamicString(StoreDynamicString string, const char *format, ...);
LIBSTORE_NO_EXPORT void StoreAppendDynamicStringV(StoreDynamicString string, const char *format, va_list vp);
LIBSTORE_NO_EXPORT const char *StoreReadDynamicString(StoreDynamicString string);
LIBSTORE_NO_EXPORT void StoreFreeDynamicString(StoreDynamicString string);

LIBSTORE_NO_EXPORT StoreList StoreCreateList();
LIBSTORE_NO_EXPORT void StoreAppendList(StoreList list, struct Store *element);
LIBSTORE_NO_EXPORT void StoreFreeList(StoreList list);

LIBSTORE_NO_EXPORT StoreMap StoreCreateMap();

/**
 * Inserts a store value with a string key into a map, potentially overwriting an existing entry with this key.
 *
 * @param map	the map to insert into
 * @param key	the key of the entry to insert into the map
 * @param value	the value of the entry to insert into the map; the map will take ownership over it and make sure it gets freed
 * @result		true if an entry with this key already existed in the map
 */
LIBSTORE_NO_EXPORT bool StoreInsertMap(StoreMap map, const char *key, struct Store *value);

LIBSTORE_NO_EXPORT void StoreFreeMap(StoreMap map);

#endif
