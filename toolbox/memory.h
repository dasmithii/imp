#ifndef _BASELINE_MEMORY_UTILITIES_
#define _BASELINE_MEMORY_UTILITIES_
#include <stdlib.h>


// Swaps n-sized blocks of memory at given addresses.
void swap(const void *addr1
	    , const void *addr2
	    , size_t size);

// Reverses memory blocks of given size.
void reverse(const void *const addr
	       , const size_t num_elems
	       , const size_t elem_size);

// Offsets memory address of any pointer type by <n> bytes.
void *offset(const void *const addr
           , const int bytes);

// Indexes into <addr> a though it an array.
void *blockIndex(const void *const addr
               , const size_t index
               , const size_t elem_size);

// Zeros memory block.
void zero(void *const addr
	    , const size_t len);

// Frees pointer unless its value is NULL.
void safeFree(void *const ptr);

// Returns pointer to an independent copy of data pointed
// to by <ptr>. NULL on failure.
void *cloneBytes(const void *const ptr
	           , const size_t bytes);

#endif