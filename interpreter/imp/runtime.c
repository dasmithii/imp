#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <imp/builtin/general.h>
#include <imp/builtin/route.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>
#include <imp/builtin/closure.h>
#include <imp/builtin/vector.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/base.h>
#include <imp/builtin/miscellaneous.h>
#include <imp/c.h>
#include <imp/parser.h>
#include <imp/runtime.h>




typedef struct {
	jmp_buf env;
	Object *catcher;
} TryData;


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
	assert(Object_isValid(object));

	Object_reference(context);
	Object_reference(object);
	for(int i = 0; i < argc; i++){
		Object_reference(argv[i]);
	}
	Object_reference(origin);

	Object *r = NULL;

	Object *special = Object_getDeep(object, "_activate");
	Object *internal = NULL;
	if(special){
		r = Runtime_activateOn(runtime, context, special, argc, argv, origin);
	} else {
		internal = Object_getDataDeep(object, "__activate");
		if(!internal){
			Runtime_throwString(runtime, "object not activatable");
			goto fuckit;
		}

		CFunction cf = *((CFunction*) internal);

		if(Object_hasKeyDeep(object, "__privilege")){

			// execute with arguments that haven't yet been 
			// dereferenced
			r = cf(runtime, context, origin, argc, argv);
			
		} else if(BuiltIn_id(object) == BUILTIN_CLOSURE){

			// execute with dereferenced arguments prefixed
			// with 'self' for use in closure
			const int argc2 = argc + 1;
			Object **argv2 = malloc(sizeof(Object*) * argc2);
			if(!argv2){
				abort();
			}
			argv2[0] = origin;
			for(int i = 1; i < argc2; i++){
				argv2[i] = unrouteInContext(argv[i - 1], context);
				Object_reference(argv2[i]);
			}

			r = cf(runtime, context, object, argc2, argv2);  // TODO: deal with this

			for(int i = 1; i < argc2; i++){
				Object_unreference(argv2[i]);
			}
			free(argv2);

		} else {

			// execute with dereferenced arguments
			Object **argv2 = malloc(sizeof(Object*) * argc);
			if(!argv2){
				abort();
			}
			for(int i = 0; i < argc; i++){
				argv2[i] = unrouteInContext(argv[i], context);
				Object_reference(argv2[i]);
			}

			r = cf(runtime, context, origin, argc, argv2);  // TODO: deal with this
			
			for(int i = 0; i < argc; i++){
				Object_unreference(argv2[i]);
			}
			free(argv2);
		}
	}


	fuckit:

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
	Object_unmark(obj);
}


void Runtime_markRecursive(Runtime *runtime, Object *object){
	assert(runtime);
	assert(Object_isValid(object));

	if(object->gc_mark){
		return;
	}
	object->gc_mark = true;

	if(Object_hasSpecialMethod(object, "mark")){
		Runtime_callSpecialMethod(runtime
			                    , NULL
			                    , object
			                    , "mark"
			                    , 0
			                    , NULL);
	}

	for(int i = 0; i < object->slotCount; i++){
		if(!Slot_isPrimitive(object->slots + i)){
			Runtime_markRecursive(runtime, Slot_object(object->slots + i));
		}
	}
}


static void Runtime_runGC(Runtime *self){
	assert(self);
	assert(self->gc_locks == 0);
	assert(self->gc_on == false);
	self->gc_on = true;

	// Unmark all allocations.
	Vector_each(&self->collectables, unmark); // TODO: check error

	// mark volatile objects and dependencies
	for(int i = 0; i < self->collectables.size; i++){
		Object *object = *((Object**) Vector_hook(&self->collectables, i));
		if(Object_referenceCount(object) > 0){
			Runtime_markRecursive(self, object);
		}
	}

	// Mark all accessible allocations.
	Runtime_markRecursive(self, self->root_scope);
	if(self->lastReturnValue){
		Runtime_markRecursive(self, self->lastReturnValue);
	}

	// Move all accessible allocations to a new vector. Call 
	// onCollect methods now, before freeing any objects.
	Vector leftovers;
	Vector_init(&leftovers, sizeof(Object*));
	for(int i = 0; i < self->collectables.size; i++){
		Object *item = *((Object**) Vector_hook(&self->collectables, i));
		assert(Object_isValid(item));

		if(item->gc_mark){
			Vector_append(&leftovers, &item);
		} else {
			if(Object_hasSpecialMethod(item, "collect")){
				Runtime_callSpecialMethod(self
					                    , NULL
					                    , item
					                    , "collect"
					                    , 0
					                    , NULL);
			}
		}
	}

	// Delete old vector of pointers. Replace it.
	Vector_clean(&self->collectables);
	self->collectables = leftovers;

	// free old objects
	for(int i = 0; i < self->collectables.size; i++){
		Object *item = *((Object**) Vector_hook(&self->collectables, i));
		if(!item->gc_mark){
			Object_free(item);
		}
	}

	self->gc_on = false;
}


