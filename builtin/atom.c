#include "atom.h"
#include <string.h>
#include <stdio.h>
#include "general.h"
#include <ctype.h>

char *ImpAtom_getRaw(Object *self){
	assert(Object_isValid(self));
	return (char*) Object_getDataDeep(self, "__data");
}

static bool validAtomText(char *text){
	if(!text){
		return false;
	}
	int len = strlen(text);
	for(int i = 0; i < len; i++){
		if(!isalnum(text[i]) && text[i] != '_'){
			return false;
		}
	}
	return true;
}


static bool validAtom(Object *obj){
	return Object_isValid(obj)                      &&
	       BuiltIn_protoId(obj) == BUILTIN_ATOM     &&
	       validAtomText(ImpAtom_getRaw(obj));
}  



void ImpAtom_setRaw(Object *self, char *text){
	assert(Object_isValid(self));
	assert(validAtomText(text));
	Object_putDataDeep(self, "__data", strdup(text));
}

void ImpAtom_print(Object *self){
	assert(validAtom(self));
	char *data = Object_getDataDeep(self, "__data");
	printf("%s", data);
}


static Object *ImpAtom_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(validAtom(caller));
	ImpAtom_print(caller);
	return caller;	
}


void ImpAtom_set(Object *self, Object *value){
	assert(validAtom(self));
	assert(validAtom(value));
	ImpAtom_setRaw(self, ImpAtom_getRaw(value));
}


static Object *ImpAtom_clone_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	Object_putDataShallow(r, "__data", strdup(ImpAtom_getRaw(caller)));
	return r;	
}



static Object *ImpAtom_activate_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	char *raw = ImpAtom_getRaw(caller);
	Object *obj = Object_getDeep(context, raw);

	return Runtime_activate(runtime, context, obj, argc, argv);
}

void ImpAtom_init(Object *self){
	BuiltIn_setId(self, BUILTIN_ATOM);

	Object_registerCMethod(self, "__print", ImpAtom_print_internal);
	Object_registerCMethod(self, "__clone", ImpAtom_clone_internal);
	Object_registerCMethod(self, "__activate", ImpAtom_activate_internal);
	ImpAtom_setRaw(self, "defaultAtom");
}