#include <assert.h>
#include <string.h>

#include "miscellaneous.h"
#include "../c.h"
#include "general.h"
#include "string.h"



   
static iObject *break_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "break does not accept arguments");
	}
	iObject *sig = iRuntime_rawObject(runtime);
	iBuiltin_setId(sig, iBUILTIN_BREAK);
	iRuntime_setReturnValue(runtime, sig);
	return NULL;
}


static iObject *continue_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *self
	                    , int argc
	                    , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "break does not accept arguments");
	}
	iObject *sig = iRuntime_rawObject(runtime);
	iBuiltin_setId(sig, iBUILTIN_CONTINUE);
	iRuntime_setReturnValue(runtime, sig);
	return NULL;
}


static iObject *return_(iRuntime *runtime
	                  , iObject *context
	                  , iObject *caller
	                  , int argc
	                  , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));

	if(argc > 1){
		iRuntime_throwString(runtime, context, "return accepts only one parameter.");
	} else if(argc == 1){
		iRuntime_setReturnValue(runtime, argv[0]);
		return argv[0];
	} else {
		iRuntime_setReturnValue(runtime, NULL);
	}
	return NULL;
}

static iObject *throw_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));

	if(argc != 1){
		iRuntime_throwString(runtime, context, "throw accepts exactly one parameter.");
	}
	iRuntime_throw(runtime, context, argv[0]);
	return NULL;
}


void iMisc_init(iObject *self, iRuntime *runtime){
	assert(self);
	iRuntime_registerCMethod(runtime, self, "break", break_);
	iRuntime_registerCMethod(runtime, self, "continue", continue_);
	iRuntime_registerCMethod(runtime, self, "return", return_);
	iRuntime_registerCMethod(runtime, self, "throw", throw_);

	iRuntime_executeSourceInContext(runtime
		                         , "(def nil:asString 'nil')"
		                         "\n(def nil:? {return 0})"
		                         , runtime->root_scope);
	runtime->nil = iObject_getShallow(runtime->root_scope, "nil");
	iObject_reference(runtime->nil); // nil is eternal
}