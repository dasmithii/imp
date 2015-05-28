#include <stdio.h>
#include <string.h>

#include "../c.h"
#include "../object.h"

#include "set.h"
#include "general.h"
#include "route.h"




bool ImpSet_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_SETTER;
}


static Object *ImpSet_activate_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpSet_isValid(caller));

	if(argc != 2){
		Runtime_throwString(runtime, "set requires exactly 2 arguments.");
		return NULL;
	} else if(!ImpRoute_isValid(argv[0])){
		Runtime_throwString(runtime, "set requires a route for argument 1.");
		return NULL;
	}

	Object *route = argv[0];
	Object *value = unrouteInContext(argv[1], context);

	Object *parent = ImpRoute_submapping(route, context);
	if(!parent){
		Runtime_throwString(runtime, "set failed; try def.");
	}

	char field[32];
	ImpRoute_argv(route, ImpRoute_argc(route) - 1, field);

	Object_putShallow(parent, field, value);

	return NULL;
}


void ImpSet_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_SETTER);
	Object_registerCMethod(self, "__activate", ImpSet_activate_internal);
}