#include "object.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>


static bool validKey(char *key){
	if(!key){
		return false;
	}
	int len = strlen(key);
	if(len == 0){
		return false;
	}
	for(int i = 0; i < len; i++){
		if(!isalnum(key[i]) && key[i] != '_'){
			return false;
		}
	}
	return true;
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
	if(Slot_isPrimitive(self) && self->data){
		free(self->data);
	}
	free(self->key);
	self->key = NULL;
	self->data = NULL;
}


static int Slot_compare(Slot *self, Slot *other){
	assert(self);
	assert(other);
	assert(validKey(self->key));
	assert(validKey(other->key));
	return strcmp(self->key, other->key);
}

static int Slot_compare_generic(const void *self, const void *other){
	assert(self);
	assert(other);
	return Slot_compare((Slot*) self, (Slot*) other);
}



Slot *Object_getSlotShallow(Object *self, char *key){
	assert(Object_isValid(self));
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
	assert(Object_isValid(self));
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
	assert(Object_isValid(self));
	assert(!keyForInternal(key));


	Slot *slot = Object_getSlotShallow(self, key);
	if(slot){
		return Slot_object(slot);
	}
	return NULL;
}

Object *Object_getDeep(Object *self, char *key){
	assert(Object_isValid(self));
	assert(!keyForInternal(key));

	Slot *slot = Object_getSlotDeep(self, key);
	if(!slot){
		return NULL;
	}
	return Slot_object(slot);
}


bool Object_hasKeyShallow(Object *self, char *key){
	assert(Object_isValid(self));
	assert(validKey(key));

	return Object_getSlotShallow(self, key) != NULL;
}

void *Object_getDataShallow(Object *self, char *key){
	assert(Object_isValid(self));
	assert(keyForInternal(key));

	Slot *slot = Object_getSlotShallow(self, key);
	if(!slot){
		return NULL;
	}
	return slot->data;
}


void *Object_getDataDeep(Object *self, char *key){
	assert(Object_isValid(self));
	assert(keyForInternal(key));

	Slot *slot = Object_getSlotDeep(self, key);
	if(!slot){
		return NULL;
	}
	return slot->data;
}




bool Object_hasKeyDeep(Object *self, char *key){
	assert(Object_isValid(self));
	assert(validKey(key));

	return Object_getSlotDeep(self, key) != NULL;
}


static void Object_insertShallow(Object *self, char *key, void *data){
	assert(Object_isValid(self));
	assert(validKey(key));

	Slot *existing = Object_getSlotShallow(self, key);
	if(existing){
		if(Slot_isPrimitive(existing) && existing->data){
			free(existing->data);
		}
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

	assert(Object_isValid(self));
}

static void Object_insertDeep(Object *self, char *key, void *data){
	assert(Object_isValid(self));
	assert(validKey(key));

	Object *object = self;
	while(object){
		if(Object_hasKeyShallow(object, key)){
			Object_insertShallow(object, key, data);
			return;
		}
		object = Object_getShallow(object, "_prototype");
	} 

	Object_insertShallow(self, key, data);
}



void Object_putDataShallow(Object *self, char *key, void *data){
	assert(Object_isValid(self));
	assert(keyForInternal(key));

	Object_insertShallow(self, key, data);

	assert(Object_isValid(self));
}


void Object_putDataDeep(Object *self, char *key, void *data){
	assert(Object_isValid(self));
	assert(keyForInternal(key));

	Object_insertDeep(self, key, data);

	assert(Object_isValid(self));
}



void Object_putShallow(Object *self, char *key, Object *value){
	assert(Object_isValid(self));
	assert(Object_isValid(value));
	assert(!keyForInternal(key));

	Object_insertShallow(self, key, value);

	assert(Object_isValid(self));
}


void Object_putDeep(Object *self, char *key, Object *value){
	assert(Object_isValid(self));
	assert(Object_isValid(value));
	assert(!keyForInternal(key));

	Object_insertDeep(self, key, value);

	assert(Object_isValid(self));
}


void Object_remShallow(Object *self, char *key){
	assert(Object_isValid(self));
	assert(validKey(key));

	Slot *slot = Object_getSlotShallow(self, key);
	if(!slot){
		printf("FUCK\n");
		abort();
		return;
	}
	Slot_clean(slot);
	int index = slot - self->slots;
	for(int i = index; i < self->slotCount - 1; i++){
		self->slots[i] = self->slots[i+1];
	}
	self->slotCount -= 1;

	if(self->slotCount == 0){
		free(self->slots);
		self->slots = NULL;
	} else {
		self->slots = realloc(self->slots, self->slotCount * sizeof(Slot));
	}

	assert(Object_isValid(self));
}




void Object_mark(Object *self){
	assert(Object_isValid(self));
	
	self->gc_mark = true;
}


void Object_unmark(Object *self){
	assert(Object_isValid(self));
	self->gc_mark = false;
}


void Object_markRecursive(Object *self){
	assert(Object_isValid(self));

	Object_mark(self);
	for(int i = 0; i < self->slotCount; i++){
		Slot *slot = self->slots + i;
		if(!Slot_isPrimitive(slot)){
			Object *obj = Slot_object(slot);
			if(obj->gc_mark == false){
				Object_markRecursive(obj);
			}
		}
	}

	assert(Object_isValid(self));
}



void Object_clean(Object *self){
	assert(Object_isValid(self));

	if(self->slotCount > 0){
		for(int i = 0; i < self->slotCount; i++){
			Slot_clean(self->slots + i);
		}

		free(self->slots);
		self->slots = NULL;
		self->slotCount = 0;
	}
}


void Object_free(Object *self){
	assert(Object_isValid(self));

	Object_clean(self);
	free(self);
}


Object *Object_rootPrototype(Object *self){
	assert(Object_isValid(self));

	Object *proto = Object_getShallow(self, "_prototype");
	if(!proto){
		return self;
	}
	return Object_rootPrototype(proto);
}


void Object_putKeyShallow(Object *self, char *key){
	assert(Object_isValid(self));
	assert(validKey(key));
	Object_insertShallow(self, key, NULL);
}

bool Slot_isValid(Slot *self){
	if(!self){
		return false;
	}
	if(!validKey(self->key)){
		return false;
	}
	if(!Slot_isPrimitive(self) && !self->data){  //TODO?: make recursive?
		return false;
	}
	return true;
}

bool Object_isValid(Object *self){
	if(!self){
		return false;
	}
	for(int i = 0; i < self->slotCount; i++){
		if(!Slot_isValid(self->slots + i)){
			return false;
		}
	}
	return true;
}


void Object_reference(Object *self){
	assert(Object_isValid(self));

	if(!Object_hasKeyShallow(self, "__referenceCount")){
		int *data = malloc(sizeof(int));
		*data = 1;
		Object_putDataShallow(self, "__referenceCount", data);
	} else {
		int *data = Object_getDataShallow(self, "__referenceCount");
		*data += 1;
	}
}

void Object_unreference(Object *self){
	assert(Object_isValid(self));
	assert(Object_hasKeyShallow(self, "__referenceCount"));

	int *data = Object_getDataShallow(self, "__referenceCount");
	*data -= 1;
	assert((*data) >= 0);
	if(*data == 0){
		Object_remShallow(self, "__referenceCount");
	}
}

int Object_referenceCount(Object *self){
	assert(Object_isValid(self));
	if(!Object_hasKeyShallow(self, "__referenceCount")){
		return 0;
	}
	return *((int*) Object_getDataShallow(self, "__referenceCount"));
}

