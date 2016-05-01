#include <stdlib.h> // free
#include <string.h>

#include "memory.h"
#include "store.h"

Store *StoreCreateStringValue(const char *stringValue)
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_STRING;
	store->content.stringValue = strdup(stringValue);

	return store;
}

Store *StoreCreateIntValue(int intValue)
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_INT;
	store->content.intValue = intValue;

	return store;
}

Store *StoreCreateFloatValue(double floatValue)
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_FLOAT;
	store->content.floatValue = floatValue;

	return store;
}

Store *StoreCreateListValue()
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_LIST;
	store->content.listValue = StoreCreateList();

	return store;
}

Store *StoreCreateMapValue()
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_MAP;
	store->content.mapValue = StoreCreateMap();

	return store;
}

const char *StoreGetTypeName(Store *store)
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

void StoreFree(Store *store)
{
	switch(store->type) {
		case STORE_STRING:
			free(store->content.stringValue);
		break;
		case STORE_LIST:
			StoreFreeList(store->content.listValue);
		break;
		case STORE_MAP:
			StoreFreeMap(store->content.mapValue);
		break;
		default:
			// No need to free ints or doubles
		break;
	}

	StoreFreeMemory(store);
}
