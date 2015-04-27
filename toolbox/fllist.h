#ifndef _FLUX_LINKED_LIST_UTILITY_
#define _FLUX_LINKED_LIST_UTILITY_
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	size_t elementSize;
	void *firstElement;
} FLList;


void FLList_init(FLList *self, size_t elementSize);
void FLList_clean(FLList *self);
void FLList_clear(FLList *self);

int FLList_prepend(FLList *self, void *data);
int FLList_append(FLList *self, void *data);
int FLList_insert(FLList *self, void *data, size_t i);
void FLList_remove(FLList *self, size_t i);
void FLList_removeHead(FLList *self);
void FLList_set(FLList *self, size_t i, void *data);

void *FLList_hook(FLList *self, size_t i);
void *FLList_hookLast(FLList *self);
void FLList_fetch(FLList *self, size_t i, void *dest);
void FLList_fetchLast(FLList *self, void *dest);

size_t FLList_length(FLList *self);
void FLList_sort(FLList *self, int (*cmp)(const void*, const void*));


void FLList_each(FLList *self, void (*function)(void*));
void FLList_reverse(FLList *self);


void *FLList_next(FLList *self, void *item);
void FLList_removeNext(FLList *self, void *item);
int FLList_insertAfter(FLList *self, void *item, void *data);


bool FLList_isEmpty(FLList *self);


#endif