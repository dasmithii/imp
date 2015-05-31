#include <imp/c.h>
#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>



static bool isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_BOOLEAN;
}


static bool getRaw(Object *self){
	assert(ImpBoolean_isValid(self));
	return *((bool*) Object_getDataDeep(self, "__data"));
}


static void setRaw(Object *self, bool value){
	assert(ImpBoolean_isValid(self));
	*((bool*) Object_getDataDeep(self, "__data")) = value;
}



Object *Boolean_clone(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));

	if(argc > 0){
		Runtime_throwString(runtime, "Boolean:clone accepts no arguments.");
		return NULL;
	}

	Object *r = Runtime_rawObject(runtime);
	Object_putDataShallow(r, "_prototype", Object_rootPrototype(caller));
	void *data = malloc(sizeof(bool));
	if(!data){
		abort();
	}
	Object_putDataShallow(r, "__data", data);
	ImpBoolean_setRaw(r, ImpBoolean_getRaw(caller));
	
	return r;	
}


void ImpBoolean_init(Object *self){
	assert(self);

	BuiltIn_setId(self, BUILTIN_BOOLEAN);
	void *data = malloc(sizeof(bool));
	if(!data){
		abort();
	}
	Object_putDataShallow(self, "__data", data);
	ImpBoolean_setRaw(self, false);
	Object_registerCMethod(self, "__clone", ImpBoolean_clone_internal);
}



void ImpBoolean_setTrue(Object *self){
	assert(ImpBoolean_isValid(self));
	ImpBoolean_setRaw(self, true);
}


void ImpBoolean_setFalse(Object *self){
	assert(ImpBoolean_isValid(self));
	ImpBoolean_setRaw(self, false);
}


void ImpBoolean_negate(Object *self){
	assert(ImpBoolean_isValid(self));
	ImpBoolean_setRaw(self, !ImpBoolean_getRaw(self));
}

