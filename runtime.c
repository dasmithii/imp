#include "runtime.h"
#include "parser.h"
#include <string.h>
#include <stdio.h>

#include "builtin/general.h"
#include "builtin/route.h"
#include "builtin/number.h"
#include "builtin/string.h"
#include "builtin/print.h"
#include "builtin/def.h"
#include "builtin/set.h"
#include "builtin/closure.h"
#include "builtin/boolean.h"
#include "builtin/while.h"
#include "builtin/vector.h"
#include "builtin/return.h"
#include "c.h"




// Activates <object> with given arguments on the <origin> 
// object. This is particularly useful for method-type
// objects.
Object *Runtime_activateOn(Runtime *runtime
	                            , Object *context
	                            , Object *object
	                            , int argc
	                            , Object **argv
	                            , Object *origin){
	assert(runtime);
	assert(context);
	assert(Object_isValid(object));
	Object *r = NULL;

	Object_reference(context);
	Object_reference(object);
	for(int i = 0; i < argc; i++){
		Object_reference(argv[i]);
	}
	Object_reference(origin);



	Object *special = Object_getDeep(object, "_activate");
	void *internal = Object_getDataDeep(object, "__activate");
	if(special){
		r = Runtime_activateOn(runtime 
			                    , context
			                    , special
			                    , argc
			                    , argv
			                    , origin);
	} else if(internal){
		CFunction cf = *((CFunction*) internal);

		if(BuiltIn_id(object) == BUILTIN_CLOSURE){
			// make self first arg in argv
			int argc2 = argc + 1;
			Object **argv2 = malloc(sizeof(Object*) * argc2);
			argv2[0] = origin;
			for(int i = 1; i < argc2; i++){
				argv2[i] = argv[i - 1];
			}

			r = cf(runtime, context, object, argc2, argv2);  // TODO: deal with this
			free(argv2);
		} else {
			r = cf(runtime, context, object, argc, argv);
		}
	}


	Object_unreference(context);
	Object_unreference(object);
	for(int i = 0; i < argc; i++){
		Object_unreference(argv[i]);
	}
	Object_unreference(origin);


	if(special || internal){
		return r;
	}
	Runtime_throwString(runtime, "Object not callable.");
	return NULL;
}



Object *Runtime_activate(Runtime *runtime
	                   , Object *context
	                   , Object *object
	                   , int argc
	                   , Object **argv){
	assert(Object_isValid(object));
	assert(runtime);
	return Runtime_activateOn(runtime 
			                , context
			                , object
			                , argc
			                , argv
			                , object);
}



static void unmark(void *addr){
	assert(addr);
	Object *obj = *((Object**) addr);
	assert(Object_isValid(obj));

	Object_unmark(obj);

	assert(Object_isValid(obj));
}


static void Runtime_markRecursiveIfVolatile(Runtime *runtime, Object *object){
	assert(runtime);
	assert(Object_isValid(object));

	if(Object_referenceCount(object) > 0){  // some internal objects are reference counted / volatile 
		Runtime_markRecursive(runtime, object);
	}

	assert(Object_isValid(object));
}



static void Runtime_collectOne(Runtime *runtime, Object *object){
	assert(runtime);
	assert(Object_isValid(object));

	Object *special = Object_getDeep(object, "_collect");
	if(special){
		Runtime_activateOn(runtime 
			             , NULL
			             , special
			             , 0
			             , NULL
			             , object);
	} else {
		void *internal = Object_getDataDeep(object, "__collect");
		if(internal){
			CFunction cf = *((CFunction*) internal);
			cf(runtime, NULL, object, 0, NULL);
		}
	}
	assert(Object_isValid(object));
	Object_free(object);
}


void Runtime_markRecursive(Runtime *runtime, Object *object){
	assert(runtime);
	assert(Object_isValid(object));

	Object_markRecursive(object);

	assert(Object_isValid(object));


	Object *special = Object_getDeep(object, "_mark");
	if(special){
		Runtime_activateOn(runtime 
			             , NULL
			             , special
			             , 0
			             , NULL
			             , object);
	} else {
		void *internal = Object_getDataDeep(object, "__mark");
		if(internal){
			CFunction cf = *((CFunction*) internal);
			cf(runtime, NULL, object, 0, NULL);
		}
	}

	assert(Object_isValid(object));
}

static void Runtime_runGC(Runtime *self){
	assert(self);
	assert(self->gc_locks == 0);

	// Unmark all allocations.
	Vector_each(&self->collectables, unmark); // TODO: check error

	// mark volatile objects and dependencies
	for(int i = 0; i < self->collectables.size; i++){
		Object *object = *((Object**) Vector_hook(&self->collectables, i));
		Runtime_markRecursiveIfVolatile(self, object);
	}

	// Mark all accessible allocations.
	Runtime_markRecursive(self, self->root_scope);

	// Move all accessible allocations to a new vector. Clean
	// the others.
	Vector leftovers;
	Vector_init(&leftovers, sizeof(Object*));
	for(int i = 0; i < self->collectables.size; i++){
		Object *item = *((Object**) Vector_hook(&self->collectables, i));
		assert(Object_isValid(item));

		if(item->gc_mark){
			Vector_append(&leftovers, &item);
		} else { 
			Runtime_collectOne(self, item);
		}
	}

	// Delete old vector of pointers. Replace it.
	Vector_clean(&self->collectables);
	self->collectables = leftovers;
}



