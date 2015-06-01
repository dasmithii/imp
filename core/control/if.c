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

	if(Object_hasMethod(obj, "?")){
		Object_reference(ctx);
		Object_reference(obj);
		Object *asBoolean = Runtime_callMethod(runtime
			                                 , ctx
			                                 , obj
			                                 , "?", 0, NULL);

		Object_unreference(ctx);
		Object_unreference(obj);
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
		Object_reference(ctx);
		Object_reference(obj);
		Object *asNumber = Runtime_callMethod(runtime
			                                , ctx
			                                , obj
			                                , "asNumber", 0, NULL);
		Object_unreference(ctx);
		Object_unreference(obj);
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
Object *if_activate(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc < 2){
		Runtime_throwString(runtime, "if given insufficient arguments");
	}

	for(int i = 0; i < argc - 1; i += 2){
		Object *cond = argv[i];
		if(Object_canBeActivated(cond)){
			cond = Runtime_activate(runtime, context, cond, 0, NULL);
		}
		if(!isZero(runtime, context, cond)){
			Runtime_activate(runtime, context, argv[i+1], 0, NULL);
			return NULL;
		}
	}
	
	if(argc % 2 == 1){
		Runtime_activate(runtime, context, argv[argc-1], 0, NULL);
	}

	return NULL;
}


Object *if_onImport(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	Object *boolean_module = Imp_import(runtime, "core/Boolean");
	T = Object_getShallow(boolean_module, "true");
	F = Object_getShallow(boolean_module, "false");
	return NULL;
}

