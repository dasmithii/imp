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
			Object *r = Runtime_activate(runtime, context, argv[i+1], 0, NULL);
			if(r){
				Runtime_setReturnValue(runtime, r);
			}
			return r;
		}
	}
	
	if(argc % 2 == 1){
		Object *r = Runtime_activate(runtime, context, argv[argc-1], 0, NULL);
		if(r){
			Runtime_setReturnValue(runtime, r);
		}
		return r;
	}

	return NULL;
}
