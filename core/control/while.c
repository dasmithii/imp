#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>
#include <stdbool.h>
#include <string.h>


static bool isZero(Runtime *runtime
	             , Object *ctx
	             , Object *obj){
	if(!obj){
		return true;
	}

	if(BuiltIn_id(obj) == BUILTIN_NUMBER){
		return ImpNumber_getRaw(obj) == 0;
	}

	if(Object_hasMethod(obj, "?")){
		Object_reference(ctx);
		Object_reference(obj);
		Object *asBoolean = Runtime_callMethod(runtime
			                                 , ctx
			                                 , obj
			                                 , "?", 0, NULL);

		Object_unreference(ctx);
		Object_unreference(obj);
		return ImpNumber_getRaw(asBoolean) == 0;
	}

	Runtime_throwString(runtime, "object not boolean");
	return false;
}



// If accepts one or more condition-executable pairs as
// arguments, plus an optional final argument, the 
// 'else' block. Conditions are scanned until one is
// met, at which point, the corresponding executable is
// executed. In the case that no conditions are met,
// the final argument is executed. 
Object *while_activate(Runtime *runtime
	                 , Object *context
	                 , Object *caller
	                 , int argc
	                 , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	Object *condition = argv[0];
	Object *step = argv[1];

	if(argc != 2){
		Runtime_throwString(runtime, "while accepts exactly 2 arguments.");
	} 
	if(!Object_canBeActivated(step)){
		Runtime_throwString(runtime, "while step must be activatable");
	}

	for(;;){
		// check condition
		Object *c = condition;
		if(Object_canBeActivated(c)){
			c = Runtime_activate(runtime, context, condition, 0, NULL);
		}
		if(isZero(runtime, context, c)){
			break;
		}

		// execute block
		Object *stepR = Runtime_activate(runtime, context, step, 0, NULL);
		if(stepR){
			Runtime_setReturnValue(runtime, stepR);
			return stepR;
		}
	}

	return NULL;
}


