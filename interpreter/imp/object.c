#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <imp/object.h>
#include <imp/lexer.h>




static bool validKey(char *key){
	return iIsValidRouteText(key);
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


void iObject_init(iObject *self){
	assert(self);
	self->slotCount = 0;
	self->slots = NULL;
	self->gc_mark = false;
	self->refcount = 0;
}


void *iSlot_data(iSlot *self){
	assert(self);
	assert(iSlot_isPrimitive(self));
	return self->data;
}


iObject *iSlot_object(iSlot *self){
	assert(self);
	assert(!iSlot_isPrimitive(self));
	return (iObject*) self->data;
}


bool iSlot_isPrimitive(iSlot *self){
	assert(self);
	return self->key[0] == '_' && self->key[1] == '_';
}


void iSlot_clean(iSlot *self){
	assert(self);
	if(iSlot_isPrimitive(self) && self->data){
		free(self->data);
		self->data = NULL;
	}
	free(self->key);
	self->key = NULL;
	self->data = NULL;
}


static int iSlot_compare(iSlot *self, iSlot *other){
	assert(self);
	assert(other);
	assert(validKey(self->key));
	assert(validKey(other->key));
	return strcmp(self->key, other->key);
}


static int iSlot_compare_generic(const void *self, const void *other){
	assert(self);
	assert(other);
	return iSlot_compare((iSlot*) self, (iSlot*) other);
}


iSlot *iObject_getSlotShallow(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(validKey(key));
	if(self->slotCount == 0){
		return NULL;
	}
	iSlot skey = {.key = key};
	return bsearch(&skey
		         , (void*) self->slots
		         , self->slotCount
		         , sizeof(iSlot)
		         , iSlot_compare_generic);
}


iSlot *iObject_getSlotDeep(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(validKey(key));

	iSlot *shallow = iObject_getSlotShallow(self, key);
	if(shallow){
		return shallow;
	}
	iObject *prototype = iObject_getShallow(self, "#");
	if(!prototype){
		return NULL;
	}
	return iObject_getSlotDeep(prototype, key);
}


iObject *iObject_getShallow(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(!keyForInternal(key));


	iSlot *slot = iObject_getSlotShallow(self, key);
	if(slot){
		return iSlot_object(slot);
	}
	return NULL;
}


iObject *iObject_getDeep(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(!keyForInternal(key));

	iSlot *slot = iObject_getSlotDeep(self, key);
	if(!slot){
		return NULL;
	}
	return iSlot_object(slot);
}


bool iObject_hasKeyShallow(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(validKey(key));

	return iObject_getSlotShallow(self, key) != NULL;
}


void *iObject_getDataShallow(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(keyForInternal(key));

	iSlot *slot = iObject_getSlotShallow(self, key);
	if(!slot){
		return NULL;
	}
	return slot->data;
}


void *iObject_getDataDeep(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(keyForInternal(key));

	iSlot *slot = iObject_getSlotDeep(self, key);
	if(!slot){
		return NULL;
	}
	return slot->data;
}


bool iObject_hasKeyDeep(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(validKey(key));

	return iObject_getSlotDeep(self, key) != NULL;
}


static void iObject_insertShallow(iObject *self, char *key, void *data){
	assert(iObject_isValid(self));
	assert(validKey(key));

	iSlot *existing = iObject_getSlotShallow(self, key);
	if(existing){
		if(iSlot_isPrimitive(existing) && existing->data){
			free(existing->data);
		}
		existing->data = data;
		return;
	}

	self->slots = realloc(self->slots, (self->slotCount + 1) * sizeof(iSlot)); // TODO: check return
	self->slots[self->slotCount].key = strdup(key);
	self->slots[self->slotCount].data = data;
	self->slotCount += 1;
	qsort(self->slots
		, self->slotCount
		, sizeof(iSlot)
		, iSlot_compare_generic);
}


static void iObject_insertDeep(iObject *self, char *key, void *data){
	assert(iObject_isValid(self));
	assert(validKey(key));

	iObject *object = self;
	while(object){
		if(iObject_hasKeyShallow(object, key)){
			iObject_insertShallow(object, key, data);
			return;
		}
		object = iObject_getShallow(object, "#");
	} 

	iObject_insertShallow(self, key, data);
}


void iObject_putDataShallow(iObject *self, char *key, void *data){
	assert(iObject_isValid(self));
	assert(keyForInternal(key));

	iObject_insertShallow(self, key, data);

	assert(iObject_isValid(self));
}


void iObject_putDataDeep(iObject *self, char *key, void *data){
	assert(iObject_isValid(self));
	assert(keyForInternal(key));

	iObject_insertDeep(self, key, data);

	assert(iObject_isValid(self));
}


void iObject_putShallow(iObject *self, char *key, iObject *value){
	assert(iObject_isValid(self));
	assert(iObject_isValid(value));
	assert(!keyForInternal(key));
	assert(key[0] != '#' || key[1] != 0 || self != value);
	iObject_insertShallow(self, key, value);
}


void iObject_putDeep(iObject *self, char *key, iObject *value){
	assert(iObject_isValid(self));
	assert(iObject_isValid(value));
	assert(!keyForInternal(key));

	iObject_insertDeep(self, key, value);

	assert(iObject_isValid(self));
}


void iObject_remShallow(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(validKey(key));

	iSlot *slot = iObject_getSlotShallow(self, key);
	if(!slot){
		printf("FUCK\n");
		abort();
		return;
	}
	iSlot_clean(slot);
	int index = slot - self->slots;
	for(int i = index; i < self->slotCount - 1; i++){
		self->slots[i] = self->slots[i+1];
	}
	self->slotCount -= 1;

	if(self->slotCount == 0){
		free(self->slots);
		self->slots = NULL;
	} else {
		self->slots = realloc(self->slots, self->slotCount * sizeof(iSlot));
	}

	assert(iObject_isValid(self));
}


void iObject_mark(iObject *self){
	assert(iObject_isValid(self));
	self->gc_mark = true;
}


void iObject_unmark(iObject *self){
	self->gc_mark = false;
}


void iObject_clean(iObject *self){
	assert(iObject_isValid(self));

	if(self->slotCount > 0){
		for(int i = 0; i < self->slotCount; i++){
			iSlot_clean(self->slots + i);
		}

		free(self->slots);
		self->slots = NULL;
		self->slotCount = 0;
	}
}


void iObject_free(iObject *self){
	assert(iObject_isValid(self));

	iObject_clean(self);
	free(self);
}


iObject *iObject_prototype(iObject *self){
	return iObject_getShallow(self, "#");
}


iObject *iObject_rootPrototype(iObject *self){
	assert(iObject_isValid(self));

	iObject *proto = iObject_getShallow(self, "#");
	if(!proto){
		return self;
	}
	return iObject_rootPrototype(proto);
}


void iObject_putKeyShallow(iObject *self, char *key){
	assert(iObject_isValid(self));
	assert(validKey(key));
	iObject_insertShallow(self, key, NULL);
}


bool iSlot_isValid(iSlot *self){
	if(!self){
		return false;
	}
	if(!validKey(self->key)){
		return false;
	}
	// if(!iSlot_isPrimitive(self) && !self->data){  //TODO?: make recursive?
	// 	return false;
	// }
	return true;
}


bool iObject_isValid(iObject *self){
	if(!self){
		return true;
	}
	for(int i = 0; i < self->slotCount; i++){
		if(!iSlot_isValid(self->slots + i)){
			return false;
		}
	}
	return true;
}


void iObject_reference(iObject *self){
	if(!self){
		return;
	}
	assert(iObject_isValid(self));
	self->refcount++;
}


void iObject_unreference(iObject *self){
	if(!self){
		return;
	}
	assert(iObject_isValid(self));
	assert(self->refcount >= 1);
	self->refcount--;
}


int iObject_referenceCount(iObject *self){
	assert(iObject_isValid(self));
	return (int) self->refcount;
}


void iObject_print(iObject *self){
	assert(iObject_isValid(self));
	printf("iObject %p\n", self);
	for(int i = 0; i < self->slotCount; i++){
		printf(" - %s: %p", self->slots[i].key, self->slots[i].data);
		if(i < self->slotCount - 1){
			printf("\n");
		}
	}
}


bool iObject_canBeActivated(iObject *self){
	assert(iObject_isValid(self));
	return iObject_hasKeyDeep(self, "_activate") ||
	       iObject_hasKeyDeep(self, "__activate");
}

bool iObject_hasSpecialMethod(iObject *self, char *name){
	char _prefixed[64];
	sprintf(_prefixed, "_%s", name);
	return iObject_hasMethod(self, _prefixed);
}

bool iObject_hasMethod(iObject *self, char *name){
	iObject *deep = iObject_getDeep(self, name);
	if(deep){
		return iObject_canBeActivated(deep);
	} else if(*name != '_'){
		return iObject_hasSpecialMethod(self, name);
	}
	return false;
}
