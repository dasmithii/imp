#include <stdio.h>

#include "boolean.h"
#include "general.h"
#include "return.h"
#include "route.h"




bool ImpReturn_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_RETURN;
}


static Object *ImpReturn_activate_internal(Runtime *runtime
	                             , Object *context
	                             , Object *caller
	                             , int argc
	                             , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpReturn_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "return accepts only one parameter.");
	} else {
		Object *value = argv[0];
		if(BuiltIn_id(value) == BUILTIN_ROUTE){
			value = ImpRoute_mapping(value, context);
		}
		Runtime_setReturnValue(runtime, value);
	}
	return NULL;
}


void ImpReturn_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_RETURN);
	Object_registerCMethod(self, "__activate", ImpReturn_activate_internal);
}
