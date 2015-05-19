// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#include "heap.h"
#include "memory.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>


int Heap_init(Heap *self
	        , size_t elementSize
	        , int(*compareFunction)(const void*, const void*))
{
	self->elementCount = 0;
	self->elementSize = elementSize;
	self->compareFunction = compareFunction;
	self->data = malloc(elementSize);
	return self->data? 0:1;
}


void Heap_clean(Heap *self)
{
	free(self->data);
	self->elementCount = 0;
	self->elementSize = 0;
	self->data = NULL;
	self->compareFunction = NULL;
}





static inline bool isPowerOf2(size_t n)
{
	if(n == 0)
		return false;
	return (n & (n - 1)) == 0;
}


static inline bool isSumOfPowersOf2(size_t n)
{
	return isPowerOf2(n + 1);
}


// Assumes <n> is a SoPo2. f(1) = 1. f(3) = 2.
static inline size_t sumOfPowerOfTwoIndex(size_t n)
{
	size_t ret = 0;
	while(n){
		n >>= 1;
		++ret;
	}
	return ret;
}


// Assumes <n> is a SoPo2.
static inline size_t nextSumOfPowersOf2(size_t n)
{
	size_t index = sumOfPowerOfTwoIndex(n);
	return n + pow(2, index);
}


static int addHeapLayer(Heap *self)
{
	size_t bufferSize = nextSumOfPowersOf2(self->elementCount);
	size_t allocationSize = bufferSize * self->elementSize;
	void *data = realloc(self->data, allocationSize);
	if(data){
		self->data = data;
		return 0;
	}
	return 1;
}


static int ensureSpace(Heap *self)
{
	if(self->elementCount != 0 && isSumOfPowersOf2(self->elementCount))
		return addHeapLayer(self);
	return 0;
}


static inline void *heapItemAddress(Heap *self, size_t i)
{
	return (void*) (self->data + i * self->elementSize);
}


static inline void *nextHeapSlot(Heap *self)
{
	return heapItemAddress(self, self->elementCount);
}


static inline size_t parentIndex(size_t i)
{
	if(i % 2)
		++i;
	return i / 2 - 1;
}


static inline bool ordered(Heap *self, size_t i, size_t j)
{
	const void *p1 = heapItemAddress(self, i);
	const void *p2 = heapItemAddress(self, j);
	return self->compareFunction(p1, p2) >= 0;
}


static void swapHeapItems(Heap *self, size_t i, size_t j)
{
	const void *p1 = heapItemAddress(self, i);
	const void *p2 = heapItemAddress(self, j);
	swap(p1, p2, self->elementSize);
}

static void heapifyIndex(Heap *self, size_t i)
{
	if(i){
		size_t j = parentIndex(i);
		if(!ordered(self, j, i)){
			swapHeapItems(self, j, i);
			heapifyIndex(self, j);
		}
	}	
}


static void heapifyLast(Heap *self)
{
	heapifyIndex(self, self->elementCount - 1);
}


int Heap_insert(Heap *self
	         , const void *address)
{
	if(ensureSpace(self))
		return 1;
	void *dest = nextHeapSlot(self);
	memcpy(dest, address, self->elementSize);
	self->elementCount++;
	heapifyLast(self);
	return 0;
}


static inline size_t leftChild(size_t parent)
{
	return 2 * parent + 1;
}


static inline size_t rightChild(size_t parent)
{
	return leftChild(parent) + 1;
}


static void downwardHeapify(Heap *self, size_t root)
{	
	while(root < self->elementCount){
		size_t left = leftChild(root);
		size_t right = rightChild(root);
		size_t greatest = root;

		if(left < self->elementCount && ordered(self, left, greatest))
			greatest = left;
		if(right < self->elementCount && ordered(self, right, greatest))
			greatest = right;

		if(root == greatest){
			break;
		} else{
			swapHeapItems(self, root, greatest);
			root = greatest;
		}
	}
}


int Heap_removeTop(Heap *self)
{
	void *top = Heap_hookTop(self);
	void *end = heapItemAddress(self, self->elementCount - 1);
	memmove(top, end, self->elementSize);
	self->elementCount--;
	downwardHeapify(self, 0);
	// TODO: consolidate memory
	return 0;
}


void *Heap_hookTop(Heap *self)
{
	return (void*) self->data;
}


void Heap_fetchTop(Heap *self
	             , void *destination)
{
	memcpy(destination, self->data, self->elementSize);
}


void Heap_foreach(Heap *self, void (*function)(const void *))
{
	for(int i = 0; i < self->elementCount; ++i){
		const void *addr = heapItemAddress(self, i);
		function(addr);
	}
}


bool Heap_isEmpty(Heap *self)
{
	return self->elementCount? false:true;
}

