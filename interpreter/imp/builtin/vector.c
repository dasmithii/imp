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


static Object *print_(Runtime *runtime
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


static Object *clone_(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:~ does not accept arguments.");
		return NULL;
	}
	Object *r = Runtime_simpleClone(runtime, caller);
	Vector *internal = malloc(sizeof(Vector));
	if(!internal){
		abort();
	}
	Vector_init(internal, sizeof(Object*));
	Object_putDataShallow(r, "__data", internal);

	return r;
}


static Object *append_(Runtime *runtime
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


static Object *prepend_(Runtime *runtime
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


static Object *insert_(Runtime *runtime
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


static Object *remove_(Runtime *runtime
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


static Object *removeBack_(Runtime *runtime
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


static Object *removeFront_(Runtime *runtime
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


static Object *copy_(Runtime *runtime
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
	Object_putShallow(r, "#", Object_rootPrototype(caller));
	return r;
}


static Object *get_(Runtime *runtime
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


static Object *collect_(Runtime *runtime
	                  , Object *context
	                  , Object *self
	                  , int argc
	                  , Object **argv){
	assert(ImpVector_isValid(self));
	Vector_clean(ImpVector_getRaw(self));
	return NULL;
}


static Object *mark_(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	Vector *internal = ImpVector_getRaw(caller);
	
	if(!internal){
		return NULL;
	}

	for(int i = 0; i < internal->size; i++){
		Object *element = *((Object**) Vector_hook(internal, i));
		Runtime_markRecursive(runtime, element);
	}
	return NULL;
}


static Object *asString_(Runtime *runtime
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
	Object_reference(r);
	ImpString_setRaw(r, "[");
	for(int i = 0; i < internal->size; i++){
		Object *item = *((Object**) Vector_hook(internal, i));
		Runtime_callMethod(runtime
			             , context
			             , r
			             , "concatenate"
			             , 1
			             , &item);
		if(i != internal->size - 1){
			ImpString_concatenateRaw(r, ", ");
		}
	}
	ImpString_concatenateRaw(r, "]");
	Object_unreference(r);
	return r;
}

static Object *size_(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);
	assert(ImpVector_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "Vector:size does not accept arguments");
	}
	Vector *internal = ImpVector_getRaw(caller);
	Object *r = Runtime_cloneField(runtime, "Number");
	ImpNumber_setRaw(r, (double) internal->size);
	return r;
}


void ImpVector_init(Object *self, Runtime *runtime){
	assert(self);

	BuiltIn_setId(self, BUILTIN_VECTOR);

	Runtime_registerCMethod(runtime, self, "collect", collect_);
	Runtime_registerCMethod(runtime, self, "mark", mark_);

	Runtime_registerCMethod(runtime, self, "print", print_);
	Runtime_registerCMethod(runtime, self, "~", clone_);
	Runtime_registerCMethod(runtime, self, "append", append_);
	Runtime_registerCMethod(runtime, self, "prepend", prepend_);
	Runtime_registerCMethod(runtime, self, "insert", insert_);
	Runtime_registerCMethod(runtime, self, "remove", remove_);
	Runtime_registerCMethod(runtime, self, "removeBack", removeBack_);
	Runtime_registerCMethod(runtime, self, "removeFront", removeFront_);
	Runtime_registerCMethod(runtime, self, "$", copy_);
	Runtime_registerCMethod(runtime, self, "get", get_);
	Runtime_registerCMethod(runtime, self, "size", size_);

	Runtime_registerCMethod(runtime, self, "asString", asString_);
}
