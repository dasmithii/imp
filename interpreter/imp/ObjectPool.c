#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "ObjectPool.h"
#include "runtime.h"




typedef struct FreeList {
	iObject *block;
	size_t blockSize;
	struct FreeList *nextNode;
} FreeList;


static inline bool FreeList_hasSpace(FreeList *self){
	return self->block? true:false;
}


static inline void FreeList_clean(FreeList *self){
	self->block = NULL;
	self->blockSize = 0;

	FreeList *it = self->nextNode;

	self->nextNode = NULL;

	while(it){
		FreeList *next = it->nextNode;
		free(it);
		it = next;
	}
}


static inline iObject *FreeList_allocateObject(FreeList *self){
	assert(FreeList_hasSpace(self));
	iObject *r = self->block;
	self->block++;
	self->blockSize--;
	FreeList *nextNode = self->nextNode;
	if(self->blockSize == 0){
		if(self->nextNode){
			*self = *nextNode;
			free(nextNode);
		} else {
			FreeList_clean(self);
		}
	}
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
	iObject *slots;
	FreeList freeSlots;
	struct Crate *successor;
	size_t capacity;
	size_t usage;
} Crate;


static inline Crate Crate_withCapacity(size_t n){
	assert(n > 0);
	Crate r;
	r.slots = malloc(n * sizeof(iObject));
	if(!r.slots){
		abort();
	}
	r.freeSlots.block = r.slots;
	r.freeSlots.blockSize = n;
	r.freeSlots.nextNode = NULL;
	r.successor = NULL;
	r.capacity = n;
	r.usage = 0;
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


static inline iObject *Crate_allocateObject(Crate *self){
	assert(Crate_hasSpace(self));
	self->usage++;
	return FreeList_allocateObject(&self->freeSlots);
}


static inline void Crate_unmarkObjects(Crate *self){
	for(size_t i = 0; i < self->capacity; i++){
		iObject_unmark(self->slots + i);
	}
}


static inline void Crate_markReferencedObjects(Crate *self, iRuntime *runtime){
	FreeList *frees = &self->freeSlots;
	iObject *object = self->slots;
	while(object < self->slots + self->capacity){
		if(frees && frees->block == object){
			object += frees->blockSize;
			frees = frees->nextNode;
		} else {
			if(iObject_referenceCount(object) > 0){
				iRuntime_callMethod(runtime, NULL, object, "_markRecursively", 0, NULL);
			}
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


static void Crate_callUnmarkedObjectCleanupFunctions(Crate *self, iRuntime *runtime){
	FreeList *frees = &self->freeSlots;
	iObject *object = self->slots;
	while(object < self->slots + self->capacity){
		if(frees && frees->block == object){
			object += frees->blockSize;
			frees = frees->nextNode;
		} else {
			if(!object->gc_mark){
				if(iObject_hasMethod(object, "_clean")){
					iRuntime_callMethod(runtime, NULL, object, "_clean", 0, NULL);
				}
			}
			object++;
		}
	}

}


// returns # of objects cleaned
static int Crate_freeUnmarkedObjects(Crate *self, iRuntime *runtime){
	int n = 0;

	FreeList *frees = &self->freeSlots;
	iObject *object = self->slots;
	while(object < self->slots + self->capacity){
		if(frees && frees->block == object){
			object += frees->blockSize;
			frees = frees->nextNode;
		} else {
			if(!object->gc_mark){
				iObject_clean(object);
				n++;
			}
			object++;
		}
	}

	// rebuild free list
	frees = &self->freeSlots;
	FreeList_clean(frees);

	// setup free lists.
	iObject *beg = NULL;
	size_t size = 0;
	for(size_t i = 0; i < self->capacity; i++){
		if(!self->slots[i].gc_mark){
			if(beg){
				size++;
			} else {
				beg = self->slots + i;
				size = 1;
			}
		} else if(beg){
			if(frees->block){
				frees->nextNode = malloc(sizeof(FreeList));
				frees = frees->nextNode;
				frees->nextNode = NULL;
			}
			frees->block = beg;
			frees->blockSize = size;
			frees->nextNode = NULL;
			beg = NULL;
		}
	}
	if(beg){
		if(frees->block){
			frees->nextNode = malloc(sizeof(FreeList));
			frees = frees->nextNode;
			frees->nextNode = NULL;
		}
		frees->block = beg;
		frees->blockSize = size;
		frees->nextNode = NULL;
		beg = NULL;
	}
	assert(n <= self->usage);
	self->usage -= n;
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
typedef struct iObjectPool_ {
	Crate *first;  // first node in forward-linked list of memory blocks
	Crate *next;  // first node with unused space in its block; null if none exist
	Crate *last;
	bool gcRunning;
	size_t gcLocks;
	size_t objectCount;
	iRuntime *runtime;
} iObjectPool_;


iObjectPool iObjectPool_forRuntime(void *runtime){
	iObjectPool_ *r = malloc(sizeof(iObjectPool_));
	if(!r){
		abort();
	}
	r->first = Crate_newWithCapacity(100); 
	r->next = r->first;
	r->last = r->next;
	r->gcRunning = false;
	r->gcLocks = 0;
	r->objectCount = 0;
	r->runtime = (iRuntime*) runtime;
	return r;
}


static void iObjectPool_diagnose(iObjectPool self){
	printf("Object Pool\n");
	printf(" - objects: %zu\n", self->objectCount);
	int n = 0;
	Crate *crate = self->first;
	while(crate){
		n++;
		crate = crate->successor;
	}
	printf(" - crates:  %d\n", n);
	n = 0;
	crate = self->first;
	while(crate){
		n++;
		printf(" - crate %d: {capacity: %zu, usage: %zu, address: %p}\n", n, crate->capacity, crate->usage, crate);
		crate = crate->successor;
	}
}

// markAndSweep collects all unreachable objects, rebuilds
// free lists for each crate, and arranges crates into a 
// linked list headed by full crates and absent of empty
// crates.
static void iObjectPool_markAndSweep(iObjectPool self){
	// printf("MARK AND SWEEEP  !!!!  OC: %d\n", self->objectCount);
	assert(self);
	assert(self->gcLocks == 0);
	assert(self->gcRunning == false);   // avoid running this recursively
	self->gcRunning = true;

	// unmark all objects
	Crate *crate = self->first;
	while(crate){
		Crate_unmarkObjects(crate);
		crate = crate->successor;
	}

	// mark volatile objects and dependencies
	crate = self->first;
	while(crate){
		Crate_markReferencedObjects(crate, self->runtime);
		crate = crate->successor;
	}

	// mark other accessible allocations
	if(self->runtime->lastReturnValue){
		iRuntime_callMethod(self->runtime
			             , NULL
			             , self->runtime->lastReturnValue
			             , "_markRecursively"
			             , 0
			             , NULL);
	}


	// call cleanup functions
	crate = self->first;
	while(crate){
		Crate_callUnmarkedObjectCleanupFunctions(crate, self->runtime);
		crate = crate->successor;
	}

	// delete unmarked objects, rebuild free lists
	crate = self->first;
	while(crate){
		self->objectCount -= Crate_freeUnmarkedObjects(crate, self->runtime);
		crate = crate->successor;
	}


	// arrange crates such that all full crates are at beginning 
	// of list. reset first and next pointers accordingly. Remov
	// empty crates.
	Crate *pit = self->first;   // previous iterator
	Crate *it = pit->successor; // iterator 
	self->next = NULL;
	self->last = NULL;
	while(it){
		Crate *next = it->successor;
		if(Crate_hasSpace(it)){
			// iterate
			pit = it;
			self->next = it;
		} else if(Crate_isEmpty(it)){
			// remove
			pit->successor = it->successor;
			Crate_free(it);
		} else {
			// move to front
			pit->successor = it->successor;
			it->successor = self->first;
			self->first = it;
		}
		it = next;
	}

	// update last pointer
	Crate *p = self->first;
	while(p->successor){
		p = p->successor;
	}
	self->last = p;

	// update next pointer
	it = self->first;
	while(it){
		if(Crate_hasSpace(it)){
			self->next = it;
		}
		it = it->successor;
	}


	self->gcRunning = false;
}


iObject *iObjectPool_allocate(iObjectPool self){
	if(self->gcRunning == false         &&
	   self->gcLocks == 0               &&
	   self->objectCount >= 500         &&
	   self->objectCount %  250  == 0){
		iObjectPool_markAndSweep(self);
	}

	if(!self->next){
		if(!self->gcRunning && !self->gcLocks){
			iObjectPool_markAndSweep(self);
			if(self->next){
				goto PRALLO;
			} 
		}
		self->last->successor = Crate_newWithCapacity(self->objectCount + 2);
		self->last = self->last->successor;
		self->next = self->last;
	}
	PRALLO:
	self->objectCount++;
	iObject *r = Crate_allocateObject(self->next);
	if(!Crate_hasSpace(self->next)){
		self->next = self->next->successor;
	}
	iObject_init(r);
	return r;
}


void iObjectPool_lockGC(iObjectPool self){
	assert(self);
	assert(self->gcRunning == false);
	self->gcLocks++;
}


void iObjectPool_unlockGC(iObjectPool self){
	assert(self);
	assert(self->gcLocks > 0);
	self->gcLocks--;
}


void iObjectPool_free(iObjectPool pool){
	Crate_clean(pool->first);
	Crate *node = pool->first->successor;
	while(node){
		Crate *next = node->successor;
		Crate_free(node);
		node = next;
	}
	free(pool);
}

