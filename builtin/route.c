#include "route.h"
#include <string.h>
#include <stdio.h>
#include "general.h"
#include <ctype.h>




char *ImpRoute_getRaw(Object *self){
	assert(Object_isValid(self));
	return (char*) Object_getDataDeep(self, "__data");
}

static bool validRouteText(char *text){
	if(!text){
		return false;
	}
	int len = strlen(text);
	for(int i = 0; i < len; i++){
		if(!isalnum(text[i]) && text[i] != '_' && text[i] != ':'){
			return false;
		}
	}
	return true;
}


static bool validRoute(Object *obj){
	return Object_isValid(obj)                      &&
	       BuiltIn_protoId(obj) == BUILTIN_ROUTE    &&
	       validRouteText(ImpRoute_getRaw(obj));
}  



void ImpRoute_setRaw(Object *self, char *text){
	assert(Object_isValid(self));
	assert(validRouteText(text));
	Object_putDataDeep(self, "__data", strdup(text));
}

void ImpRoute_print(Object *self){
	assert(validRoute(self));
	char *data = Object_getDataDeep(self, "__data");
	printf("%s", data);
}


static Object *ImpRoute_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(validRoute(caller));
	ImpRoute_print(caller);
	return caller;	
}


void ImpRoute_set(Object *self, Object *value){
	assert(validRoute(self));
	assert(validRoute(value));
	ImpRoute_setRaw(self, ImpRoute_getRaw(value));
}


static Object *ImpRoute_clone_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	Object_putDataShallow(r, "__data", strdup(ImpRoute_getRaw(caller)));
	return r;	
}



static Object *ImpRoute_activate_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	char *raw = ImpRoute_getRaw(caller);
	Object *obj = Object_getDeep(context, raw);

	return Runtime_activate(runtime, context, obj, argc, argv);
}

void ImpRoute_init(Object *self){
	BuiltIn_setId(self, BUILTIN_ROUTE);
	Object_registerCMethod(self, "__print", ImpRoute_print_internal);
	Object_registerCMethod(self, "__clone", ImpRoute_clone_internal);
	Object_registerCMethod(self, "__activate", ImpRoute_activate_internal);
	ImpRoute_setRaw(self, "defaultRoute");
}


Object *ImpRoute_mapping(Object *self, Object *context){
	assert(Object_isValid(self));
	return Object_getDeep(context, ImpRoute_getRaw(self));
}