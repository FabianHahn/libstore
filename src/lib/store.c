#include <string.h>

#include "memory.h"
#include "store.h"

Store *StoreCreateString(const char *stringValue)
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_STRING;
	store->content.stringValue = strdup(stringValue);

	return store;
}

Store *StoreCreateInt(int intValue)
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_INT;
	store->content.intValue = intValue;

	return store;
}

Store *StoreCreateFloat(double floatValue)
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_FLOAT;
	store->content.floatValue = floatValue;

	return store;
}

Store *StoreCreateList()
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_LIST;
	store->content.listValue = StoreCreateListType();

	return store;
}

Store *StoreCreateStruct()
{
	Store *store = StoreAllocateMemoryType(Store);
	store->type = STORE_STRUCT;
	store->content.structValue = StoreCreateMapType();

	return store;
}

void StoreFree(Store *store)
{
	switch(store->type) {
		case STORE_STRING:
			free(store->content.stringValue);
		break;
		case STORE_LIST:
			StoreFreeListType(store->content.listValue);
		break;
		case STORE_STRUCT:
			StoreFreeMapType(store->content.structValue);
		break;
		default:
			// No need to free ints or doubles
		break;
	}

	StoreFreeMemory(store);
}
