// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#ifndef TOOLBOX_LIST_H_
#define TOOLBOX_LIST_H_
#include "stdlib.h"

typedef struct ListNode {
	struct ListNode *prev;
	struct ListNode *next;
	void *data;
} ListNode;

typedef struct {
	ListNode *first;
	ListNode *last;
	size_t length;
	size_t elem_size;
} List;

void List_init(List *self, size_t elem_size);
int List_prepend(List *self, void *elem);
int List_append(List *self, void *elem);
int List_insert(List *self, int index, void *elem);
void List_remove(List *self, int index);
void List_empty(List *self);
#define List_clean(a) List_empty(a)
ListNode *List_hookNode(List *self, int index);
void *List_hook(List *self, int index);
void List_fetch(List *self, int index, void *dest);
void List_set(List *self, int index, void *data);
void List_each(List *self, void (*function)(void*));


#endif