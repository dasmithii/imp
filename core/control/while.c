#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/number.h>




// note: this function, isZero, is duplicated for all
// control structures (in each file) because I'm inept.
// Make sure to replicate any changes to this function
// in the other files as well.
static bool isZero(iRuntime *runtime
	             , iObject *ctx
	             , iObject *obj){
	if(!obj){
		return true;
	}

	if(iBuiltin_id(obj) == iBUILTIN_NUMBER){
		return iNumber_getRaw(obj) == 0;
	}

	if(iObject_hasMethod(obj, "?")){
		iObject_reference(ctx);
		iObject_reference(obj);
		iObject *asBoolean = iRuntime_callMethod(runtime
			                                  , ctx
			                                  , obj
			                                  , "?", 0, NULL);

		iObject_unreference(ctx);
		iObject_unreference(obj);
		return iNumber_getRaw(asBoolean) == 0;
	}

	iRuntime_throwString(runtime, ctx, "object not boolean");
	return false;
}





// If accepts one or more condition-executable pairs as
// arguments, plus an optional final argument, the 
// 'else' block. Conditions are scanned until one is
// met, at which point, the corresponding executable is
// executed. In the case that no conditions are met,
// the final argument is executed. 
iObject *while_activate(iRuntime *runtime
	                  , iObject *context
	                  , iObject *caller
	                  , int argc
	                  , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));

	iObject *condition = argv[0];
	iObject *step = argv[1];

	if(argc != 2){
		iRuntime_throwString(runtime, context, "while requires exactly 2 arguments");
	} 

	if(iBuiltin_id(step) != iBUILTIN_CLOSURE){
		iRuntime_throwString(runtime, context, "while step must be a closure");
	}


	for(;;){
		// check condition
		iObject *c = condition;
		if(iObject_canBeActivated(c)){
			c = iRuntime_activateOn(runtime, context, condition, 0, NULL, iObject_getDeep(context, "self"));
		}
		if(isZero(runtime, context, c)){
			break;
		}

		// execute block
		iObject *stepR = iRuntime_activateOn(runtime, context, step, 0, NULL, iObject_getDeep(context, "self"));
		if(stepR){
			if(iBuiltin_id(stepR) == iBUILTIN_BREAK){
				break;
			} else if(iBuiltin_id(stepR) == iBUILTIN_CONTINUE) {
				continue;
			} else {
				iRuntime_setReturnValue(runtime, stepR);
			}
			return stepR;
		}
	}

	return NULL;
}


