#ifndef LIBSTORE_MEMORY_H
#define LIBSTORE_MEMORY_H

#include <store/api.h>

/**
 * Allocate a block of memory on the heap.
 *
 * @param bytes		the amount of memory in bytes to allocate
 * @result			a pointer to the heap block allocated
 */
LIBSTORE_NO_EXPORT void *storeAllocateMemory(int bytes);

/**
 * Frees a previously allocated block of memory on the heap.
 *
 * @param memory	a pointer to the memory to be freed
 */
LIBSTORE_NO_EXPORT void storeFreeMemory(void *memory);

/**
 * Allocate a block of memory on the heap for a specified type.
 *
 * @param type		the type of which to allocate an object
 * @result			a pointer to the heap block allocated
 */
#define storeAllocateMemoryType(TYPE) ((TYPE *) storeAllocateMemory(sizeof(TYPE)))

#endif
