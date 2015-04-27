#include "fllist.h"
#include "error.h"
#include <string.h>


void FLList_init(FLList *self, size_t elementSize)
{
	self->elementSize = elementSize;
	self->firstElement = NULL;
}


void FLList_clean(FLList *self)
{
	FLList_clear(self);
	self->elementSize = 0;
}


static inline void FLList_setNext(FLList *self, void *node, void *next)
{
	void *dest = (char*) node + self->elementSize;
	memcpy(dest, &next, sizeof(void*));
}


void *FLList_next(FLList *self, void *item)
{
	void *next;
	void *addr = (void*) ((char*) item + self->elementSize);
	memmove(&next, addr, sizeof(void*));
	return next;
}


void FLList_clear(FLList *self)
{
	void *data = self->firstElement;
	while(data){
		void *prev = data;
		data = FLList_next(self, prev);
		free(prev);
	}
}


int FLList_prepend(FLList *self, void *data)
{
	void *node = malloc(self->elementSize + sizeof(void*));
	if(!node) return 1;
	memcpy(node, data, self->elementSize);
	FLList_setNext(self, node, self->firstElement);
	self->firstElement = node;
	return 0;
}


static inline void *newNode(FLList *self, void *data)
{
	void *node = malloc(self->elementSize + sizeof(void*));
	if(!node) return NULL;
	memcpy(node, data, self->elementSize);
	FLList_setNext(self, node, NULL);
	return node;
}


static inline void *nodeBefore(FLList *self, void *next, void *data)
{
	void *node = newNode(self, data);
	FLList_setNext(self, node, next);
	return node;
}


int FLList_insertAfter(FLList *self, void *n0, void *data)
{
	void *n1 = FLList_next(self, n0);
	void *node = nodeBefore(self, n1, data);
	if(node){
		FLList_setNext(self, n0, node);
		return 0;
	}
	return 1;
}


int FLList_append(FLList *self, void *data)
{
	if(self->firstElement){
		return FLList_insertAfter(self
			                    , FLList_hookLast(self)
		             	        , data);
	} else {
		self->firstElement = newNode(self, data);
		return self->firstElement? 0:1;
	}
}



int FLList_insert(FLList *self, void *data, size_t i)
{
	if(i == 0)
		return FLList_prepend(self, data);
	else
		return FLList_insertAfter(self
			                    , FLList_hook(self, i - 1)
			                    , data);
}



void FLList_removeNext(FLList *self, void *n0)
{
	void *n1 = FLList_next(self, n0);
	void *n2 = FLList_next(self, n1);
	FLList_setNext(self, n0, n2);
	free(n1);
}


inline void FLList_removeHead(FLList *self)
{
	void *first = self->firstElement;
	self->firstElement = FLList_next(self, first);
	free(first);
}


void FLList_remove(FLList *self, size_t i)
{
	if(i == 0)
		FLList_removeHead(self);
	else
		FLList_removeNext(self
			            , FLList_hook(self, i - 1));
}


void FLList_set(FLList *self, size_t i, void *source)
{
	void *dest = FLList_hook(self, i);
	memmove(dest, source, self->elementSize);
}


void *FLList_hook(FLList *self, size_t i)
{
	void *node = self->firstElement;
	for(int x = 0; x < i; ++x)
		node = FLList_next(self, node);
	return node;
}


void *FLList_hookLast(FLList *self)
{
	void *node = self->firstElement;
	while(node && FLList_next(self, node))
		node = FLList_next(self, node);
	return node;
}


void FLList_fetch(FLList *self, size_t i, void *dest)
{
	void *source = FLList_hook(self, i);
	memmove(dest, source, self->elementSize);
}


void FLList_fetchLast(FLList *self, void *dest)
{
	void *source = FLList_hookLast(self);
	memmove(dest, source, self->elementSize);
}


void FLList_each(FLList *self, void (*function)(void*))
{
	void *data = self->firstElement;
	while(data){
		function(data);
		data = FLList_next(self, data);
	}
}


size_t FLList_length(FLList *self)
{
	size_t n = 0;
	void *data = self->firstElement;
	while(data){
		++n;
		data = FLList_next(self, data);
	}
	return n;
}


bool FLList_isEmpty(FLList *self)
{
	return self->firstElement? false:true;
}


