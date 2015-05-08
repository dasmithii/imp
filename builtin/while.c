#include "while.h"
#include "../object.h"
#include "general.h"
#include "../c.h"
#include "../runtime.h"
#include <stdbool.h>
#include <stdio.h>
#include "boolean.h"


static Object *ImpWhile_activate_internal(Runtime *runtime
	                                    , Object *context
	                                    , Object *caller
	                                    , int argc
	                                    , Object **argv){


	// TODO: check arguments
	Object *condition = argv[0];
	Object *step = argv[1];
	Object_putKeyShallow(condition, "__volatile");
	Object_putKeyShallow(step, "__volatile");

	while(ImpBoolean_getRaw(Runtime_activate(runtime, context, condition, 0, NULL))){
		Object *subcontext = Runtime_clone(runtime, context);
		Object_putKeyShallow(subcontext, "__volatile");
		Runtime_activate(runtime, subcontext, step, 0, NULL);
		Object_remShallow(subcontext, "__volatile");
	}

	Object_remShallow(condition, "__volatile");
	Object_remShallow(step, "__volatile");

	return NULL;
}


void ImpWhile_init(Object *self){
	BuiltIn_setId(self, BUILTIN_WHILE);
	Object_registerCMethod(self, "__activate", ImpWhile_activate_internal);
}