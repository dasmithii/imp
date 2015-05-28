// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#include "list.h"
#include "stdlib.h"
#include "assert.h"
#include "strings.h"

static ListNode *List_nodeWith(List *self, void *elem){
	assert(self);
	assert(elem);
	ListNode *ret = malloc(sizeof(ListNode));
	if(!ret){
		return NULL;
	}
	ret->data = malloc(self->elem_size);
	if(!ret->data){
		free(ret);
		return NULL;
	}
	memcpy(ret->data, elem, self->elem_size);
	ret->prev = NULL;
	ret->next = NULL;
	return ret;
}

static ListNode *List_nodeBetween(List *self, void *elem, ListNode *pre, ListNode *trailer){
	assert(self);
	assert(elem);
	ListNode *node = List_nodeWith(self, elem);
	if(!node){
		return NULL;
	}
	node->prev = pre;
	node->next = trailer;
	if(pre){
		pre->next = node;
	}
	if(trailer){
		trailer->prev = node;
	}
	return node;
}

static ListNode *List_nodeBefore(List *self, void *elem, ListNode *trailer){
	return List_nodeBetween(self, elem, NULL, trailer);
}

static ListNode *List_nodeAfter(List *self, void *elem, ListNode *pre){
	return List_nodeBetween(self, elem, pre, NULL);
}

void List_init(List *self, size_t elem_size){
	assert(self);
	self->elem_size = elem_size;
	self->length = 0;
	self->first = NULL;
	self->last = NULL;
}


int List_prepend(List *self, void *elem){
	assert(self);
	assert(elem /* NULL elements not allowed */);

	ListNode *node = List_nodeBefore(self, elem, self->first);
	if(!node){
		return -1;
	}
	self->first = node;
	if(self->length == 0){
		self->last = node;
	}
	self->length++;
	return 0;
}

int List_append(List *self, void *elem){
	assert(self);
	assert(elem /* NULL elements not allowed */);

	ListNode *node = List_nodeAfter(self, elem, self->last);
	if(!node){
		return -1;
	}
	self->last = node;
	if(self->length == 0){
		self->first = node;
	}
	self->length++;
	return 0;
}

int List_insert(List *self, int index, void *elem){
	assert(self);
	assert(index >= 0 && index < self->length);
	if(index == 0){
		return List_prepend(self, elem);
	} else if(index == self->length){
		return List_append(self, elem);
	}
	ListNode *trailer = List_hookNode(self, index);
	ListNode *node = List_nodeBetween(self, elem, trailer->prev, trailer);
	if(!node){
		return 1;
	}
	self->length++;
	return 0;
}

void List_remove(List *self, int index){
	assert(self);
	assert(index >= 0 && index < self->length);
	ListNode *node = List_hookNode(self, index);
	if(node->next){
		node->next->prev = node->prev;
	}
	if(node->prev){
		node->prev->next = node->next;
	}
	if(index == 0){
		self->first = node->next;
	} else if(index == self->length - 1){
		self->last = node->prev;
	}
	free(node->data);
	free(node);
	self->length--;
}

void List_empty(List *self){
	while(self->length > 0){
		List_remove(self, 0);
	}
}

ListNode *List_hookNode(List *self, int index){
	assert(self);
	assert(index >= 0 && index < self->length);
	ListNode *node;
	if(index < self->length/2){
		node = self->first;
		while(index){
			node = node->next;
			index--;
		}
	} else{
		node = self->last;
		for(int i = 0; i < self->length - index - 1; ++i){
			node = node->prev;
		}
	}
	return node;
}

void *List_hook(List *self, int index){
	return List_hookNode(self, index)->data;
}

void List_fetch(List *self, int index, void *dest){
	assert(self);
	assert(index >= 0 && index < self->length);
	assert(dest);
	void *data = List_hook(self, index);
	memcpy(dest, data, self->elem_size);
}


void List_set(List *self, int index, void *data){
	assert(self);
	assert(index >= 0 && index < self->length);
	assert(data);
	ListNode *node = List_hookNode(self, index);
	memcpy(node->data, data, self->elem_size);
}

void List_each(List *self, void (*function)(void*)){
	ListNode *node = self->first;
	while(node){
		function(node->data);
		node = node->next;
	}
}
