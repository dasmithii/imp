#ifndef IMP_OBJECT_H_
#define IMP_OBJECT_H_
#include <stdbool.h>
#include <stdlib.h>


typedef struct {
	char *key;
	void *data; 
} Slot;


typedef struct Object {
	Slot *slots;
	int slotCount;
	bool gc_mark;
} Object;



Object *Slot_object(Slot *self);
void *Slot_data(Slot *self);
bool Slot_isPrimitive(Slot *self);
void Slot_clean(Slot *self);
void Slot_setFunction(Slot *self, Object*(*f)(int argc, Object *argv));

Slot *Object_getSlotShallow(Object *self, char *key);
Slot *Object_getSlotDeep(Object *self, char *key);
Object *Object_getShallow(Object *self, char *key);
Object *Object_getDeep(Object *self, char *key);
void *Object_getDataShallow(Object *self, char *key);
void *Object_getDataDeep(Object *self, char *key);
bool Object_hasKeyShallow(Object *self, char *key);
bool Object_hasKeyDeep(Object *self, char *key);
void Object_putShallow(Object *self, char *key, Object *value);
void Object_putDeep(Object *self, char *key, Object *value);
void Object_putDataShallow(Object *self, char *key, void *value);
void Object_putDataDeep(Object *self, char *key, void *value);
void Object_remShallow(Object *self, char *key);

void Object_mark(Object *self);
void Object_unmark(Object *self);
void Object_markRecursive(Object *self);

void Object_init(Object *self);
void Object_clean(Object *self);
void Object_free(Object *self);



#endif