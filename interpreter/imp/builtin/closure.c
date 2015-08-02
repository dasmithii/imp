#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "closure.h"
#include "general.h"
#include "route.h"




// Internally, closures store an AST, a pointer to the
// context in which they were compiled, and a cache of
// objects which they referenced during compile time.
//
// Closure object contexts
typedef struct {
	iParseNode *code; // closure
	iObject *context;
} Internal;


static Internal *iClosure_getRaw(iObject *self){
	assert(iObject_isValid(self));
	return (Internal*) iObject_getDataDeep(self, "__data");
}


bool iClosure_isValid(iObject *obj){
	if(!iObject_isValid(obj) || iBuiltin_id(obj) != iBUILTIN_CLOSURE){
		return false;
	}

	if(iObject_hasKeyShallow(obj, "#")){
		return iObject_isValid(iClosure_getRaw(obj)->context);
	}
	return true;
}


void iClosure_print(iObject *self){
	assert(iClosure_isValid(self));
	iParseNode_print(iClosure_getRaw(self)->code);
}


static iObject *activate_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *caller
	                    , int argc
	                    , iObject **argv){
	assert(runtime);
	assert(iClosure_isValid(caller));

	Internal *internal = iObject_getDataDeep(caller, "__data");

	iObject *scope = iRuntime_simpleClone(runtime, internal->context);
	iObject_reference(scope);

	// inject function arguments
	iObject_putShallow(scope, "@", iRuntime_callMethod(runtime
		                                 , context
		                                 , runtime->Array
		                                 , "withContents"
		                                 , argc - 1
		                                 , argv + 1));


	// inject self argument
	if(argc > 0){
		iObject_putShallow(scope, "self", argv[0]);
	} else {
		iRuntime_throwString(runtime, context, "closure not provided with self argument");
	}

	iObject *r = iRuntime_executeInContext(runtime
		                               , scope
		                               , *internal->code);

	iObject_unreference(scope);

	return r;
}


void iClosure_compile(iRuntime *runtime, iObject *self, iParseNode *code, iObject *context){
	assert(runtime);
	assert(code);
	assert(iObject_isValid(self));
	assert(iObject_isValid(context));

	iObject_reference(self);
	iObject_reference(context);

	Internal *internal = malloc(sizeof(Internal));
	if(!internal){
		abort();
	}
	internal->context = context;
	internal->code = malloc(sizeof(iParseNode));
	if(!internal->code){
		abort();
	}
	*(internal->code) = iParseNode_deepCopy(code);
	internal->code->type = iNODE_BLOCK;

	iObject_putDataShallow(self, "__data", internal);

	iObject_unreference(self);
	iObject_unreference(context);
}


static iObject *clean_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iClosure_isValid(caller));

	Internal *raw = iClosure_getRaw(caller);
	iParseNode_deepClean(raw->code);
	free(raw->code);
	raw->code = NULL;
	return NULL;
}


static iObject *_markInternalsRecursively(iRuntime *runtime
	                                    , iObject *context
	                                    , iObject *self
	                                    , int argc
	                                    , iObject **argv){
	assert(runtime);
	
	Internal *raw = iClosure_getRaw(self);
	if(raw && raw->context){
		iRuntime_callMethod(runtime, context, raw->context, "_markRecursively", 0, NULL);
	}
	return NULL;
}


void iClosure_init(iObject *self, iRuntime *runtime){
	assert(iObject_isValid(self));

	iBuiltin_setId(self, iBUILTIN_CLOSURE);
	iObject_registerCActivator(self, activate_);
	iRuntime_registerCMethod(runtime, self, "_clean", clean_);
	iRuntime_registerCMethod(runtime, self, "_markInternalsRecursively", _markInternalsRecursively);
}