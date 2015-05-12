#include "return.h"
#include "general.h"
#include "atom.h"
#include <stdio.h>
#include "boolean.h"


static Object *ImpReturn_activate_internal(Runtime *runtime
	                             , Object *context
	                             , Object *caller
	                             , int argc
	                             , Object **argv){
	assert(argc == 1);
	Object *value = argv[0];
	if(BuiltIn_id(value) == BUILTIN_ATOM){
		value = ImpAtom_mapping(value, context);
	}
	Runtime_setReturnValue(runtime, value);
	return NULL;
}

void ImpReturn_init(Object *self){
	BuiltIn_setId(self, BUILTIN_RETURN);
	Object_registerCMethod(self, "__activate", ImpReturn_activate_internal);
}