Object *Runtime_rawObject(Runtime *self){
	assert(self);

	int oc = Runtime_objectCount(self);

	if(self->gc_on == false       &&
	   self->gc_locks == 0        &&
	   oc >= 20 && oc % 20 == 0){
		Runtime_runGC(self);
	}


	// allocate record, and return new object
	Object *r = malloc(sizeof(Object));
	if(!r){
		abort();
	}
	Object_init(r);
	assert(Object_isValid(r));
	Vector_append(&self->collectables, &r);

	return r;
}


void Runtime_init(Runtime *self, char *root, int argc, char **argv){
	assert(self);

	self->argc = argc;
	self->argv = argv;


	self->gc_locks = 0;
	self->gc_on = false;
	Runtime_lockGC(self);

	Stack_init(&self->tryStack, sizeof(TryData));

	self->error = NULL;
	Vector_init(&self->collectables, sizeof(Object*));

	// IMPORTANT: be careful while messing with the order of 
	// builtin initializations (!).

	self->Object = Runtime_rawObject(self);
	ImpBase_init(self->Object, self);

	self->root_scope = Runtime_make(self, Object);
	Object_putShallow(self->root_scope, "self", self->root_scope);

	#define IMP_INIT_IN_SLOT(OBJECT, NAME)                       \
		self->OBJECT = Runtime_make(self, Object);               \
		Object_reference(self->OBJECT);                          \
		Object_putShallow(self->root_scope, NAME, self->OBJECT); \
		Imp##OBJECT##_init(self->OBJECT, self)
	#define IMP_INIT(NAME) IMP_INIT_IN_SLOT(NAME, #NAME)

	IMP_INIT(String);
	IMP_INIT(Number);
	IMP_INIT(Route);
	IMP_INIT(Closure);
	IMP_INIT(Vector);
	IMP_INIT_IN_SLOT(Importer, "import");
	Object_putShallow(self->Importer, "rootPath", root);

	ImpMisc_init(self->root_scope, self);

	Runtime_unlockGC(self);
}


Object *Runtime_simpleClone(Runtime *runtime, Object *base){
	Object_reference(base);
	Object *r = Runtime_rawObject(runtime);
	Object_unreference(base);

	Object_putShallow(r, "#", base);
	return r;
}


Object *Runtime_clone(Runtime *runtime, Object *base){
	assert(runtime);
	assert(Object_isValid(base));
	assert(Runtime_isManaged(runtime, base));

	if(Object_hasSpecialMethod(base, "~")){
		return Runtime_callSpecialMethod(runtime
			                        , NULL
			                        , base
			                        , "~"
			                        , 0
			                        , NULL);
	} else {
		return Runtime_simpleClone(runtime, base);
	}
}


Object *Runtime_cloneField(Runtime *runtime, char *field){
	assert(runtime);
	assert(field);
	return Runtime_clone(runtime, Object_getDeep(runtime->root_scope, field));
}


