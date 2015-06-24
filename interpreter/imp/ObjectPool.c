#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ObjectPool.h"
#include "runtime.h"




typedef struct FreeList {
	Object *block;
	size_t blockSize;
	struct FreeList *nextNode;
} FreeList;


static inline bool FreeList_hasSpace(FreeList *self){
	return self->blockSize? true:false;
}


static inline void FreeList_clean(FreeList *self){
	self = self->nextNode;
	while(self){
		FreeList *next = self->nextNode;
		free(self);
		self = next;
	}
}


static inline Object *FreeList_allocateObject(FreeList *self){
	assert(FreeList_hasSpace(self));
	Object *r = self->block;
	self->block++;
	self->blockSize--;
	FreeList *nextNode = self->nextNode;
	if(self->blockSize == 0 && self->nextNode){
		*self = *nextNode;
		free(nextNode);
	}
	Object_init(r);
	return r;
}


// A crate is a container capable of storing a 
// static number of objects. At any moment, it has 
// an arbitrary arrangement of objects within. And the
// goal is, without moving the objects inside, to 
// insert new objects until the crate is full.
// 
// Objects are never removed from the crate manually
// (garbage collection handles that). And so, insertion
// can be handled by keeping an array of open sections
// within the crate, which are filled sequentially.
typedef struct Crate {
	Object *slots;
	FreeList freeSlots;
	struct Crate *successor;
	size_t capacity;
} Crate;


static inline Crate Crate_withCapacity(size_t n){
	assert(n > 0);
	Crate r;
	r.slots = malloc(n * sizeof(Object));
	if(!r.slots){
		abort();
	}
	r.freeSlots.block = r.slots;
	r.freeSlots.blockSize = n;
	r.freeSlots.nextNode = NULL;
	r.successor = NULL;
	r.capacity = n;
	return r;
}


static inline Crate *Crate_newWithCapacity(size_t n){
	Crate *r = malloc(sizeof(Crate));
	if(!r){
		abort();
	}
	*r = Crate_withCapacity(n);
	return r;
}


static inline bool Crate_hasSpace(Crate *self){
	return FreeList_hasSpace(&self->freeSlots);
}


static inline bool Crate_isEmpty(Crate *self){
	return self->freeSlots.blockSize == self->capacity;
}


static inline Object *Crate_allocateObject(Crate *self){
	assert(Crate_hasSpace(self));
	return FreeList_allocateObject(&self->freeSlots);
}


static inline void Crate_unmarkObjects(Crate *self){
	for(int i = 0; i < self->capacity; i++){
		Object_unmark(self->slots + i);
	}
}


static inline void Crate_markReferencedObjects(Crate *self, Runtime *runtime){
	FreeList *frees = &self->freeSlots;
	Object *object = self->slots;
	while(object < self->slots + self->capacity){
		if(frees && frees->block == object){
			printf("no rec\n");
			object += frees->blockSize;
			frees = frees->nextNode;
		} else {
			printf("rec\n");
			Runtime_markRecursive(runtime, object);
			object++;
		}
	}
}


static inline void Crate_clean(Crate *self){
	FreeList_clean(&self->freeSlots);
	free(self->slots);
}


static inline void Crate_free(Crate *self){
	Crate_clean(self);
	free(self);
}


static inline void Crate_squeeze(Crate *self){
	Crate_clean(self);
	*self = *self->successor;
	if(self->successor){
		Crate *squeezed = self->successor;
		self->successor = self->successor->successor;
		free(squeezed);
	}
}


// returns # of objects cleaned
static inline int Crate_cleanUnmarkedObjects(Crate *self, Runtime *runtime){
	printf("CLEANING...\n");
	int n = 0;

	FreeList *frees = &self->freeSlots;
	Object *object = self->slots;
	while(object < self->slots + self->capacity){
		if(frees && frees->block == object){
			object = object + frees->blockSize;
			frees = frees->nextNode;
		} else {
			if(!object->gc_mark){
				Runtime_collectObject(runtime, object);
				n++;
				frees->blockSize++;
				FreeList *toSqueeze = frees->nextNode;
				if(toSqueeze && frees->block + frees->blockSize == toSqueeze->block){
					frees->blockSize += toSqueeze->blockSize;
					frees->nextNode = toSqueeze->nextNode;
					free(toSqueeze);
				}
			}
			object++;
		}
	}
	return n;
}


