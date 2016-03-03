#ifndef LIBSTORE_TYPES
#define LIBSTORE_TYPES

#include <stdarg.h> // va_list

#include "api.h"

struct Store; // forward declaration

typedef void * StoreDynamicString;
typedef void * StoreList;
typedef void * StoreMap;

LIBSTORE_NO_EXPORT StoreDynamicString StoreCreateDynamicString();
LIBSTORE_NO_EXPORT void StoreAppendDynamicString(StoreDynamicString string, const char *format, ...);
LIBSTORE_NO_EXPORT void StoreAppendDynamicStringV(StoreDynamicString string, const char *format, va_list va);
LIBSTORE_NO_EXPORT const char *StoreReadDynamicString(StoreDynamicString string);
LIBSTORE_NO_EXPORT void StoreFreeDynamicString(StoreDynamicString string);

LIBSTORE_NO_EXPORT StoreList StoreCreateList();
LIBSTORE_NO_EXPORT void StoreAppendList(StoreList list, struct Store *element);
LIBSTORE_NO_EXPORT void StoreFreeList(StoreList list);

LIBSTORE_NO_EXPORT StoreMap StoreCreateMap();
LIBSTORE_NO_EXPORT void StoreInsertMap(StoreMap map, const char *key, struct Store *value);
LIBSTORE_NO_EXPORT void StoreFreeMap(StoreMap map);

#endif