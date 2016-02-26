#ifndef LIBSTORE_TYPES
#define LIBSTORE_TYPES

#include <stdarg.h> // va_list

#include "api.h"

typedef void * StoreDynamicString;
typedef void * StoreList;
typedef void * StoreMap;

LIBSTORE_NO_EXPORT StoreDynamicString StoreCreateDynamicStringType();
LIBSTORE_NO_EXPORT void StoreAppendDynamicString(StoreDynamicString string, const char *format, ...);
LIBSTORE_NO_EXPORT void StoreAppendDynamicStringV(StoreDynamicString string, const char *format, va_list va);
LIBSTORE_NO_EXPORT const char *StoreReadDynamicString(StoreDynamicString string);
LIBSTORE_NO_EXPORT void StoreFreeDynamicStringType(StoreDynamicString string);

LIBSTORE_NO_EXPORT StoreList StoreCreateListType();
LIBSTORE_NO_EXPORT void StoreFreeListType(StoreList list);

LIBSTORE_NO_EXPORT StoreMap StoreCreateMapType();
LIBSTORE_NO_EXPORT void StoreFreeMapType(StoreMap map);

#endif
