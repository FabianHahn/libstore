#include <stdlib.h>

#include "store/memory.h"

void *StoreAllocateMemory(int bytes)
{
	return malloc(bytes);
}

void *StoreFreeMemory(void *memory)
{
	free(memory);
}
