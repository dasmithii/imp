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
#ifndef IMP_OBJECT
#define IMP_OBJECT

#include <stdbool.h>
#include <stdlib.h>




typedef struct {
	char *key;
	void *data; 
} iSlot;


typedef struct iObject {
	iSlot *slots;
	int slotCount;
	bool gc_mark;
	unsigned short refcount;
} iObject;


//// management
void iObject_init(iObject *self);
void iObject_clean(iObject *self);
void iObject_free(iObject *self);


//// slot methods
iObject *iSlot_object(iSlot *self);
void *iSlot_data(iSlot *self);
bool iSlot_isPrimitive(iSlot *self);
void iSlot_clean(iSlot *self);
void iSlot_setFunction(iSlot *self, iObject*(*f)(int argc, iObject *argv));


//// garbage collection
void iObject_mark(iObject *self);
void iObject_unmark(iObject *self);
void iObject_reference(iObject *self);
void iObject_unreference(iObject *self);
int iObject_referenceCount(iObject *self);


//// slot access
void iObject_putKeyShallow(iObject *self, char *key);
void iObject_putShallow(iObject *self, char *key, iObject *value);
void iObject_putDeep(iObject *self, char *key, iObject *value);
void iObject_putDataShallow(iObject *self, char *key, void *value);
void iObject_putDataDeep(iObject *self, char *key, void *value);
void iObject_remShallow(iObject *self, char *key);
iSlot *iObject_getSlotShallow(iObject *self, char *key);
iSlot *iObject_getSlotDeep(iObject *self, char *key);
iObject *iObject_getShallow(iObject *self, char *key);
iObject *iObject_getDeep(iObject *self, char *key);
void *iObject_getDataShallow(iObject *self, char *key);
void *iObject_getDataDeep(iObject *self, char *key);
bool iObject_hasKeyShallow(iObject *self, char *key);
bool iObject_hasKeyDeep(iObject *self, char *key);
iObject *iObject_rootPrototype(iObject *self);
iObject *iObject_prototype(iObject *self);


//// miscellaneous
bool iObject_isValid(iObject *self);
bool iSlot_isValid(iSlot *self);
void iObject_print(iObject *self);

bool iObject_canBeActivated(iObject *self);
bool iObject_hasMethod(iObject *self, char *name);
bool iObject_hasSpecialMethod(iObject *self, char *name);



#endif