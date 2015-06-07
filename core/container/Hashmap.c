// This file implements a hashmap object for the imp
// standard library. 
//
// Our prototypical method of collision resistance is
// Separate Chaining (using linked lists in hash
// table buckets).
//
// Resizing is done in full (i.e. not incrementally).

#include <stdlib.h>
#include <stdbool.h>

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>


// Hashmap bucket arrays are always >= the minimum
// bucket count. They are initialized as such and 
// never shrink below their initial size.
#define MIN_BUCKET_COUNT 8

// If we are inserting a new element, the bucket array
// is above its minimum size, and our load is above
// the following value, the entire bucket array is
// re-created. It is lengthened and elements are
// re-inserted. This is an expensive operation. 
#define MAX_LOAD .70

// When many elements are removed from a hashmap, and
// the ratio of elements per bucket drops below the 
// following value, we reallocate the bucket array to
// a smaller size and re-insert each hashmap element.
// Again, this is an expensive operation.
#define MIN_LOAD .1


typedef struct Node {
	Object *key;
	Object *value;
	struct Node *next;
} Node;


static bool Node_hasKey(Node *node, Object *key, Runtime *runtime){
	Object *n = Runtime_callMethod(runtime
		                         , NULL
		                         , key
		                         , "=="
		                         , 1
		                         , &node->key);
	return ImpNumber_getRaw(n) != 0;
}


typedef struct {
	Node *head;
} Bucket;


static Node *Bucket_hookNode(Bucket *bucket, Object *key, Runtime *runtime){
	Node *node = bucket->head;

	while(node){
		if(Node_hasKey(node, key, runtime)){
			return node;
		}
		node = node->next;
	}
	return NULL;
}


static void Bucket_insert(Bucket *bucket
	                    , Object *key
	                    , Object *value
	                    , Runtime *runtime){
	Node *existing = Bucket_hookNode(bucket, key, runtime);
	if(existing){
		existing->value = value;
		return;
	}

	Node *newNode = malloc(sizeof(newNode));
	if(!newNode){
		abort();
	}
	newNode->key = key;
	newNode->value = value;
	newNode->next = bucket->head;
	bucket->head = newNode;
}


static void Bucket_remove(Bucket *bucket, Object *key, Runtime *runtime){
	Node *existing = Bucket_hookNode(bucket, key, runtime);
	if(existing){
		if(existing == bucket->head){
			bucket->head = existing->next;
		} else {
			Node *pre = bucket->head;
			while(pre->next != existing){
				pre = pre->next;
			}
			pre->next = existing->next;
		}
		free(existing);
	}
}


static void Bucket_clean(Bucket *bucket){
	Node *node = bucket->head;
	while(node){
		Node *next = node->next;
		free(node);
		node = next;
	}
	bucket->head = NULL;
}



typedef struct {
	size_t elemC; // element count
	size_t buckC; // bucket count
	Bucket *bucks;// bucket array
} Map;


static size_t hashCode(Object *key, Runtime *runtime){
	Object *n = Runtime_callMethod(runtime, NULL, key, "hashCode", 0, NULL);
	if(BuiltIn_id(n) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "hashCode did not return number");
	}
	return (size_t) ImpNumber_getRaw(n);
}


static Bucket *Map_bucketFor(Map *map, Object *key, Runtime *runtime){
	return map->bucks + (hashCode(key, runtime) % map->buckC);
}


static Map *getRaw(Object *self){
	return Object_getDataDeep(self, "__data");
}


static void Map_setBuckC(Map *map, size_t c, Runtime *runtime){
	// make new bucks array
	Bucket *bucksP = calloc(c, sizeof(Bucket)); // bucks'
	if(!bucksP){
		abort();
	}
	for(int i = 0; i < map->buckC; i++){
		Node *node = map->bucks[i].head;
		while(node){
			size_t hc = hashCode(node->key, runtime);
			Bucket_insert(bucksP + (hc % c), node->key, node->value, runtime);
			node = node->next;
		}
	}

	// free old one
	for(int i = 0; i < map->buckC; i++){
		Bucket_clean(map->bucks + i);
	}
	if(map->bucks){
		free(map->bucks);
	}

	// save new one in its place
	map->buckC = c;
	map->bucks = bucksP;
}


Object *Hashmap_put(Runtime *runtime
	              , Object *context
	              , Object *self
	              , int argc
	              , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "Hashmap:put requires two arguments");
	}

	Map *const map = getRaw(self);

	const double load = (double) map->elemC / (double) map->buckC;
	if(load >= MAX_LOAD){
		Map_setBuckC(map, 2 * map->buckC, runtime);
	}

	Bucket_insert(Map_bucketFor(map, argv[0], runtime)
		        , argv[0]
		        , argv[1]
		        , runtime);
	return NULL;
}

Object *Hashmap_get(Runtime *runtime
	              , Object *context
	              , Object *self
	              , int argc
	              , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Hashmap:get requires exactly one argument");
	}

	Map *const map = getRaw(self);

	Node *existing = Bucket_hookNode(Map_bucketFor(map, argv[0], runtime)
		                           , argv[0]
		                           , runtime);
	if(existing){
		return existing->value;
	}
	return NULL;
}

Object *Hashmap_remove(Runtime *runtime
	                 , Object *context
	                 , Object *self
	                 , int argc
	                 , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Hashmap:remove requires exactly one argument");
	}

	Map *const map = getRaw(self);

	const double load = (double) map->elemC / (double) map->buckC;
	if(load <= MIN_LOAD && map->buckC > MIN_BUCKET_COUNT){
		size_t targetSize = MIN_LOAD * map->elemC;
		if(targetSize < MIN_BUCKET_COUNT){
			targetSize = MIN_BUCKET_COUNT;
		}
		Map_setBuckC(map, targetSize, runtime);
	}
	Bucket_remove(Map_bucketFor(map, argv[0], runtime)
		        , argv[0]
		        , runtime);
	return NULL;
}

Object *Hashmap_mark(Runtime *runtime
	               , Object *context
	               , Object *self
	               , int argc
	               , Object **argv){
	Map *const map = getRaw(self);
	if(!map){
		return NULL;
	}

	for(int i = 0; i < map->buckC; i++){
		Node *node = map->bucks[i].head;
		while(node){
			Runtime_markRecursive(runtime, node->key);
			Runtime_markRecursive(runtime, node->value);
			node = node->next;
		}
	}
	return NULL;
}


Object *Hashmap_collect(Runtime *runtime
	                  , Object *context
	                  , Object *self
	                  , int argc
	                  , Object **argv){
	Map *const map = getRaw(self);
	for(int i = 0; i < map->buckC; i++){
		Bucket_clean(map->bucks + i);
	}
	if(map->bucks){
		free(map->bucks);
	}
	free(map);
	return NULL;
}


Object *Hashmap_clone(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", self);
	Map *internal = malloc(sizeof(Map));
	if(!internal){
		abort();
	}
	internal->elemC = 0;
	internal->buckC = MIN_BUCKET_COUNT;
	internal->bucks = calloc(MIN_BUCKET_COUNT, sizeof(Bucket));
	if(!internal->bucks){
		abort();
	}
	Object_putDataShallow(r, "__data", internal);
	return r;
}
