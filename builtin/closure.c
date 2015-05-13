#include "closure.h"
#include <string.h>
#include "general.h"
#include <stdlib.h>
#include <stdio.h>
#include "../toolbox/vector.h"
#include "vector.h"

typedef struct {
	ParseNode *code; // closure
	Object *context; // cached objects in closure
} ImpClosure_internal;


Object *c1 = NULL;
Object *c2 = NULL;

static ImpClosure_internal *ImpClosure_getRaw(Object *self){
	assert(Object_isValid(self));
	return (ImpClosure_internal*) Object_getDataDeep(self, "__data");
}

static bool validClosure(Object *obj){
	return Object_isValid(obj)                       &&
	       BuiltIn_protoId(obj) == BUILTIN_CLOSURE   &&
	       Object_isValid(ImpClosure_getRaw(obj)->context);
}



void ImpClosure_print(Object *self){
	assert(validClosure(self));
	ParseNode_print(ImpClosure_getRaw(self)->code);
	// TODO: print context
}


static Object *ImpClosure_activate_internal(Runtime *runtime
	                                      , Object *context
	                                      , Object *caller
	                                      , int argc
	                                      , Object **argv){
	assert(runtime);
	assert(validClosure(caller));

	ImpClosure_internal *internal = Object_getDataShallow(caller, "__data");

	Object *scope = Runtime_clone(runtime, internal->context);
	assert(Object_isValid(scope));
	Object_reference(scope);
	assert(Object_isValid(scope));

	assert(validClosure(caller));


	Object *arguments = Runtime_cloneField(runtime, "vector");
	assert(Object_isValid(arguments));
	Object_putShallow(scope, "arguments", arguments);

	assert(Object_isValid(scope));
	Vector *raw = ImpVector_getRaw(arguments);
	for(int i = 0; i < argc; i++){
		Vector_append(raw, &argv[i]);
	}
	assert(Object_isValid(arguments));

	
	assert(Object_isValid(scope));
 
	Object *r = Runtime_executeInContext(runtime
		                               , scope
		                               , *internal->code);

	assert(Object_isValid(scope));


	Object_unreference(scope);
	assert(Object_isValid(scope));


	return r;
}



static void ParseNode_cacheReferences(ParseNode *node, Object *context, Object *cache){
	assert(node);
	assert(Object_isValid(context));
	assert(Object_isValid(cache));

	if(node->type == LEAF_NODE){
		if(node->contents.token->type == TOKEN_SLOT){
			char *atom = node->contents.token->data.text;
			Object *reference = Object_getDeep(context, atom);
			if(reference){
				Object_putShallow(cache, atom, reference);
			}
		}
	} else {
		for(int i = 0; i < node->contents.non_leaf.argc; i++){
			ParseNode_cacheReferences(node->contents.non_leaf.argv + i, context, cache);
		}
	}
}



void ImpClosure_compile(Runtime *runtime, Object *self, ParseNode *code, Object *context){
	ImpClosure_internal *internal = malloc(sizeof(ImpClosure_internal));
	internal->context = Runtime_rawObject(runtime);
	internal->code = malloc(sizeof(ParseNode));

	*(internal->code) = ParseNode_deepCopy(code);


	internal->code->type = BLOCK_NODE;


	Object_putDataShallow(self, "__data", internal);
	ParseNode_cacheReferences(code, context, internal->context);
	assert(Object_isValid(internal->context));
}


// note: compile should be called after clone
static Object *ImpClosure_clone_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", Object_rootPrototype(caller));
	if(c1){
		c2 = r;
	} else {
		c1 = r;
	}
	return r;
}

static Object *ImpClosure_collect_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));

	ImpClosure_internal *raw = ImpClosure_getRaw(caller);
	assert(Object_isValid(raw->context));
	ParseNode_deepClean(raw->code);
	free(raw->code);
	raw->code = NULL;
	raw->context = NULL;
	free(raw);
	Object_remShallow(caller, "__data");

	assert(Object_isValid(caller));

	return NULL;
}


static Object *ImpClosure_mark_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(validClosure(caller));

	ImpClosure_internal *raw = ImpClosure_getRaw(caller);
	assert(Object_isValid(raw->context));

	Runtime_markRecursive(runtime, raw->context);

	assert(validClosure(caller));
	assert(Object_isValid(raw->context));

	return NULL;
}


void ImpClosure_init(Object *self){
	assert(Object_isValid(self));

	BuiltIn_setId(self, BUILTIN_CLOSURE);
	Object_registerCMethod(self, "__activate", ImpClosure_activate_internal);
	Object_registerCMethod(self, "__clone", ImpClosure_clone_internal);
	Object_registerCMethod(self, "__collect", ImpClosure_collect_internal);
	Object_registerCMethod(self, "__mark", ImpClosure_mark_internal);

	assert(Object_isValid(self));
}