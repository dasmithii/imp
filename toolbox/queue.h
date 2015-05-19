// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#ifndef TOOLBOX_QUEUE_H_
#define TOOLBOX_QUEUE_H_
#include <stdlib.h>
#include "list.h"

typedef List Queue;

void Queue_init(Queue *self, size_t elem_size);
int Queue_push(Queue *self, void *elem);
void Queue_pop(Queue *self);
void *Queue_hook(Queue *self);
void Queue_fetch(Queue *self, void *dest);
size_t Queue_size(Queue *self);
#define Queue_empty(self) (Queue_size(self) == 0)

#endif