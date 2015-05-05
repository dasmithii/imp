#include "closure.h"
#include <string.h>
#include "general.h"
#include <stdlib.h>
#include <stdio.h>

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
	ImpClosure_internal *internal = Object_getDataDeep(caller, "__data");

	// TODO: form new scope with compile-context variables, function 
	// arguments, and room for local variables.
 
	return Runtime_executeInContext(runtime
		                          , internal->context
		                          , *internal->code);
}



static void ParseNode_cacheReferences(ParseNode *node, Object *context, Object *cache){
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


// Compiles closure by, firstly, making a deep copy of its
// parse tree, and secondly, caching all contextual objects
// that may be referenced to in said parse tree.
void ImpClosure_compile(Object *self, ParseNode *code, Object *context){
	ImpClosure_internal *internal = malloc(sizeof(ImpClosure_internal));
	internal->code = malloc(sizeof(ParseNode));
	internal->context = malloc(sizeof(Object));
	*(internal->code) = ParseNode_deepCopy(code);
	internal->code->type = CALL_NODE;
	Object_putDataDeep(self, "__data", internal);

	ParseNode_cacheReferences(code, context, internal->context);

}


static Object *ImpClosure_clone_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	Object_putDataShallow(r, "__data", malloc(sizeof(ImpClosure_internal)));
	return r;
}

static Object *ImpClosure_clean_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	ImpClosure_internal *raw = ImpClosure_getRaw(caller);
	ParseNode_deepClean(raw->code);
	free(raw->code);
	raw->code = NULL;

	Object_clean(raw->context);
	free(raw->context);
	raw->context = 0;
	return NULL;
}

// TODO: add __unmark and __markRecursive methods, which account for 
// internal context


void ImpClosure_init(Object *self){
	BuiltIn_setId(self, BUILTIN_CLOSURE);
	Object_registerCMethod(self, "__activate", ImpClosure_activate_internal);
	Object_registerCMethod(self, "__clone", ImpClosure_clone_internal);
	Object_registerCMethod(self, "__clean", ImpClosure_clean_internal);
}