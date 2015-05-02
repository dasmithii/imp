#include "string.h"
#include <string.h>
#include <stdio.h>
#include "general.h"


char *ImpString_getRaw(Object *self){
	return (char*) Object_getDataDeep(self, "__data");
}

void ImpString_setRaw(Object *self, char *text){
	Object_putDataDeep(self, "__data", strdup(text));
}

void ImpString_print(Object *self){
	printf("%s", ImpString_getRaw(self));
}


static Object *ImpString_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	ImpString_print(caller);
	return caller;	
}


void ImpString_set(Object *self, Object *value){
	ImpString_setRaw(self, ImpString_getRaw(value));
}


static Object *ImpString_clone_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putDataShallow(r, "_prototype", caller);
	Object_putDataShallow(r, "__data", strdup(ImpString_getRaw(caller)));
	return r;	
}


void ImpString_init(Object *self){
	BuiltIn_setId(self, BUILTIN_STRING);

	Object_registerCMethod(self, "__print", ImpString_print_internal);
	Object_registerCMethod(self, "__clone", ImpString_clone_internal);

	ImpString_setRaw(self, "");
}