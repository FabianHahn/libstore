#ifndef LIBSTORE_MEMORY
#define LIBSTORE_MEMORY

#include "api.h"

/**
 * Allocate a block of memory on the heap.
 *
 * @param bytes		the amount of memory in bytes to allocate
 * @result			a pointer to the heap block allocated
 */
LIBSTORE_NO_EXPORT void *StoreAllocateMemory(int bytes);

/**
 * Allocate a block of memory on the heap for a specified type.
 *
 * @param type		the type of which to allocate an object
 * @result			a pointer to the heap block allocated
 */
#define StoreAllocateMemoryType(TYPE) ((TYPE *) StoreAllocateMemory(sizeof(TYPE)))

#endif