static Object *Runtime_tokenToObject(Runtime *self, Object *scope, Token *token){
	assert(self);
	assert(Object_isValid(scope));
	assert(token);

	Object *r = NULL;
	Object_reference(scope);

	switch(token->type){
	case TOKEN_ROUTE:
		{
			Object *route = Runtime_clone(self, self->Route);
			assert(token->data.text);
			ImpRoute_setRaw(route, token->data.text);
			r = route;
			break;
		}
	case TOKEN_NUMBER:
		{
			Object *number = Runtime_cloneField(self, "Number");
			ImpNumber_setRaw(number, token->data.number);
			r = number;
			break;
		}
	case TOKEN_STRING:
		{
			Object *str = Runtime_cloneField(self, "String");
			ImpString_setRaw(str, token->data.text);
			r = str;
			break;
		}
	default:
		break;
	}

	Object_unreference(scope);
	return r;
}


void Runtime_setReturnValue(Runtime *self, Object *value){
	assert(self);
	// value == null is allowed
	self->lastReturnValue = value;
	self->returnWasCalled = true;
}


void Runtime_clearReturnValue(Runtime *self){
	assert(self);
	self->lastReturnValue = NULL;
	self->returnWasCalled = false;
}


Object *Runtime_executeInContext(Runtime *runtime
	                           , Object *scope
	                           , ParseNode node){
	assert(runtime);
	assert(Object_isValid(scope));
	Object *r = NULL;

	Object_reference(scope);

	// // check if block has only one statement
	// if(node.type == BLOCK_NODE  && node.contents.non_leaf.argc == 1 &&
	//    (node.contents.non_leaf.argv[0].type != LEAF_NODE ||
	//    	node.contents.non_leaf.argv[0].contents.token->type != TOKEN_SOFT_OPEN)){
	// 	node.contents.non_leaf.argv[0].type = CALL_NODE;
	// }

	switch(node.type){
	case LEAF_NODE:
		{
			r = Runtime_tokenToObject(runtime, scope, node.contents.token);
			break;
		}
	case BLOCK_NODE:
		{
			Runtime_clearReturnValue(runtime);

			// // check if block has only one statement
			// if(node.contents.non_leaf.argc >= 1  &&
			//    (node.contents.non_leaf.argv[0].type != LEAF_NODE ||
			//    	(node.contents.non_leaf.argv[0].contents.token->type != TOKEN_SOFT_OPEN &&
			//    	 node.contents.non_leaf.argv[0].contents.token->type != TOKEN_HARD_OPEN))){
			// 	printf("hey\n");
			// 	ParseNode_print(&node.contents.non_leaf.argv[0]);
			// }


			for(int i = 0; i < node.contents.non_leaf.argc; i++){
				Runtime_executeInContext(runtime
					                   , scope
					                   , node.contents.non_leaf.argv[i]);
				if(runtime->returnWasCalled){
					break;
				}
			}
			r = runtime->lastReturnValue;
			Runtime_clearReturnValue(runtime);
		}
		break;
	case CALL_NODE:
		{
			// iterate through parse node... TODO: mark these in collection
			Object **subs = malloc(node.contents.non_leaf.argc * sizeof(Object*));
			if(!subs){
				abort();
			}
			for(int i = 0; i < node.contents.non_leaf.argc; i++){
				subs[i] = Runtime_executeInContext(runtime
					                             , scope
					                             , node.contents.non_leaf.argv[i]);

				if(subs[i]){
					Object_reference(subs[i]);
				}
			}

			const int argc = node.contents.non_leaf.argc - 1;
			Object **argv =  subs + 1;

			if(BuiltIn_id(subs[0]) == BUILTIN_ROUTE           &&
			   strcmp(ImpRoute_getRaw(subs[0]), "try") == 0){
				if(argc != 2){
					Runtime_throwString(runtime, "try requires exactly two arguments");
				}
				TryData data;
				data.catcher = argv[1];
				// push TryData
				if(!setjmp(data.env)){
					Stack_push(&runtime->tryStack, &data);
					r = Runtime_activate(runtime
		               , scope
		               , argv[0]
		               , 0
		               , NULL);
					Stack_pop(&runtime->tryStack);
				}
				// pop TryData
			} else {
				r = Runtime_activate(runtime
		               , scope
		               , subs[0]
		               , argc
		               , argv);
			}

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
			r = Runtime_cloneField(runtime, "Closure");
			Runtime_lockGC(runtime);
			ImpClosure_compile(runtime, r, &node, scope);
			Runtime_unlockGC(runtime);
		}
		break;
	}

	Object_unreference(scope);

	return r;
}


