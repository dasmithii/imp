#include "object.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>


static bool validKey(char *key){
	return key && strlen(key) > 0;
}

static bool keyForInternal(char *key){
	assert(validKey(key));
	return strstr(key, "__") == key;
}

static bool keyForSpecial(char *key){
	assert(validKey(key));
	return strstr(key, "__") == key;
}

static bool keyForRegular(char *key){
	assert(validKey(key));
	return !keyForSpecial(key) && !keyForInternal(key);
}


void Object_init(Object *self){
	assert(self);
	self->slotCount = 0;
	self->slots = NULL;
	self->gc_mark = false;
}


void *Slot_data(Slot *self){
	assert(self);
	assert(Slot_isPrimitive(self));
	return self->data;
}

Object *Slot_object(Slot *self){
	assert(self);
	assert(!Slot_isPrimitive(self));
	return (Object*) self->data;
}

bool Slot_isPrimitive(Slot *self){
	assert(self);
	return 0 == strncmp("__", self->key, 2);
}

void Slot_clean(Slot *self){
	assert(self);
	if(Slot_isPrimitive(self)){
		free(self->data);
	}
	free(self->key);
}


static int Slot_compare(Slot *self, Slot *other){
	assert(self);
	assert(other);
	return strcmp(self->key, other->key);
}

static int Slot_compare_generic(const void *self, const void *other){
	assert(self);
	assert(other);
	return Slot_compare((Slot*) self, (Slot*) other);
}



Slot *Object_getSlotShallow(Object *self, char *key){
	assert(self);
	assert(validKey(key));
	if(self->slotCount == 0){
		return NULL;
	}
	Slot skey = {.key = key};
	return bsearch(&skey
		         , (void*) self->slots
		         , self->slotCount
		         , sizeof(Slot)
		         , Slot_compare_generic);
}


Slot *Object_getSlotDeep(Object *self, char *key){
	assert(self);
	assert(validKey(key));

	Slot *shallow = Object_getSlotShallow(self, key);
	if(shallow){
		return shallow;
	}
	Object *prototype = Object_getShallow(self, "_prototype");
	if(!prototype){
		return NULL;
	}
	return Object_getSlotDeep(prototype, key);
}


Object *Object_getShallow(Object *self, char *key){
	assert(self);
	assert(!keyForInternal(key));


	Slot *slot = Object_getSlotShallow(self, key);
	if(slot){
		return Slot_object(slot);
	}
	return NULL;
}

Object *Object_getDeep(Object *self, char *key){
	assert(self);
	assert(!keyForInternal(key));

	Slot *slot = Object_getSlotDeep(self, key);
	if(!slot){
		return NULL;
	}
	assert(!Slot_isPrimitive(slot));
	return Slot_object(slot);
}


bool Object_hasKeyShallow(Object *self, char *key){
	assert(self);
	assert(validKey(key));

	return Object_getSlotShallow(self, key) != NULL;
}

void *Object_getDataShallow(Object *self, char *key){
	assert(self);
	assert(keyForInternal(key));

	Slot *slot = Object_getSlotShallow(self, key);
	if(!slot){
		return NULL;
	}
	assert(Slot_isPrimitive(slot));
	return slot->data;
}


void *Object_getDataDeep(Object *self, char *key){
	assert(self);
	assert(keyForInternal(key));

	Slot *slot = Object_getSlotDeep(self, key);
	if(!slot){
		return NULL;
	}
	assert(Slot_isPrimitive(slot));
	return slot->data;
}




bool Object_hasKeyDeep(Object *self, char *key){
	assert(self);
	assert(validKey(key));

	return Object_getDeep(self, key) != NULL;
}


static void Object_insertShallow(Object *self, char *key, void *data){
	assert(self);
	assert(validKey(key));

	Slot *existing = Object_getSlotShallow(self, key);
	if(existing){
		existing->data = data;
		return;
	}

	self->slots = realloc(self->slots, (self->slotCount + 1) * sizeof(Slot)); // TODO: check return
	self->slots[self->slotCount].key = strdup(key);
	self->slots[self->slotCount].data = data;
	self->slotCount += 1;
	qsort(self->slots
		, self->slotCount
		, sizeof(Slot)
		, Slot_compare_generic);
}

static void Object_insertDeep(Object *self, char *key, void *data){
	assert(self);
	assert(validKey(key));

	Object *object = self;
	while(object && !Object_hasKeyShallow(object, key)){
		object = Object_getShallow(object, "_prototype");
	}
	if(object && Object_hasKeyShallow(object, key)){
		Object_insertShallow(object, key, data);
	} else {
		Object_insertShallow(self, key, data);
	}
}



void Object_putDataShallow(Object *self, char *key, void *data){
	assert(self);
	assert(keyForInternal(key));

	Object_insertShallow(self, key, data);
}


void Object_putDataDeep(Object *self, char *key, void *data){
	assert(self);
	assert(keyForInternal(key));

	Object_insertDeep(self, key, data);
}



void Object_putShallow(Object *self, char *key, Object *value){
	assert(self);
	assert(!keyForInternal(key));

	Object_insertShallow(self, key, (void*) value);
}


void Object_putDeep(Object *self, char *key, Object *value){
	assert(self);
	assert(!keyForInternal(key));

	Object_insertDeep(self, key, (void*) value);
}


void Object_remShallow(Object *self, char *key){
	assert(self);
	assert(validKey(key));

	Slot *slot = Object_getSlotShallow(self, key);
	if(!slot){
		return;
	}
	Slot_clean(slot);
	int index = (slot - self->slots) / sizeof(Slot);
	for(int i = index; i < self->slotCount - 1; i++){
		self->slots[i] = self->slots[i+1];
	}
	self->slotCount -= 1;
	// TODO: realloc to be smaller, maybe?
}




void Object_mark(Object *self){
	assert(self);
	
	self->gc_mark = true;
}


void Object_unmark(Object *self){
	assert(self);
	self->gc_mark = false;
}


void Object_markRecursive(Object *self){
	assert(self);

	Object_mark(self);
	for(int i = 0; i < self->slotCount; i++){
		Slot *slot = self->slots + i;
		if(!Slot_isPrimitive(slot)){
			Object_markRecursive(Slot_object(slot));
		}
	}
}



void Object_clean(Object *self){
	for(int i = 0; i < self->slotCount; i++){
		Slot_clean(self->slots + i);
	}
	free(self->slots);
	self->slotCount = 0;
}


void Object_free(Object *self){
	Object_clean(self);
	free(self);
}


Object *Object_rootPrototype(Object *self){
	Object *proto = Object_getShallow(self, "_prototype");
	if(!proto){
		return self;
	}
	return Object_rootPrototype(proto);
}


void Object_putKeyShallow(Object *self, char *key){
	Object_insertShallow(self, key, NULL);
}


