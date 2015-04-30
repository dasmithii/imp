#ifndef IMP_ALLOCATOR_H_
#define IMP_ALLOCATOR_H_
#include <pthread.h>
#include "toolbox/vector.h"
#include "object.h"



// The allocator manages Imp objects via malloc and
// mark & sweep garbage collection. Note that it is
// not thread-safe on its own. Runtime must be halted
// externally, at least for now.
typedef struct {
	Vector object_pointers;
} Allocator;

void Allocator_init(Allocator *self);
void Allocator_clean(Allocator *self);

Object *Allocator_malloc(Allocator *self);
void Allocator_collect(Allocator *self);

#endif