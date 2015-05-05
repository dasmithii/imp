#include "boolean.h"
#include "../object.h"
#include "general.h"
#include "../c.h"
#include "number.h"
#include "../runtime.h"

static Object *ImpBoolean_clone_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	// TODO: check arguments
	Object *r = Runtime_rawObject(runtime);
	Object_putDataShallow(r, "_prototype", Object_rootPrototype(caller));
	Object_putDataShallow(r, "__data", malloc(sizeof(bool)));
	ImpBoolean_setRaw(r, ImpBoolean_getRaw(caller));
	return r;	
}


void ImpBoolean_init(Object *self){
	BuiltIn_setId(self, BUILTIN_BOOLEAN);
	Object_putDataShallow(self, "__data", malloc(sizeof(bool)));
	ImpBoolean_setRaw(self, false);
	Object_registerCMethod(self, "__clone", ImpBoolean_clone_internal);
}


bool ImpBoolean_getRaw(Object *self){
	return *((bool*) Object_getDataDeep(self, "__data"));
}


void ImpBoolean_setRaw(Object *self, bool value){
	*((bool*) Object_getDataDeep(self, "__data")) = value;
}


void ImpBoolean_setTrue(Object *self){
	ImpBoolean_setRaw(self, true);
}


void ImpBoolean_setFalse(Object *self){
	ImpBoolean_setRaw(self, false);
}


void ImpBoolean_negate(Object *self){
	ImpBoolean_setRaw(self, !ImpBoolean_getRaw(self));
}

