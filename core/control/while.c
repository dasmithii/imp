#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>
#include <stdbool.h>
#include <string.h>

// references to singleton values in boolean module
static Object *T = NULL;
static Object *F = NULL;

static bool isZero(Runtime *runtime
	             , Object *ctx
	             , Object *obj){
	if(!obj || obj == F){
		return true;
	}

	if(Object_hasMethod(obj, "asBoolean")){
		Object *asBoolean = Runtime_callMethod(runtime
			                                 , ctx
			                                 , obj
			                                 , "asBoolean", 0, NULL);
		if(asBoolean == F){
			return true;
		} else {
			return false;
		}
	}

	if(BuiltIn_id(obj) == BUILTIN_NUMBER &&
	   ImpNumber_getRaw(obj) == 0){
		return true;
	}

	if(BuiltIn_id(obj) == BUILTIN_STRING &&
	   strcmp(ImpString_getRaw(obj), "") == 0){
		return true;
	}

	if(Object_hasMethod(obj, "asNumber")){
		Object *asNumber = Runtime_callMethod(runtime
			                                , ctx
			                                , obj
			                                , "asNumber", 0, NULL);
		if(ImpNumber_getRaw(asNumber) == 0){
			return true;
		} else {
			return false;
		}
	}

	// TODO:? check if a boolean was returned at all
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
	} else if(!Object_canBeActivated(condition)){
		Runtime_throwString(runtime, "while condition must be activatable");
	} else if(!Object_canBeActivated(step)){
		Runtime_throwString(runtime, "while step must be activatable");
	}

	for(;;){
		// check condition
		Runtime_activate(runtime, context, condition, 0, NULL);
		if(isZero(runtime, context, Runtime_returnValue(runtime))){
			break;
		}

		// execute block
		Object *subcontext = Runtime_clone(runtime, context);
		Runtime_activate(runtime, subcontext, step, 0, NULL);
	}

	return NULL;
}


Object *while_onImport(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	Object *boolean_module = Imp_import(runtime, "core/Boolean");
	T = Object_getShallow(boolean_module, "true");
	F = Object_getShallow(boolean_module, "false");
	return NULL;
}

