#ifndef BASELINE_CIRCLE
#define BASELINE_CIRCLE
#include <stdlib.h>
#include <string.h>
#include "fllist.h"


// Represents a circularly-linked list. It's a thin wrapper 
// around the regular forward list.
typedef struct {
	FLList list;
	void *item;
} Circle;


// Main API.
void *Circle_next(Circle *self);
void Circle_init(Circle *self, size_t elementSize);
void Circle_clean(Circle *self);
void *Circle_hook(Circle *self);
void Circle_fetch(Circle *self, void *dest);
void Circle_shift(Circle *self);
int Circle_insert(Circle *self, void *data);
int Circle_append(Circle *self, void *data);
void Circle_remove(Circle *self);
void Circle_each(Circle *self, void (*function)(void*));
size_t Circle_length(Circle *self);

bool Circle_isEmpty(Circle *self);

#endif