// Imp's memory manager is both a memory-pooled 
// allocator of Imp objects and a garbage collector 
// (mark & sweep, full pause).
//
// The memory pool is implemented as a series of
// "crates" (explained above). These are filled 
// sequentially, and when openings do not exist, new,
// exponentially bigger crates are malloc'ed. Subsequent
// to each round of garbage collection, empty crates are
// freed.
//
// All in all, this results in best case O(1) 
// allocation, with quite the small 1. Worst case depends
// on malloc calls, which aren't especially bad or 
// frequent (because of exponentially large crates). Thus
// the system is pretty good
typedef struct ImpObjectPool_ {
	Crate first;  // first node in forward-linked list of memory blocks
	Crate *next;  // first node with unused space in its block
	bool gcRunning;
	size_t gcLocks;
	size_t objectCount;
	Runtime *runtime;
} ImpObjectPool_;


ImpObjectPool ImpObjectPool_forRuntime(void *runtime){
	ImpObjectPool_ *r = malloc(sizeof(ImpObjectPool_));
	if(!r){
		abort();
	}
	r->first = Crate_withCapacity(100); 
	r->next = &r->first;
	r->gcRunning = false;
	r->gcLocks = 0;
	r->objectCount = 0;
	r->runtime = (Runtime*) runtime;
	return r;
}


static void ImpObjectPool_markAndSweep(ImpObjectPool self){
	printf("MARK AND SWEEEP  !!!!  OC: %d\n", self->objectCount);
	assert(self);
	assert(self->gcLocks == 0);
	assert(self->gcRunning == false);   // avoid running this recursively
	self->gcRunning = true;

	// unmark all objects
	Crate *crate = &self->first;
	while(crate){
		Crate_unmarkObjects(crate);
		crate = crate->successor;
	}

	// mark volatile objects and dependencies
	crate = &self->first;
	while(crate){
		Crate_markReferencedObjects(crate, self->runtime);
		crate = crate->successor;
	}

	// mark other accessible allocations
	Runtime_markRecursive(self->runtime, self->runtime->root_scope);
	if(self->runtime->lastReturnValue){
		Runtime_markRecursive(self->runtime, self->runtime->lastReturnValue);
	}

	// clean unmarked objects, rebuild free lists and remove empty
	// crates
	crate = &self->first;
	while(crate){
		self->objectCount -= Crate_cleanUnmarkedObjects(crate, self->runtime);
		if(crate != &self->first && Crate_isEmpty(crate)){
			Crate_squeeze(crate);
		} else {
			crate = crate->successor;
		}
	}

	self->gcRunning = false;
}


static int n = 0;
Object *ImpObjectPool_allocate(ImpObjectPool self){
	n++;
	printf("ALLOCATION: %d\n", n);
	if(self->gcRunning == false         &&
	   self->gcLocks == 0               &&
	   self->objectCount >= 100         &&
	   self->objectCount %  20  == 0){
		ImpObjectPool_markAndSweep(self);
	}

	if(!Crate_hasSpace(self->next)){
		if(!self->gcRunning && !self->gcLocks){
			ImpObjectPool_markAndSweep(self);
			if(Crate_hasSpace(self->next)){
				goto PRALLO;
			}
		}
		self->next->successor = Crate_newWithCapacity(self->objectCount + 2);
		self->next = self->next->successor;
	}
	PRALLO:
	self->objectCount++;
	return Crate_allocateObject(self->next);
}


void ImpObjectPool_lockGC(ImpObjectPool self){
	assert(self);
	assert(self->gcRunning == false);
	self->gcLocks++;
}


void ImpObjectPool_unlockGC(ImpObjectPool self){
	assert(self);
	assert(self->gcLocks > 0);
	self->gcLocks--;
}


void ImpObjectPool_free(ImpObjectPool pool){
	Crate_clean(&pool->first);
	Crate *node = pool->first.successor;
	while(node){
		Crate *next = node->successor;
		Crate_free(node);
		node = next;
	}
	free(pool);
}

