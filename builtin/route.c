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


bool ImpRoute_isValid(Object *obj){
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
	assert(ImpRoute_isValid(self));
	char *data = Object_getDataDeep(self, "__data");
	printf("%s", data);
}


static Object *ImpRoute_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(ImpRoute_isValid(caller));
	Runtime_print(runtime, context, ImpRoute_mapping(caller, context));
	return NULL;	
}


void ImpRoute_set(Object *self, Object *value){
	assert(ImpRoute_isValid(self));
	assert(ImpRoute_isValid(value));
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


int ImpRoute_argc(Object *self){
	assert(ImpRoute_isValid(self));
	char *raw = ImpRoute_getRaw(self);
	int r = 1;
	while(*raw){
		if(*raw == ':'){
			++r;
		}
		++raw;
	}
	return r;
}

void ImpRoute_argv(Object *self, int i, char *dest){
	assert(ImpRoute_isValid(self));
	assert(i < ImpRoute_argc(self));

	char *raw = ImpRoute_getRaw(self);
	int c = 0;
	while(c < i){
		if(*raw == ':'){
			++c;
		}
		++raw;
	}
	while(*raw && *raw != ':'){
		*dest = *raw;
		++raw;
		++dest;
	}
	*dest = 0;
}


Object *ImpRoute_mapping(Object *self, Object *context){
	assert(ImpRoute_isValid(self));
	Object *r = context;
	int argc = ImpRoute_argc(self);
	for(int i = 0; i < argc; i++){
		char cho[64];
		ImpRoute_argv(self, i, cho);
		r = Object_getDeep(r, cho);
		if(!r){
			return NULL;
		}
	}
	return r;
}