Object *Runtime_rawObject(Runtime *self){
	assert(self);

	int oc = Runtime_objectCount(self);

	if(self->gc_locks == 0 && oc >= 20 && oc % 20 == 0){
		Runtime_runGC(self);
	}


	// allocate record, and return new object
	Object *r = malloc(sizeof(Object));
	Object_init(r);
	assert(Object_isValid(r));
	Vector_append(&self->collectables, &r);

	return r;
}








void Runtime_init(Runtime *self){
	assert(self);
	self->gc_locks = 0;
	Runtime_lockGC(self);

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

	Object *route = Runtime_rawObject(self);
	ImpRoute_init(route);
	Object_putShallow(self->root_scope, "route", route);

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

	Object *vec = Runtime_rawObject(self);
	ImpVector_init(vec);
	Object_putShallow(self->root_scope, "vector", vec);

	Object *returner = Runtime_rawObject(self);
	ImpReturn_init(returner);
	Object_putShallow(self->root_scope, "return", returner);

	Runtime_unlockGC(self);
}


Object *Runtime_clone(Runtime *runtime, Object *object){
	assert(runtime);
	assert(Object_isValid(object));

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

Object *Runtime_cloneField(Runtime *runtime, char *field){
	return Runtime_clone(runtime, Object_getDeep(runtime->root_scope, field));
}


static Object *Runtime_tokenToObject(Runtime *self, Object *scope, Token *token){
	assert(self);
	assert(Object_isValid(scope));
	assert(token);

	switch(token->type){
	case TOKEN_ROUTE:
		{
			Object *route = Runtime_cloneField(self, "route");
			assert(token->data.text);
			ImpRoute_setRaw(route, token->data.text);
			return route;
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


void Runtime_setReturnValue(Runtime *self, Object *value){
	self->lastReturnValue = value;
}

void Runtime_clearReturnValue(Runtime *self){
	Runtime_setReturnValue(self, NULL);
}

Object *Runtime_returnValue(Runtime *self){
	return self->lastReturnValue;
}


Object *Runtime_executeInContext(Runtime *runtime
	                              , Object *scope
	                              , ParseNode node){
	assert(runtime);
	assert(Object_isValid(scope));
	Object *r = NULL;


	Object_reference(scope);

	// if leaf node, form value
	if(node.type == LEAF_NODE){
		r = Runtime_tokenToObject(runtime, scope, node.contents.token);
	}


	// TODO: check that first sub is function type?


	switch(node.type){
	case BLOCK_NODE:
		{
			Runtime_clearReturnValue(runtime);
			for(int i = 0; i < node.contents.non_leaf.argc; i++){
				Runtime_executeInContext(runtime
					                   , scope
					                   , node.contents.non_leaf.argv[i]);
			}
			r = Runtime_returnValue(runtime);
		}
		break;
	case CALL_NODE:
		{
			// iterate through parse node... TODO: mark these in collection
			Object **subs = malloc(node.contents.non_leaf.argc * sizeof(Object*));
			for(int i = 0; i < node.contents.non_leaf.argc; i++){
				subs[i] = Runtime_executeInContext(runtime
					                             , scope
					                             , node.contents.non_leaf.argv[i]);

				if(subs[i]){
					Object_reference(subs[i]);
				}
			}
			r = Runtime_activate(runtime
				               , scope
				               , subs[0]
				               , node.contents.non_leaf.argc - 1
				               , subs + 1);

			for(int i = 0; i < node.contents.non_leaf.argc; i++){
				if(subs[i]){
					Object_unreference(subs[i]);
				}
			}

			free(subs);
		}
		break;
	case MACRO_NODE:
		// TODO: expand and execute macro
		break;
	case CLOSURE_NODE:
		{
			r = Runtime_cloneField(runtime, "closure");
			Runtime_lockGC(runtime);
			ImpClosure_compile(runtime, r, &node, scope);
			Runtime_unlockGC(runtime);
		}
		break;
	}

	Object_unreference(scope);

	return r;
}

Object *Runtime_execute(Runtime *self, char *code){
	ParseTree tree;
	Object *r = NULL;

	int rc = ParseTree_init(&tree, code);
	if(rc){
		Runtime_throwString(self, tree.error);
	} else {
		r = Runtime_executeInContext(self, self->root_scope, tree.root);
	}

	ParseTree_clean(&tree);
	return r;
}



int Runtime_objectCount(Runtime *self){
	return self->collectables.size;
}


void Runtime_throwString(Runtime *runtime, char *exception){
	printf("Uncaught exception: %s\n", exception);
	abort();
}

void Runtime_print(Runtime *runtime, Object *context, Object *object){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(object));

	Object *special = Object_getDeep(object, "_print");
	if(special){
		Runtime_activateOn(runtime 
			             , context
			             , special
			             , 0
			             , NULL
			             , object);
	}

	void *internal = Object_getDataDeep(object, "__print");
	if(internal){
		CFunction cf = *((CFunction*) internal);
		cf(runtime, context, object, 0, NULL);
		return;
	}

	Object_print(object);
}


void Runtime_lockGC(Runtime *self){
	assert(self);
	self->gc_locks++;
}


void Runtime_unlockGC(Runtime *self){
	assert(self);
	self->gc_locks--;
	assert(self->gc_locks >= 0);
}


bool Runtime_isManaged(Runtime *self, Object *object){
	assert(self);
	assert(object);
	for(int i = 0; i < self->collectables.size; i++){
		Object *item = *((Object**) Vector_hook(&self->collectables, i));
		if(item == object){
			return true;
		}
	}
	return false;
}
