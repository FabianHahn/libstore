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
