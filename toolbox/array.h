// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#ifndef TOOLBOX_ARRAY_H_
#define TOOLBOX_ARRAY_H_
#include <stdlib.h>


typedef struct {
	size_t num_elems;
	size_t elem_size;
	char *data;
} Array;

// Initializes a preallocated Array.
int Array_init(Array *const arr
	            , const size_t num_elems
	            , const size_t elem_size);

// Cleans Array internals without freeing the structure.
void Array_clean(Array *const arr);

// Allocates, initializes, and returns an Array.
Array *Array_new(const size_t num_elems
	                 , const size_t elem_size);

// Cleans and frees parameter.
void Array_free(Array *const arr);

// Returns pointer to nth element.
void *Array_hook(const Array *const arr
	           , const size_t i);

// Copies data into element.
void Array_set(Array *const arr
             , const size_t i
	         , const void *const ptr);

// Copies data from element.
void Array_fetch(const Array *const arr
	           , const size_t i
	           , void *const dest);

// Sets all Array bytes to zero.
void Array_zero(Array *const arr);

// Copies data from address <val> to each Array element.
void Array_fill(Array *const arr
	          , const void *const val);

// Reverses elements.
void Array_reverse(Array *const arr);

// Calls qsort() internally.
void Array_sort(Array *const arr
	             , int (*cmp)(const void*, const void*));

// Applies <func> to each Array element directly.
void Array_each(Array *const arr
	             , void (*func)(void*));

// Copies all internal data, returning an independent Array.
int Array_copy(const Array *const src
	         , Array *const dest);

// Allocates and returns a copied Array. Returns NULL on failure.
Array *Array_clone(const Array *const arr);

// Returns byte count of internal allocation.
size_t Array_innerSize(const Array *const arr);

// Sets number of elements.
int Array_setCapacity(Array *const arr
	                , const size_t elems);


#endif