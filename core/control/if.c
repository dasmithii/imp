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
iObject *if_activate(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));

	if(argc < 2){
		iRuntime_throwString(runtime, context, "if given insufficient arguments");
	}

	// check that all provided blocks are valid
	for(int i = 1; i < argc; i += 2){
		if(iBuiltin_id(argv[i]) != iBUILTIN_CLOSURE){
			iRuntime_throwString(runtime, context, "if requires closure arguments");
			return NULL;
		}
	}

	for(int i = 0; i < argc - 1; i += 2){
		iObject *cond = argv[i];
		if(iObject_canBeActivated(cond)){
			cond = iRuntime_activateOn(runtime, context, cond, 0, NULL, iObject_getDeep(context, "self"));
		}
		if(!isZero(runtime, context, cond)){
			iObject *r = iRuntime_activateOn(runtime, context, argv[i+1], 0, NULL, iObject_getDeep(context, "self"));
			if(r){
				iRuntime_setReturnValue(runtime, r);
			}
			return r;
		}
	}
	
	if(argc % 2 == 1){
		iObject *r = iRuntime_activateOn(runtime, context, argv[argc-1], 0, NULL, iObject_getDeep(context, "self"));
		if(r){
			iRuntime_setReturnValue(runtime, r);
		}
		return r;
	}

	return NULL;
}
