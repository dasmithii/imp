// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#include "queue.h"

void Queue_init(Queue *self, size_t elem_size){
	List_init(self, elem_size);
}

int Queue_push(Queue *self, void *elem){
	return List_append(self, elem);
}

void Queue_pop(Queue *self){
	return List_remove(self, 0);
}

void *Queue_hook(Queue *self){
	return List_hook(self, 0);
}

void Queue_fetch(Queue *self, void *dest){
	List_fetch(self, 0, dest);
}

size_t Queue_size(Queue *self){
	return self->length;
}

