#include "atom.h"
#include <string.h>
#include <stdio.h>
#include "general.h"

char *ImpAtom_getRaw(Object *self){
	return (char*) Object_getDataDeep(self, "__data");
}

void ImpAtom_setRaw(Object *self, char *text){
	Object_putDataDeep(self, "__data", strdup(text));
}

void ImpAtom_print(Object *self){
	char *data = Object_getDataDeep(self, "__self");
	printf("%s", data);
}


static Object *ImpAtom_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	ImpAtom_print(caller);
	return caller;	
}


void ImpAtom_set(Object *self, Object *value){
	ImpAtom_setRaw(self, ImpAtom_getRaw(value));
}


static Object *ImpAtom_clone_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putDataShallow(r, "_prototype", caller);
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

	ImpAtom_setRaw(self, "");
}