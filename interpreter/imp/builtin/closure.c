#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../toolbox/vector.h"

#include "closure.h"
#include "general.h"
#include "route.h"
#include "vector.h"


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
	for(int i = 0; i < argc; i++){
		assert(Object_isValid(argv[i]));
	}

	Internal *internal = Object_getDataDeep(caller, "__data");


	Object *scope = Runtime_simpleClone(runtime, internal->context);
	Object_reference(scope);

	// inject function arguments
	Object *arguments = Runtime_cloneField(runtime, "Vector");
	Object_putShallow(scope, "@", arguments);
	Vector *raw = ImpVector_getRaw(arguments);
	for(int i = 1; i < argc; i++){
		Vector_append(raw, &argv[i]);
	}

	// inject self argument
	if(argc > 0){
		Object_putShallow(scope, "self", argv[0]);
	} else {
		Runtime_throwString(runtime, "closure not provided with self argument");
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


static Object *collect_(Runtime *runtime
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


static Object *mark_(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);

	Internal *raw = ImpClosure_getRaw(caller);
	if(raw && raw->context){
		Runtime_markRecursive(runtime, raw->context);
	}
	return NULL;
}


void ImpClosure_init(Object *self, Runtime *runtime){
	assert(Object_isValid(self));

	BuiltIn_setId(self, BUILTIN_CLOSURE);
	Object_registerCActivator(self, activate_);
	Runtime_registerCMethod(runtime, self, "collect", collect_);
	Runtime_registerCMethod(runtime, self, "mark", mark_);
}