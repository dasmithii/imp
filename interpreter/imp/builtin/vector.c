#include "general.h"
#include "number.h"
#include "string.h"
#include "vector.h"




bool ImpVector_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_VECTOR;
}


Vector *ImpVector_getRaw(Object *self){
	assert(ImpVector_isValid(self));
	return (Vector*) Object_getDataDeep(self, "__data");
}


static Object *ImpVector_print_(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpVector_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:print does not accept arguments.");
		return NULL;
	}

	Vector *raw = ImpVector_getRaw(caller);
	for(int i = 0; i < raw->size; i++){
		Object *element = *((Object**) Vector_hook(raw, i));
		Runtime_print(runtime, context, element);
	}

	return NULL;
}


static Object *ImpVector_clone_(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:clone does not accept arguments.");
		return NULL;
	}
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);

	Vector *internal = malloc(sizeof(Vector));
	if(!internal){
		abort();
	}
	Vector_init(internal, sizeof(Object*));
	Object_putDataShallow(r, "__data", internal);

	return r;
}


static Object *ImpVector_append_(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	assert(ImpVector_isValid(caller));

	if(argc == 0){
		Runtime_throwString(runtime, "Vector:append requires arguments.");
		return NULL;
	}

	for(int i = 0; i < argc; i++){
		Vector_append(ImpVector_getRaw(caller), &argv[i]);
	}
	return NULL;
}


static Object *ImpVector_prepend_(Runtime *runtime
	                                    , Object *context
	                                    , Object *caller
	                                    , int argc
	                                    , Object **argv){
	assert(ImpVector_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "Vector:prepend requires exactly 1 argument.");
		return NULL;
	}
	Vector_prepend(ImpVector_getRaw(caller), &argv[0]);
	return NULL;
}


static Object *ImpVector_insert_(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 2){
		Runtime_throwString(runtime, "Vector:insert requires exactly 2 arguments.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "Vector:insert requires a number in its first argument.");
	} else {
		Vector *raw = ImpVector_getRaw(caller);
		int index   = (int) (ImpNumber_getRaw(argv[0]) + .5);
		if(index < 0 || index > raw->size){
			Runtime_throwString(runtime, "Vector:insert index out of bounds.");
		}
		Vector_insert(raw
		            , index
		            , argv[1]);
	}
	return NULL;
}


static Object *ImpVector_remove_(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "Vector:remove requires exactly 1 argument.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "Vector:remove requires a number as its argument.");
	} else {
		Vector *raw = ImpVector_getRaw(caller);
		int index   = (int) (ImpNumber_getRaw(argv[0]) + .5);
		if(index < 0 || index > raw->size){
			Runtime_throwString(runtime, "Vector:remove index out of bounds.");
		}
		Vector_remove(raw
		            , index);
	}
	return NULL;
}


static Object *ImpVector_removeBack_(Runtime *runtime
	                                       , Object *context
	                                       , Object *caller
	                                       , int argc
	                                       , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	Vector *raw = ImpVector_getRaw(caller);

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:removeBack does not accept arguments.");
	} else if(raw->size == 0){
		Runtime_throwString(runtime, "Vector:removeBack called on empty vector");
	} else{
		Vector_pop(raw);
	}

	return NULL;
}


static Object *ImpVector_removeFront_(Runtime *runtime
	                                        , Object *context
	                                        , Object *caller
	                                        , int argc
	                                        , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	Vector *raw = ImpVector_getRaw(caller);

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:removeFront does not accept arguments.");
	} else if(raw->size == 0){
		Runtime_throwString(runtime, "Vector:removeBack called on empty vector");
	} else {
		Vector_popf(raw);
	}

	return NULL;
}


static Object *ImpVector_copy_(Runtime *runtime
	                                 , Object *context
	                                 , Object *caller
	                                 , int argc
	                                 , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	Vector *raw = ImpVector_getRaw(caller);

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:copy does not accept arguments");
		return NULL;
	}

	Vector *newInternal = Vector_clone(ImpVector_getRaw(caller));
	Object *r = Runtime_rawObject(runtime);

	Object_putDataShallow(r, "__data", newInternal);
	Object_putShallow(r, "_prototype", Object_rootPrototype(caller));
	return r;
}


static Object *ImpVector_get_(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "Vector:get requires exactly 1 arguments.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "Vector:get requires a number in its first argument.");
	} else {
		Vector *raw = ImpVector_getRaw(caller);
		int index   = (int) (ImpNumber_getRaw(argv[0]) + .5);
		if(index < 0 || index > raw->size){
			Runtime_throwString(runtime, "Vector:insert index out of bounds.");
		} else {
			return *((Object**) Vector_hook(ImpVector_getRaw(caller), index));
		}
	}
	return NULL;
}


static Object *ImpVector_collect_(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	assert(ImpVector_isValid(caller));
	Vector_clean(ImpVector_getRaw(caller));
	return NULL;
}


static Object *ImpVector_mark_(Runtime *runtime
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

static Object *ImpVector_asString_(Runtime *runtime
	                                     , Object *context
	                                     , Object *caller
	                                     , int argc
	                                     , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:asString does not accept arguments");
	}
	Vector *internal = ImpVector_getRaw(caller);
	Object *r = Runtime_cloneField(runtime, "String");
	ImpString_setRaw(r, "[");
	for(int i = 0; i < internal->size; i++){
		Object *item = *((Object**) Vector_hook(internal, i));
		ImpString_concatenate_(runtime, context, r, 1, &item);
		if(i != internal->size - 1){
			ImpString_concatenateRaw(r, ", ");
		}
	}
	ImpString_concatenateRaw(r, "]");
	return r;
}


void ImpVector_init(Object *self){
	assert(self);

	BuiltIn_setId(self, BUILTIN_VECTOR);

	Object_registerCMethod(self, "__collect", ImpVector_collect_);
	Object_registerCMethod(self, "__mark", ImpVector_mark_);


	Object_registerCMethod(self, "__print", ImpVector_print_);
	Object_registerCMethod(self, "__clone", ImpVector_clone_);
	Object_registerCMethod(self, "__append", ImpVector_append_);
	Object_registerCMethod(self, "__prepend", ImpVector_prepend_);
	Object_registerCMethod(self, "__insert", ImpVector_insert_);
	Object_registerCMethod(self, "__remove", ImpVector_remove_);
	Object_registerCMethod(self, "__removeBack", ImpVector_removeBack_);
	Object_registerCMethod(self, "__removeFront", ImpVector_removeFront_);
	Object_registerCMethod(self, "__copy", ImpVector_copy_);
	Object_registerCMethod(self, "__get", ImpVector_get_);

	Object_registerCMethod(self, "__asString", ImpVector_asString_);
}
