#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>


typedef struct Node {
	Object *value;
	struct Node *next;
} Node;


typedef struct {
	Node *head;
	Node *tail;
} Internal;


Internal *getRaw(Object *self){
	return (Internal*) Object_getDataDeep(self, "__data");
}


Object *Queue_clone(Runtime *runtime
	              , Object *context
	              , Object *base
	              , int argc
	              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Queue:clone does not accept arguments");
	}
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", base);
	Internal *data = calloc(1, sizeof(Internal));
	if(!data){
		abort();
	}
	Object_putDataShallow(r, "__data", data);
	return r;
}


Object *Queue_push(Runtime *runtime
	             , Object *context
	             , Object *self
	             , int argc
	             , Object **argv){
	if(argc == 0){
		Runtime_throwString(runtime, "Queue:push requires arguments");
	}

	Internal *internal = getRaw(self);

	for(int i = 0; i < argc; i++){
		Node *newNode = malloc(sizeof(Node));
		if(!newNode){
			abort();
		}
		newNode->value = argv[i];
		newNode->next = NULL;

		if(!internal->head){
			internal->head = newNode;
			internal->tail = newNode;
		} else {
			internal->tail->next = newNode;
			internal->tail = newNode;
		}
	}

	return NULL;
}


Object *Queue_peak(Runtime *runtime
	            , Object *context
	            , Object *self
	            , int argc
	            , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Queue:peak does not accept arguments");
	}

	Internal *internal = getRaw(self);
	if(!internal->head){
		Runtime_throwString(runtime, "Queue:peak called on empty queue");
		return NULL;
	}
	return internal->head->value;
}


Object *Queue_pop(Runtime *runtime
	            , Object *context
	            , Object *self
	            , int argc
	            , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Queue:pop does not accept arguments");
	}

	Object *r = Queue_peak(runtime, context, self, 0, NULL);
	Internal *internal = getRaw(self);
	Node *n = internal->head->next;
	free(internal->head);
	internal->head = n;
	if(!n){
		internal->tail = NULL;
	}
	return r;
}


Object *Queue_isEmpty(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Queue:Queue_isEmpty does not accept arguments");
	}
	Object *r = Runtime_cloneField(runtime, "Number");
	Internal *internal = getRaw(self);
	if(internal->head){
		ImpNumber_setRaw(r, 0);
	} else {
		ImpNumber_setRaw(r, 1);
	}
	return r;
}


Object *Queue_mark(Runtime *runtime
	             , Object *context
	             , Object *self
	             , int argc
	             , Object **argv){
	Internal *internal = getRaw(self);
	if(!internal){
		return NULL;
	}
	Node *node = internal->head;
	while(node){
		Runtime_markRecursive(runtime, node->value);
		node = node->next;
	}
	return NULL;
}


Object *Queue_collect(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	Internal *internal = getRaw(self);
	if(!internal){
		return NULL;
	}
	Node *node = internal->head;
	while(node){
		Node *next = node->next;
		free(node);
		node = next;
	}
	free(internal);
	return NULL;
}

