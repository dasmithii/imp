#include "memory.h"
#include <string.h>
#include "debug.h"

// ---------------------------------------------------------- //
void swap(const void *addr1
	    , const void *addr2
	    , size_t size)
{
	assume_ptrs(addr1, addr2);
	char *ptr1 = (char*) addr1;
	char *ptr2 = (char*) addr2;
	for(int b = 0; b < size; ++b){
		const char c = *ptr1;
		*ptr1 = *ptr2;
		*ptr2 = c;
		++ptr1; ++ptr2;
	}
}

// ---------------------------------------------------------- //
void reverse(const void *const addr
	       , const size_t num_elems
	       , const size_t elem_size)
{
	assume_ptr(addr);
	assume_nzero(elem_size);
	for(int i = 0; i < num_elems / 2; ++i){
		void *const p1 = blockIndex(addr, i, elem_size);
		void *const p2 = blockIndex(addr, num_elems - i - 1, elem_size);
		swap(p1, p2, elem_size);
	}
}

// ---------------------------------------------------------- //
void *offset(const void *const addr
           , const int bytes)
{
	assume_ptr(addr);
	return (void*) (((char*) addr) + bytes);
}

// ---------------------------------------------------------- //
void *blockIndex(const void *const addr
             , const size_t index
             , const size_t elem_size)
{
	assume_ptr(addr);
	assume_pos(elem_size);
	return offset(addr, elem_size * index);
}

// ---------------------------------------------------------- //
void zero(void *const addr
	       , const size_t len)
{
	assume_ptr(addr);
	memset(addr, 0, len);
}

// ---------------------------------------------------------- //
void safeFree(void *const ptr){
	if(ptr)
		free(ptr);
}

// ---------------------------------------------------------- //
void *cloneBytes(const void *const ptr
	           , const size_t bytes){
	assume_ptr(ptr);
	void *const ret = malloc(bytes);
	if(!ret)
		return NULL;
	memcpy(ret, ptr, bytes);
	return ret;
}
