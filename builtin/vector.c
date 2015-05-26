#include "vector.h"
#include "general.h"
#include "number.h"




bool ImpVector_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_VECTOR;
}


Vector *ImpVector_getRaw(Object *self){
	assert(ImpVector_isValid(self));
	return (Vector*) Object_getDataDeep(self, "__data");
}


static Object *ImpVector_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpVector_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "vector:print does not accept arguments.");
		return NULL;
	}

	Vector *raw = ImpVector_getRaw(caller);
	for(int i = 0; i < raw->size; i++){
		Object *element = *((Object**) Vector_hook(raw, i));
		Runtime_print(runtime, context, element);
	}

	return NULL;
}


static Object *ImpVector_clone_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "vector:clone does not accept arguments.");
		return NULL;
	}

	Object_reference(caller);
	Object *r = Runtime_rawObject(runtime);
	Object_unreference(caller);

	Object_putShallow(r, "_prototype", caller);

	Vector *internal = malloc(sizeof(Vector));
	if(!internal){
		abort();
	}
	Vector_init(internal, sizeof(Object*));
	Object_putDataShallow(r, "__data", internal);

	return r;
}


static Object *ImpVector_append_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	assert(ImpVector_isValid(caller));

	if(argc == 0){
		Runtime_throwString(runtime, "vector:append requires arguments.");
		return NULL;
	}

	for(int i = 0; i < argc; i++){
		Vector_append(ImpVector_getRaw(caller), &argv[i]);
	}
	return NULL;
}


static Object *ImpVector_prepend_internal(Runtime *runtime
	                                    , Object *context
	                                    , Object *caller
	                                    , int argc
	                                    , Object **argv){
	assert(ImpVector_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "vector:prepend requires exactly 1 argument.");
		return NULL;
	}
	Vector_prepend(ImpVector_getRaw(caller), &argv[0]);
	return NULL;
}


static Object *ImpVector_insert_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 2){
		Runtime_throwString(runtime, "vector:insert requires exactly 2 arguments.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "vector:insert requires a number in its first argument.");
	} else {
		Vector *raw = ImpVector_getRaw(caller);
		int index   = (int) (ImpNumber_getRaw(argv[0]) + .5);
		if(index < 0 || index > raw->size){
			Runtime_throwString(runtime, "vector:insert index out of bounds.");
		}
		Vector_insert(raw
		            , index
		            , argv[1]);
	}
	return NULL;
}


static Object *ImpVector_remove_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "vector:remove requires exactly 1 argument.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "vector:remove requires a number as its argument.");
	} else {
		Vector *raw = ImpVector_getRaw(caller);
		int index   = (int) (ImpNumber_getRaw(argv[0]) + .5);
		if(index < 0 || index > raw->size){
			Runtime_throwString(runtime, "vector:remove index out of bounds.");
		}
		Vector_remove(raw
		            , index);
	}
	return NULL;
}


static Object *ImpVector_removeBack_internal(Runtime *runtime
	                                       , Object *context
	                                       , Object *caller
	                                       , int argc
	                                       , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	Vector *raw = ImpVector_getRaw(caller);

	if(argc != 0){
		Runtime_throwString(runtime, "vector:removeBack does not accept arguments.");
	} else if(raw->size == 0){
		Runtime_throwString(runtime, "vector:removeBack called on empty vector");
	} else{
		Vector_pop(raw);
	}

	return NULL;
}


static Object *ImpVector_removeFront_internal(Runtime *runtime
	                                        , Object *context
	                                        , Object *caller
	                                        , int argc
	                                        , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	Vector *raw = ImpVector_getRaw(caller);

	if(argc != 0){
		Runtime_throwString(runtime, "vector:removeFront does not accept arguments.");
	} else if(raw->size == 0){
		Runtime_throwString(runtime, "vector:removeBack called on empty vector");
	} else {
		Vector_popf(raw);
	}

	return NULL;
}


static Object *ImpVector_copy_internal(Runtime *runtime
	                                 , Object *context
	                                 , Object *caller
	                                 , int argc
	                                 , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	Vector *raw = ImpVector_getRaw(caller);

	if(argc != 0){
		Runtime_throwString(runtime, "vector:copy does not accept arguments");
		return NULL;
	}

	Vector *newInternal = Vector_clone(ImpVector_getRaw(caller));

	Object_reference(caller);
	Object *r = Runtime_rawObject(runtime);
	Object_unreference(caller);

	Object_putDataShallow(r, "__data", newInternal);
	Object_putShallow(r, "_prototype", Object_rootPrototype(caller));
	return r;
}


static Object *ImpVector_get_internal(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "vector:get requires exactly 1 arguments.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "vector:get requires a number in its first argument.");
	} else {
		Vector *raw = ImpVector_getRaw(caller);
		int index   = (int) (ImpNumber_getRaw(argv[0]) + .5);
		if(index < 0 || index > raw->size){
			Runtime_throwString(runtime, "vector:insert index out of bounds.");
		} else {
			return *((Object**) Vector_hook(ImpVector_getRaw(caller), index));
		}
	}
	return NULL;
}


static Object *ImpVector_collect_internal(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	assert(ImpVector_isValid(caller));
	Vector_clean(ImpVector_getRaw(caller));
	return NULL;
}


static Object *ImpVector_mark_internal(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	// base vector is empty
	if(Object_hasKeyShallow(caller, "__id")){
		return NULL;
	}

	Vector *internal = ImpVector_getRaw(caller);
	assert(internal);
	for(int i = 0; i < internal->size; i++){
		Object *element = *((Object**) Vector_hook(internal, i));
		Runtime_markRecursive(runtime, element);
	}
	return NULL;
}


void ImpVector_init(Object *self){
	assert(self);

	BuiltIn_setId(self, BUILTIN_VECTOR);

	Object_registerCMethod(self, "__collect", ImpVector_collect_internal);
	Object_registerCMethod(self, "__mark", ImpVector_mark_internal);


	Object_registerCMethod(self, "__print", ImpVector_print_internal);
	Object_registerCMethod(self, "__clone", ImpVector_clone_internal);
	Object_registerCMethod(self, "__append", ImpVector_append_internal);
	Object_registerCMethod(self, "__prepend", ImpVector_prepend_internal);
	Object_registerCMethod(self, "__insert", ImpVector_insert_internal);
	Object_registerCMethod(self, "__remove", ImpVector_remove_internal);
	Object_registerCMethod(self, "__removeBack", ImpVector_removeBack_internal);
	Object_registerCMethod(self, "__removeFront", ImpVector_removeFront_internal);
	Object_registerCMethod(self, "__copy", ImpVector_copy_internal);
	Object_registerCMethod(self, "__get", ImpVector_get_internal);
}
