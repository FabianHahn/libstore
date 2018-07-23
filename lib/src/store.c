#include <stdlib.h> // free
#include <string.h>

#include "store/memory.h"
#include "store/store.h"

static void freeStore(void *storePointer);

Store *storeCreateStringValue(const char *stringValue)
{
	Store *store = storeAllocateMemoryType(Store);
	store->type = STORE_STRING;
	store->content.stringValue = strdup(stringValue);

	return store;
}

Store *storeCreateIntValue(int intValue)
{
	Store *store = storeAllocateMemoryType(Store);
	store->type = STORE_INT;
	store->content.intValue = intValue;

	return store;
}

Store *storeCreateFloatValue(double floatValue)
{
	Store *store = storeAllocateMemoryType(Store);
	store->type = STORE_FLOAT;
	store->content.floatValue = floatValue;

	return store;
}

Store *storeCreateListValue()
{
	Store *store = storeAllocateMemoryType(Store);
	store->type = STORE_LIST;
	store->content.listValue = g_queue_new();

	return store;
}

Store *storeCreateMapValue()
{
	Store *store = storeAllocateMemoryType(Store);
	store->type = STORE_MAP;
	store->content.mapValue = g_hash_table_new_full(g_str_hash, g_str_equal, free, freeStore);

	return store;
}

const char *storeGetTypeName(Store *store)
{
	switch(store->type) {
		case STORE_STRING:
			return "string";
		case STORE_INT:
			return "int";
		case STORE_FLOAT:
			return "float";
		case STORE_LIST:
			return "list";
		case STORE_MAP:
			return "map";
		default:
			return "<invalid store type>";
	}
}

void storeFree(Store *store)
{
	switch(store->type) {
		case STORE_STRING:
			free(store->content.stringValue);
		break;
		case STORE_LIST:
			g_queue_free_full(store->content.listValue, freeStore);
		break;
		case STORE_MAP:
			g_hash_table_destroy(store->content.mapValue);
		break;
		default:
			// No need to free ints or doubles
		break;
	}

	storeFreeMemory(store);
}

static void freeStore(void *storePointer)
{
	Store *store = (Store *) storePointer;
	storeFree(store);
}
