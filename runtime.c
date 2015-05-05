#include "runtime.h"
#include "parser.h"
#include <string.h>
#include <stdio.h>

#include "builtin/general.h"
#include "builtin/atom.h"
#include "builtin/number.h"
#include "builtin/string.h"
#include "builtin/print.h"
#include "builtin/def.h"
#include "builtin/set.h"
#include "builtin/closure.h"
#include "builtin/boolean.h"
#include "builtin/while.h"
#include "c.h"



static void unmark(void *object){
	assert(object);
	Object_unmark(*((Object**) object));
}

static void Runtime_runGC(Runtime *self){
	assert(self);

	// Unmark all allocations.
	Vector_each(&self->collectables, unmark); // TODO: check error

	// Mark all accessible allocations.
	Object_markRecursive(self->root_scope);

	// Move all accessible allocations to a new vector. Clean
	// the others.
	Vector leftovers;
	Vector_init(&leftovers, sizeof(Object*));
	for(int i = 0; i < self->collectables.size; i++){
		Object *item = NULL;
		Vector_fetch(&self->collectables, i, &item);
		printf("OLD: %p\n", item);
		fflush(stdout);
		if(item->gc_mark){
			Vector_append(&leftovers, &item);
		} else {
			Object_free(item);
			printf("- collected\n");
		}
	}

	// Delete old vector of pointers. Replace it.
	Vector_clean(&self->collectables);
	self->collectables = leftovers;
}



Object *Runtime_rawObject(Runtime *self){
	assert(self);

	Object *r = malloc(sizeof(Object));
	printf("NEW: %p\n", r);
	Object_init(r);

	Vector_append(&self->collectables, &r);

	printf("\n\nGC STATE\n");
	for(int i = 0; i < self->collectables.size; i++){
		Object *item = NULL;
		Vector_fetch(&self->collectables, i, &item);
		printf("CUR: %p\n", item);
	}
		printf("\n\n\n");


	if(self->collectables.size >= 100 &&
	   self->collectables.size % 100 == 0){ // TODO: make better algorithm here
		Runtime_runGC(self);
	}


	return r;
}


static Object *Runtime_activateOn(Runtime *runtime
	                            , Object *context
	                            , Object *object
	                            , int argc
	                            , Object **argv
	                            , Object *origin){
	assert(runtime);
	assert(context);
	assert(object);

	Object *special = Object_getDeep(object, "_activate");
	if(special){
		return Runtime_activateOn(runtime 
			                    , context
			                    , special
			                    , argc
			                    , argv
			                    , origin);
	}


	void *internal = Object_getDataDeep(object, "__activate");
	if(internal){
		CFunction cf = *((CFunction*) internal);
		return cf(runtime, context, origin, argc, argv);
	}



	// TODO: throw error - uncallable
	return origin;
}

Object *Runtime_activate(Runtime *runtime
	                   , Object *context
	                   , Object *object
	                   , int argc
	                   , Object **argv){
	return Runtime_activateOn(runtime 
			                , context
			                , object
			                , argc
			                , argv
			                , object);
}






void Runtime_init(Runtime *self){
	self->error = NULL;
	Vector_init(&self->collectables, sizeof(Object*));

	self->root_scope = Runtime_rawObject(self);
	// TODO: init basic types and core functions.

	Object *s = Runtime_rawObject(self);
	ImpString_init(s);
	Object_putShallow(self->root_scope, "string", s);

	Object *n = Runtime_rawObject(self);
	ImpNumber_init(n);
	Object_putShallow(self->root_scope, "number", n);

	Object *a = Runtime_rawObject(self);
	ImpAtom_init(a);
	Object_putShallow(self->root_scope, "atom", a);

	Object *printer = Runtime_rawObject(self);
	ImpPrinter_init(printer);
	Object_putShallow(self->root_scope, "print", printer);

	Object *setter = Runtime_rawObject(self);
	ImpSet_init(setter);
	Object_putShallow(self->root_scope, "set", setter);

	Object *definer = Runtime_rawObject(self);
	ImpDef_init(definer);
	Object_putShallow(self->root_scope, "def", definer);

	Object *closure = Runtime_rawObject(self);
	ImpClosure_init(closure);
	Object_putShallow(self->root_scope, "closure", closure);

	Object *tr = Runtime_rawObject(self);
	ImpBoolean_init(tr);
	ImpBoolean_setRaw(tr, true);
	Object_putShallow(self->root_scope, "true", tr);

	Object *fa = Runtime_rawObject(self);
	ImpBoolean_init(fa);
	ImpBoolean_setRaw(fa, false);
	Object_putShallow(self->root_scope, "false", tr);

	Object *whi = Runtime_rawObject(self);
	ImpWhile_init(whi);
	Object_putShallow(self->root_scope, "while", whi);
}


