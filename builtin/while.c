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
	Object_reference(condition);
	Object_reference(step);

	for(;;){
		// check condition
		Runtime_activate(runtime, context, condition, 0, NULL);
		if(Runtime_returnValue(runtime) == NULL                              ||
		   BuiltIn_id(Runtime_returnValue(runtime)) != BUILTIN_BOOLEAN  ||
		   ImpBoolean_getRaw(Runtime_returnValue(runtime)) == false){
			break;
		}

		// execute block
		Object *subcontext = Runtime_clone(runtime, context);
		Object_reference(subcontext);
		Runtime_activate(runtime, subcontext, step, 0, NULL);
		Object_unreference(subcontext);
	}

	Object_unreference(condition);
	Object_unreference(step);

	return NULL;
}


void ImpWhile_init(Object *self){
	BuiltIn_setId(self, BUILTIN_WHILE);
	Object_registerCMethod(self, "__activate", ImpWhile_activate_internal);
}