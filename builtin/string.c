#include <string.h>
#include <stdio.h>

#include "string.h"
#include "general.h"




bool ImpString_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_STRING;
}


char *ImpString_getRaw(Object *self){
	assert(ImpString_isValid(self));
	return (char*) Object_getDataDeep(self, "__data");
}


void ImpString_setRaw(Object *self, char *text){
	assert(ImpString_isValid(self));
	assert(text);
	Object_putDataDeep(self, "__data", strdup(text));
}


void ImpString_print(Object *self){
	assert(ImpString_isValid(self));
	printf("%s", ImpString_getRaw(self));
}


static Object *ImpString_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(ImpString_isValid(caller));

	if(argc > 0){
		Runtime_throwString(runtime, "string:print does not accept arguments.");
	} else {
		ImpString_print(caller);
	}

	return NULL;	
}


void ImpString_set(Object *self, Object *value){
	assert(ImpString_isValid(self));
	assert(ImpString_isValid(value));

	ImpString_setRaw(self, ImpString_getRaw(value));
}


static Object *ImpString_clone_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(ImpString_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "string:clone does not accept arguments.");
		return NULL;
	}

	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	Object_putDataShallow(r, "__data", strdup(ImpString_getRaw(caller))); // TODO: check pointer from strdup

	return r;	
}


void ImpString_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_STRING);

	Object_registerCMethod(self, "__print", ImpString_print_internal);
	Object_registerCMethod(self, "__clone", ImpString_clone_internal);

	ImpString_setRaw(self, "");
}