#include <assert.h>
#include <string.h>

#include "miscellaneous.h"
#include "../c.h"
#include "general.h"
#include "string.h"


   
static Object *break_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "break does not accept arguments");
	}
	Object *sig = Runtime_rawObject(runtime);
	BuiltIn_setId(sig, BUILTIN_BREAK);
	Runtime_setReturnValue(runtime, sig);
	return NULL;
}


static Object *continue_(Runtime *runtime
	                   , Object *context
	                   , Object *self
	                   , int argc
	                   , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "break does not accept arguments");
	}
	Object *sig = Runtime_rawObject(runtime);
	BuiltIn_setId(sig, BUILTIN_CONTINUE);
	Runtime_setReturnValue(runtime, sig);
	return NULL;
}


static Object *return_(Runtime *runtime
	                 , Object *context
	                 , Object *caller
	                 , int argc
	                 , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc > 1){
		Runtime_throwString(runtime, "return accepts only one parameter.");
	} else if(argc == 1){
		Runtime_setReturnValue(runtime, argv[0]);
		return argv[0];
	} else {
		Runtime_setReturnValue(runtime, NULL);
	}
	return NULL;
}

static Object *throw_(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc != 1){
		Runtime_throwString(runtime, "throw accepts exactly one parameter.");
	}
	Runtime_throw(runtime, argv[0]);
	return NULL;
}


void ImpMisc_init(Object *self, Runtime *runtime){
	assert(self);
	Runtime_registerCMethod(runtime, self, "break", break_);
	Runtime_registerCMethod(runtime, self, "continue", continue_);
	Runtime_registerCMethod(runtime, self, "return", return_);
	Runtime_registerCMethod(runtime, self, "throw", throw_);

	Runtime_executeSourceInContext(runtime
		                         , "(def nil:asString 'nil')"
		                         "\n(def nil:? {return 0})"
		                         , runtime->root_scope);
	runtime->nil = Object_getShallow(runtime->root_scope, "nil");
	Object_reference(runtime->nil); // nil is eternal
}