#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../toolbox/vector.h"

#include "closure.h"
#include "general.h"
#include "route.h"
#include "vector.h"




typedef struct {
	ParseNode *code; // closure
	Object *context; // cached objects in closure
} ImpClosure_internal;


static ImpClosure_internal *ImpClosure_getRaw(Object *self){
	assert(Object_isValid(self));
	return (ImpClosure_internal*) Object_getDataDeep(self, "__data");
}


bool ImpClosure_isValid(Object *obj){
	if(!Object_isValid(obj) || BuiltIn_id(obj) != BUILTIN_CLOSURE){
		return false;
	}

	if(Object_hasKeyShallow(obj, "_prototype")){
		return Object_isValid(ImpClosure_getRaw(obj)->context);
	}
	return true;
}


void ImpClosure_print(Object *self){
	assert(ImpClosure_isValid(self));
	ParseNode_print(ImpClosure_getRaw(self)->code);
}


static Object *ImpClosure_activate_internal(Runtime *runtime
	                                      , Object *context
	                                      , Object *caller
	                                      , int argc
	                                      , Object **argv){
	assert(runtime);
	assert(ImpClosure_isValid(caller));
	for(int i = 0; i < argc; i++){
		assert(Object_isValid(argv[i]));
	}

	ImpClosure_internal *internal = Object_getDataShallow(caller, "__data");
	Object *scope = Runtime_clone(runtime, internal->context);
	Object_reference(scope);

	Object *arguments = Runtime_cloneField(runtime, "Vector");
	Object_putShallow(scope, "arguments", arguments);

	if(argc > 0){
		Object_putShallow(scope, "self", argv[0]);
	}

	Vector *raw = ImpVector_getRaw(arguments);
	for(int i = 1; i < argc; i++){
		Vector_append(raw, &argv[i]);
	}
 
	Object *r = Runtime_executeInContext(runtime
		                               , scope
		                               , *internal->code);

	Object_unreference(scope);

	return r;
}


static void ParseNode_cacheReferences(ParseNode *node, Object *context, Object *cache){
	assert(node);
	assert(Object_isValid(context));
	assert(Object_isValid(cache));

	if(node->type == LEAF_NODE){
		if(node->contents.token->type == TOKEN_ROUTE){
			char buf[32];
			char *route = node->contents.token->data.text;
			char *ptr = buf;
			while(*route && *route != ':'){
				*ptr = *route;
				ptr++;
				route++;
			}
			*ptr = 0;

			Object *reference = Object_getDeep(context, buf);
			if(reference){
				Object_putShallow(cache, buf, reference);
			}
		}
	} else {
		for(int i = 0; i < node->contents.non_leaf.argc; i++){
			ParseNode_cacheReferences(node->contents.non_leaf.argv + i, context, cache);
		}
	}
}


void ImpClosure_compile(Runtime *runtime, Object *self, ParseNode *code, Object *context){
	assert(runtime);
	assert(code);
	assert(Object_isValid(self));
	assert(Object_isValid(context));

	Object_reference(self);
	Object_reference(context);

	ImpClosure_internal *internal = malloc(sizeof(ImpClosure_internal));
	if(!internal){
		abort();
	}
	internal->context = Runtime_rawObject(runtime);
	Object_reference(internal->context);
	internal->code = malloc(sizeof(ParseNode));
	if(!internal->code){
		abort();
	}

	*(internal->code) = ParseNode_deepCopy(code);

	internal->code->type = BLOCK_NODE;

	Object_putDataShallow(self, "__data", internal);
	ParseNode_cacheReferences(code, context, internal->context);
	assert(Object_isValid(internal->context));

	Object_unreference(internal->context);
	Object_unreference(self);
	Object_unreference(context);
}


static Object *ImpClosure_clone_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));
	assert(argc == 0);

	Object_reference(caller);
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", Object_rootPrototype(caller));
	Object_unreference(caller);
	return r;
}


static Object *ImpClosure_collect_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(ImpClosure_isValid(caller));

	ImpClosure_internal *raw = ImpClosure_getRaw(caller);
	assert(Object_isValid(raw->context));
	ParseNode_deepClean(raw->code);
	free(raw->code);
	raw->code = NULL;
	raw->context = NULL;
	Object_remShallow(caller, "__data");

	return NULL;
}


static Object *ImpClosure_mark_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(ImpClosure_isValid(caller));

	ImpClosure_internal *raw = ImpClosure_getRaw(caller);
	if(Object_hasKeyShallow(caller, "_prototype")){
		Runtime_markRecursive(runtime, raw->context);
	}
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