// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#ifndef TOOLBOX_HEAP_H_
#define TOOLBOX_HEAP_H_
#include <stddef.h>
#include <stdbool.h>


typedef struct {
	size_t elementCount;
	size_t elementSize;
	char *data;
	int (*compareFunction)(const void*, const void*);
} Heap;


// Initialize empty heap.
int Heap_init(Heap *self
	        , size_t elementSize
	        , int(*compareFunction)(const void*, const void*));


// Frees all internal data, invalidating the heap.
void Heap_clean(Heap *self);


// Inserts data from <address>, maintaining the heap property.
int Heap_insert(Heap *self
	         , const void *address);


// Pops top element.
int Heap_removeTop(Heap *self);


// Returns address of first element.
void *Heap_hookTop(Heap *self);


// Copies data from first element to <destination> address.
void Heap_fetchTop(Heap *self
	             , void *destination);


// Applies function to all elements. This function should not
// mutate data, as that will invalidate the heap.
void Heap_foreach(Heap *self, void (*function)(const void *));


// 
bool Heap_isEmpty(Heap *self);

#endif