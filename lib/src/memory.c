#include <stdlib.h> // malloc free

#include "store/memory.h"

void *storeAllocateMemory(int bytes)
{
	return malloc(bytes);
}

void storeFreeMemory(void *memory)
{
	free(memory);
}
