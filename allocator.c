#include "allocator.h"
#include <stdlib.h>


void Allocator_init(Allocator *self){
	Vector_init(&self->object_pointers, sizeof(Object*));
}

void Allocator_clean(Allocator *self){
	Allocator_collect(self);
	Vector_clean(&self->object_pointers);
}

Object *Allocator_malloc(Allocator *self){
	Object *object = malloc(sizeof(Object));
	Vector_append(&self->object_pointers, &object); // TODO: check error
	return object;
}



static void unmark(void *object){
	Object_unmark((Object*) object);
}

static void mark_recursive(void *object){
	Object_markRecursive((Object*) object);
}

void Allocator_collect(Allocator *self){
	// Unmark all allocations.
	Vector_each(&self->object_pointers, unmark); // TODO: check error

	// Mark all accessible allocations.
	Vector_each(&self->object_pointers, mark_recursive);

	// Move all accessible allocations to a new vector. Clean
	// the others.
	Vector leftovers;
	Vector_init(&leftovers, sizeof(Object*));
	for(int i = 0; i < self->object_pointers.size; i++){
		Object *object;
		Vector_fetch(&self->object_pointers, i, &object);
		if(object->gc_mark){
			Vector_append(&leftovers, &object);
		} else {
			Object_free(object);
		}
	}

	// Delete old vector of pointers. Replace it.
	Vector_clean(&self->object_pointers);
	self->object_pointers = leftovers;
}
