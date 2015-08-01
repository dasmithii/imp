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
	ParseNode *code; // closure
	Object *context;
} Internal;


static Internal *ImpClosure_getRaw(Object *self){
	assert(Object_isValid(self));
	return (Internal*) Object_getDataDeep(self, "__data");
}


bool ImpClosure_isValid(Object *obj){
	if(!Object_isValid(obj) || BuiltIn_id(obj) != BUILTIN_CLOSURE){
		return false;
	}

	if(Object_hasKeyShallow(obj, "#")){
		return Object_isValid(ImpClosure_getRaw(obj)->context);
	}
	return true;
}


void ImpClosure_print(Object *self){
	assert(ImpClosure_isValid(self));
	ParseNode_print(ImpClosure_getRaw(self)->code);
}


static Object *activate_(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	assert(runtime);
	assert(ImpClosure_isValid(caller));

	Internal *internal = Object_getDataDeep(caller, "__data");

	Object *scope = Runtime_simpleClone(runtime, internal->context);
	Object_reference(scope);

	// inject function arguments
	Object_putShallow(scope, "@", Runtime_callMethod(runtime
		                                 , context
		                                 , runtime->Array
		                                 , "withContents"
		                                 , argc - 1
		                                 , argv + 1));


	// inject self argument
	if(argc > 0){
		Object_putShallow(scope, "self", argv[0]);
	} else {
		Runtime_throwString(runtime, context, "closure not provided with self argument");
	}

	Object *r = Runtime_executeInContext(runtime
		                               , scope
		                               , *internal->code);

	Object_unreference(scope);

	return r;
}


void ImpClosure_compile(Runtime *runtime, Object *self, ParseNode *code, Object *context){
	assert(runtime);
	assert(code);
	assert(Object_isValid(self));
	assert(Object_isValid(context));

	Object_reference(self);
	Object_reference(context);

	Internal *internal = malloc(sizeof(Internal));
	if(!internal){
		abort();
	}
	internal->context = context;
	internal->code = malloc(sizeof(ParseNode));
	if(!internal->code){
		abort();
	}
	*(internal->code) = ParseNode_deepCopy(code);
	internal->code->type = BLOCK_NODE;

	Object_putDataShallow(self, "__data", internal);

	Object_unreference(self);
	Object_unreference(context);
}


static Object *clean_(Runtime *runtime
	                  , Object *context
	                  , Object *caller
	                  , int argc
	                  , Object **argv){
	assert(runtime);
	assert(ImpClosure_isValid(caller));

	Internal *raw = ImpClosure_getRaw(caller);
	ParseNode_deepClean(raw->code);
	free(raw->code);
	raw->code = NULL;
	return NULL;
}


static Object *_markInternalsRecursively(Runtime *runtime
	               , Object *context
	               , Object *self
	               , int argc
	               , Object **argv){
	assert(runtime);
	
	Internal *raw = ImpClosure_getRaw(self);
	if(raw && raw->context){
		Runtime_callMethod(runtime, context, raw->context, "_markRecursively", 0, NULL);
	}
	return NULL;
}


void ImpClosure_init(Object *self, Runtime *runtime){
	assert(Object_isValid(self));

	BuiltIn_setId(self, BUILTIN_CLOSURE);
	Object_registerCActivator(self, activate_);
	Runtime_registerCMethod(runtime, self, "_clean", clean_);
	Runtime_registerCMethod(runtime, self, "_markInternalsRecursively", _markInternalsRecursively);
}