Object *Runtime_clone(Runtime *runtime, Object *object){

	// TODO: check for special and internal methods
	Object *special = Object_getDeep(object, "_clone");
	if(special){
		return Runtime_activateOn(runtime 
			                    , NULL
			                    , special
			                    , 0
			                    , NULL
			                    , object);
	}


	void *internal = Object_getDataDeep(object, "__clone");
	if(internal){
		CFunction cf = *((CFunction*) internal);
		return cf(runtime, NULL, object, 0, NULL);
	}
	Object *raw = Runtime_rawObject(runtime);
	Object_putShallow(raw, "_prototype", object);
	return raw;
}

static Object *Runtime_cloneField(Runtime *runtime, char *field){
	return Runtime_clone(runtime, Object_getDeep(runtime->root_scope, field));
}


static Object *Runtime_tokenToObject(Runtime *self, Object *scope, Token *token){
	switch(token->type){
	case TOKEN_ATOM:
		{
			Object *atom = Runtime_cloneField(self, "atom");
			ImpAtom_setRaw(atom, token->data.text);
			return atom;
		}
	case TOKEN_NUMBER:
		{
			Object *number = Runtime_cloneField(self, "number");
			ImpNumber_setRaw(number, token->data.number);
			return number;
		}
	case TOKEN_STRING:
		{
			Object *str = Runtime_cloneField(self, "string");
			ImpString_setRaw(str, token->data.text);
			return str;
		}
	// case TOKEN_NOT:
	// 	return newOf(self, scope, "!");
	// case TOKEN_AT:
	// 	return newOf(self, scope, "@");
	// case TOKEN_HASH:
	// 	return newOf(self, scope, "#");
	// case TOKEN_DOLLAR:
	// 	return newOf(self, scope, "$");
	// case TOKEN_PERCENT:
	// 	return newOf(self, scope, "%");
	// case TOKEN_CARROT:
	// 	return newOf(self, scope, "^");
	// case TOKEN_AMP:
	// 	return newOf(self, scope, "&");
	// case TOKEN_STAR:
	// 	return newOf(self, scope, "*");
	// case TOKEN_PLUS:
	// 	return newOf(self, scope, "+");
	// case TOKEN_QUESTION:
	// 	return newOf(self, scope, "?");
	// case TOKEN_COLON:
	// 	return newOf(self, scope, ":");
	// case TOKEN_SEMI:
	// 	return newOf(self, scope, ";");
	// case TOKEN_DASH:
	// 	return newOf(self, scope, "-");
	default:
		return NULL;
		break;
	}
}

Object *Runtime_executeInContext(Runtime *runtime
	                              , Object *context
	                              , ParseNode node){

	// setup scoping
	Object *scope = Runtime_rawObject(runtime);
	Object_putShallow(scope, "_prototype", context);

	// if leaf node, form value
	if(node.type == LEAF_NODE){
		return Runtime_tokenToObject(runtime, scope, node.contents.token);
	}


	// TODO: check that first sub is function type?


	Object *r = NULL;
	switch(node.type){
	case CALL_NODE:
		{
			// iterate through parse node... TODO: mark these in collection
			Object **subs = malloc(node.contents.non_leaf.argc * sizeof(Object*));
			for(int i = 0; i < node.contents.non_leaf.argc; i++){
				subs[i] = Runtime_executeInContext(runtime
					                             , scope
					                             , node.contents.non_leaf.argv[i]);
			}
			r = Runtime_activate(runtime
				               , context
				               , subs[0]
				               , node.contents.non_leaf.argc - 1
				               , subs + 1);
			free(subs);
		}
		break;
	case MACRO_NODE:
		// TODO: expand and execute macro
		break;
	case CLOSURE_NODE:
		{
			r = Runtime_cloneField(runtime, "closure");
			ImpClosure_compile(r, &node, context);
		}
		break;
	}

	return r;
}

Object *Runtime_execute(Runtime *self, char *code){
	ParseTree tree;
	ParseTree_init(&tree, code);

	// TODO: check tree.error
	Object *r = Runtime_executeInContext(self, self->root_scope, tree.root);
	ParseTree_clean(&tree);
	return r;
}




