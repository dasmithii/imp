#include "vector.h"
#include "general.h"

Vector *ImpVector_getRaw(Object *self){
	return (Vector*) Object_getDataDeep(self, "__data");
}


static Object *ImpVector_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	Vector *raw = ImpVector_getRaw(caller);
	for(int i = 0; i < raw->size; i++){
		Object *element = *((Object**) Vector_hook(raw, i));
		Runtime_print(runtime, element);
	}
}


static Object *ImpVector_clone_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);

	Vector *internal = malloc(sizeof(Vector));
	Vector_init(internal, sizeof(Object*));
	Object_putDataShallow(r, "__data", internal);

	return r;
}


static Object *ImpVector_append_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	Vector_append(ImpVector_getRaw(caller), &argv[0]);
	return NULL;
}


static Object *ImpVector_prepend_internal(Runtime *runtime
	                                    , Object *context
	                                    , Object *caller
	                                    , int argc
	                                    , Object **argv){
	Vector_prepend(ImpVector_getRaw(caller), &argv[0]);
	return NULL;
}


static Object *ImpVector_insert_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	Object *index = argv[0];
	Object *element = argv[1];
	Vector_insert(ImpVector_getRaw(caller)
		        , (int) (ImpNumber_getRaw(index) + .5)
		        , element);
	return NULL;
}


static Object *ImpVector_remove_internal(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	Object *index = argv[0];
	Vector_remove(ImpVector_getRaw(caller)
		        , (int) (ImpNumber_getRaw(index) + .5));
	return NULL;
}


static Object *ImpVector_removeBack_internal(Runtime *runtime
	                                       , Object *context
	                                       , Object *caller
	                                       , int argc
	                                       , Object **argv){
	Vector_pop(ImpVector_getRaw(caller));
	return NULL;
}


static Object *ImpVector_removeFront_internal(Runtime *runtime
	                                        , Object *context
	                                        , Object *caller
	                                        , int argc
	                                        , Object **argv){
	Vector_popf(ImpVector_getRaw(caller));
	return NULL;
}

static Object *ImpVector_copy_internal(Runtime *runtime
	                                 , Object *context
	                                 , Object *caller
	                                 , int argc
	                                 , Object **argv){
	Vector *newInternal = Vector_clone(ImpVector_getRaw(caller));
	Object *r = Runtime_rawObject(runtime);
	Object_putDataShallow(r, "__data", newInternal);
	Object_putShallow(r, "_prototype", Object_rootPrototype(caller));
	return r;
}


static Object *ImpVector_get_internal(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	int index = (int) (ImpNumber_getRaw(argv[0]) + .5);
	return *((Object**) Vector_hook(ImpVector_getRaw(caller), index));
}



static Object *ImpVector_collect_internal(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	Vector_clean(ImpVector_getRaw(caller));
	return NULL;
}

static Object *ImpVector_mark_internal(Runtime *runtime
	                                , Object *context
	                                , Object *caller
	                                , int argc
	                                , Object **argv){
	Vector *internal = ImpVector_getRaw(caller);
	for(int i = 0; i < internal->size; i++){
		Object *element = *((Object**) Vector_hook(internal, i));
		Runtime_markRecursive(runtime, element);
	}
	return NULL;
}

void ImpVector_init(Object *self){
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
