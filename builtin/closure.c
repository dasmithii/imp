#include "closure.h"
#include <string.h>
#include "general.h"
#include <stdlib.h>
#include <stdio.h>
#include "../toolbox/vector.h"

typedef struct {
	ParseNode *code; // closure
	Object *context; // cached objects in closure
} ImpClosure_internal;


static ImpClosure_internal *ImpClosure_getRaw(Object *self){
	return (ImpClosure_internal*) Object_getDataDeep(self, "__data");
}


void ImpClosure_print(Object *self){
	ParseNode_print(ImpClosure_getRaw(self)->code);
	// TODO: print context
}


static Object *ImpClosure_activate_internal(Runtime *runtime
	                                      , Object *context
	                                      , Object *caller
	                                      , int argc
	                                      , Object **argv){
	ImpClosure_internal *internal = Object_getDataShallow(caller, "__data");

	Object *scope = Runtime_shallowCopy(runtime, internal->context);
	Object_putKeyShallow(scope, "__volatile");


	Object *arguments = Runtime_cloneField(runtime, "vector");
	Object_putShallow(scope, "arguments", arguments);
	Vector *raw = ImpVector_getRaw(arguments);
	for(int i = 0; i < argc; i++){
		Vector_append(raw, &argv[i]);
	}
 
	Object *r = Runtime_executeInContext(runtime
		                               , scope
		                               , *internal->code);

	Object_remShallow(scope, "__volatile");
	return r;
}



static void ParseNode_cacheReferences(ParseNode *node, Object *context, Object *cache){
	assert(node);
	assert(context);
	assert(cache);

	if(node->type == LEAF_NODE){
		if(node->contents.token->type == TOKEN_ATOM){
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
	internal->code->type = CALL_NODE;
	Object_putDataShallow(self, "__data", internal);
	ParseNode_cacheReferences(code, context, internal->context);
}


// note: compile should be called after clone
static Object *ImpClosure_clone_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	return r;
}

static Object *ImpClosure_collect_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	ImpClosure_internal *raw = ImpClosure_getRaw(caller);
	if(raw){
		ParseNode_deepClean(raw->code);
		free(raw->code);
		raw->code = NULL;
		free(raw);
		Object_remShallow(caller, "__data");
	}
	return NULL;
}


static Object *ImpClosure_mark_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	ImpClosure_internal *raw = ImpClosure_getRaw(caller);
	if(raw){
		printf("ImpClosure, mark\n");
		Runtime_markRecursive(runtime, raw->context);
	}
}


void ImpClosure_init(Object *self){
	BuiltIn_setId(self, BUILTIN_CLOSURE);
	Object_registerCMethod(self, "__activate", ImpClosure_activate_internal);
	Object_registerCMethod(self, "__clone", ImpClosure_clone_internal);
	Object_registerCMethod(self, "__collect", ImpClosure_collect_internal);
	Object_registerCMethod(self, "__mark", ImpClosure_mark_internal);
}