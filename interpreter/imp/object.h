// Imp is a uni-typed/duck-typed language. Everything is
// an object, and objects are nothing more than
// collections of nested objects (and internal data, in
// some cases).
//
// Objects are collections of slots, essentially, slots
// being key-value pairs. 
//
// There are three types of slots:
//   1. regular    (e.g. objectName:slotName)
//   2. special    (e.g. objectName:_slotName)
//   3. internal   (e.g. objectName:__slotName)
//
// Note that slot names imply their types with prefixed 
// underscores.
//
// 1. Regular slots are programmer-defined variables and
//    are best understood in contrast with special slots.
// 
// 2. Special slots may be activated by the interpreter
//    for various reasons. _mark, for example, would be 
//    activated during the mark phase of mark & sweep
//    garbage collection. Another example is _collect,
//    a destructor of sorts. Others include _each and 
//    _onImport.
//
//    Regular slots, except for #, are never accessed by
//    the interpreter.
//
// 3. Internal slots are not mapped to imp object
//    pointers, but instead, raw pointers. They have no
//    standard structure. Avoid messing with them
//    whenever possible.
// 
//    The garbage collector, by default, will free
//    internal pointers. Keep this in mind if you are 
//    wrapping C functions or libraries. Also, always
//    use utilities from builtin/general.h and c.h; 
//    never interact with internal slots directly.
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
	unsigned short refcount;
} Object;


//// management
void Object_init(Object *self);
void Object_clean(Object *self);
void Object_free(Object *self);


//// slot methods
Object *Slot_object(Slot *self);
void *Slot_data(Slot *self);
bool Slot_isPrimitive(Slot *self);
void Slot_clean(Slot *self);
void Slot_setFunction(Slot *self, Object*(*f)(int argc, Object *argv));


//// garbage collection
void Object_mark(Object *self);
void Object_unmark(Object *self);
void Object_reference(Object *self);
void Object_unreference(Object *self);
int Object_referenceCount(Object *self);


//// slot access
void Object_putKeyShallow(Object *self, char *key);
void Object_putShallow(Object *self, char *key, Object *value);
void Object_putDeep(Object *self, char *key, Object *value);
void Object_putDataShallow(Object *self, char *key, void *value);
void Object_putDataDeep(Object *self, char *key, void *value);
void Object_remShallow(Object *self, char *key);
Slot *Object_getSlotShallow(Object *self, char *key);
Slot *Object_getSlotDeep(Object *self, char *key);
Object *Object_getShallow(Object *self, char *key);
Object *Object_getDeep(Object *self, char *key);
void *Object_getDataShallow(Object *self, char *key);
void *Object_getDataDeep(Object *self, char *key);
bool Object_hasKeyShallow(Object *self, char *key);
bool Object_hasKeyDeep(Object *self, char *key);
Object *Object_rootPrototype(Object *self);
Object *Object_prototype(Object *self);


//// miscellaneous
bool Object_isValid(Object *self);
bool Slot_isValid(Slot *self);
void Object_print(Object *self);

bool Object_canBeActivated(Object *self);
bool Object_hasMethod(Object *self, char *name);
bool Object_hasSpecialMethod(Object *self, char *name);



#endif