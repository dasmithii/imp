#include <stdlib.h>
#include <stdbool.h>

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>

#ifdef TOOLBOX_STACK_H_
#undef Stack_init
#undef Stack_clean
#undef Stack_push
#undef Stack_pop
#undef Stack_hookTop
#undef Stack_fetchTop
#undef Stack_size
#undef Stack_isEmpty
#undef Stack_clear
#endif



#define MIN_STACK_CAPACITY 8
#define MIN_STACK_LOAD .1   


typedef struct {
	Object **objects;
	size_t size;
	size_t capacity;
} Internal;


static void push(Internal *stack, Object *item){
	if(stack->size == stack->capacity){
		stack->capacity *= 2;
		stack->objects = realloc(stack->objects, stack->capacity * sizeof(Object*));
	}
	stack->objects[stack->size] = item;
	stack->size++;
}


static Object *pop(Internal *stack){
	Object *r = stack->objects[stack->size - 1];

	stack->size--;
	if(stack->capacity > MIN_STACK_CAPACITY && 
	  (float) stack->size / (float) stack->capacity < MIN_STACK_LOAD){
		stack->capacity = stack->size;
		if(stack->size < MIN_STACK_CAPACITY){
			stack->capacity = MIN_STACK_CAPACITY;
		}
		stack->objects = realloc(stack->objects, stack->capacity * sizeof(Object*));
	}

	return r;
}


static Object *top(Internal *stack){
	if(stack->size == 0){
		return NULL;
	}
	return stack->objects[stack->size - 1];
}


Object *Stack_clone(Runtime *runtime
	              , Object *context
	              , Object *base
	              , int argc
	              , Object **argv){
	Object *r = Runtime_simpleClone(runtime, base);
	Internal *data = malloc(sizeof(Internal));
	if(!data){
		abort();
	}
	data->size = 0;
	data->capacity = MIN_STACK_CAPACITY;
	data->objects = malloc(MIN_STACK_CAPACITY * sizeof(Object*));
	if(!data->objects){
		abort();
	}
	Object_putDataShallow(r, "__data", data);
	return r;
}


static Internal *getRaw(Object *stack){
	return Object_getDataDeep(stack, "__data");
}


Object *Stack_push(Runtime *runtime
	             , Object *context
	             , Object *self
	             , int argc
	             , Object **argv){
	if(argc == 0){
		Runtime_throwString(runtime, "Stack:push requires arguments");
	}
	Internal *internal = getRaw(self);
	for(int i = 0; i < argc; i++){
		push(internal, argv[i]);
	}
	return NULL;
}



Object *Stack_pop(Runtime *runtime
	            , Object *context
	            , Object *self
	            , int argc
	            , Object **argv){
	Internal *internal = getRaw(self);

	if(argc == 0){
		if(internal->size == 0){
			Runtime_throwString(runtime, "Stack:pop failed (stack empty)");
		}
		return pop(internal);
	} else if(argc == 1){
		if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
			Runtime_throwString(runtime, "Stack:pop accepts only a number as arguments");
		}
		const int nToPop = (int) ImpNumber_getRaw(argv[0]);
		if(nToPop > internal->size){
			Runtime_throwString(runtime, "Stack:pop failed (stack too small)");
		}
		if(nToPop <= 0){
			Runtime_throwString(runtime, "Stack:pop failed (invalid nToPop)");
		}
		for(int i = 0; i < nToPop; i++){
			pop(internal);
		}
		return NULL; // TODO: return list of elements popped
	} else {
		Runtime_throwString(runtime, "Stack:pop accepts no more than one argument");
	}
	return NULL;
}


Object *Stack_top(Runtime *runtime
	            , Object *context
	            , Object *self
	            , int argc
	            , Object **argv){
	Internal *internal = getRaw(self);

	if(argc != 0){
		Runtime_throwString(runtime, "Stack:top does not accept arguments");
	}

	return top(internal);
}



Object *Stack_size(Runtime *runtime
	             , Object *context
	             , Object *self
	             , int argc
	             , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Stack:size does not accept arguments");
	}

	Internal *internal = getRaw(self);
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, (double) internal->size);
	return r;
}


Object *Stack_mark(Runtime *runtime
	             , Object *context
	             , Object *self
	             , int argc
	             , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Stack:mark does not accept arguments");
	}

	Internal *internal = getRaw(self);
	if(internal){
		for(int i = 0; i < internal->size; i++){
			Runtime_markRecursive(runtime, internal->objects[i]);
		}
	}
	return NULL;
}



Object *Stack_collect(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Stack:mark does not accept arguments");
	}

	Internal *internal = getRaw(self);
	if(internal){
		free(internal->objects);
		free(internal);
	}
	return NULL;
}