Object *Runtime_executeSourceInContext(Runtime *self
	                                 , char *code
	                                 , Object *context){
	assert(self);
	assert(code); // todo ebnf check code

	ParseTree tree;
	Object *r = NULL;

	int rc = ParseTree_init(&tree, code);
	if(rc){
		Runtime_throwString(self, tree.error);
	} else {
		r = Runtime_executeInContext(self, context, tree.root);
	}

	ParseTree_clean(&tree);
	return r;
}


Object *Runtime_executeSource(Runtime *self, char *code){
	return Runtime_executeSourceInContext(self, code, self->root_scope);
}


int Runtime_objectCount(Runtime *self){
	assert(self);
	return self->collectables.size;
}


void Runtime_throw(Runtime *runtime, Object *exception){
	assert(runtime);
	assert(exception);

	Object_reference(exception);

	if(Stack_size(&runtime->tryStack) == 0){
		fprintf(stderr, "Uncaught exception: ");
		if(BuiltIn_id(exception) == BUILTIN_STRING){
			fprintf(stderr, "%s", ImpString_getRaw(exception));
		} else {
			Runtime_print(runtime, NULL, exception);
		}
		fprintf(stderr, ".\n");
		exit(1);
	}
	TryData deepestTry;
	Stack_fetchTop(&runtime->tryStack, &deepestTry);
	Stack_pop(&runtime->tryStack);
	Object_reference(deepestTry.catcher);

	Runtime_activate(runtime, runtime->root_scope, deepestTry.catcher, 1, &exception);
	Object_unreference(deepestTry.catcher);
	Object_unreference(exception);
	
	longjmp(deepestTry.env, 0);
}


void Runtime_throwString(Runtime *runtime, char *exception){
	assert(runtime);
	assert(exception);

	Object *obj = Runtime_cloneField(runtime, "String");
	ImpString_setRaw(obj, exception);
	Runtime_throw(runtime, obj);
}


void Runtime_throwFormatted(Runtime *runtime, const char *format, ...){
    va_list args;
    va_start(args, format);

    char str[256];
    vsprintf(str, format, args);
    Runtime_throwString(runtime, str);

    fprintf(stderr, ".\n");
    va_end(args);
}


void Runtime_print(Runtime *runtime, Object *context, Object *object){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(object));

	Object_reference(context);
	Object_reference(object);

	Object *special = Object_getDeep(object, "_print");
	void *internal = Object_getDataDeep(object, "__print");
	if(special){
		Runtime_activateOn(runtime 
			             , context
			             , special
			             , 0
			             , NULL
			             , object);
	} else if(internal){
		CFunction cf = *((CFunction*) internal);
		cf(runtime, context, object, 0, NULL);
	} else {
		Object_print(object);
	}

	Object_unreference(context);
	Object_unreference(object);
}


void Runtime_lockGC(Runtime *self){
	assert(self);
	assert(self->gc_on == false);
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

Object *Runtime_callSpecialMethod(Runtime *runtime
	                     , Object *context
	                     , Object *object
	                     , char *methodName
	                     , int argc
	                     , Object **argv){
	// try <object>:_<methodName>
	char buf[64];
	sprintf(buf, "_%s", methodName);
	Object *method = Object_getDeep(object, buf);
	if(method){
		return Runtime_activateOn(runtime, context, method, argc, argv, object);
	}
	Runtime_throwFormatted(runtime, "method '%s' does not exist", buf);
	return NULL;
}


Object *Runtime_callMethod(Runtime *runtime
	                     , Object *context
	                     , Object *object
	                     , char *methodName
	                     , int argc
	                     , Object **argv){
	// try <object>:<methodName>
	Object *method = Object_getDeep(object, methodName);
	if(method){
		return Runtime_activateOn(runtime, context, method, argc, argv, object);
	}
	// try <object>:<methodName>
	return Runtime_callSpecialMethod(runtime, context, object, methodName, argc, argv);
}

