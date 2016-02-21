#include <stdlib.h>
#include "memory.h"

void *StoreAllocateMemory(int bytes)
{
	return malloc(bytes);
}
