#ifndef _BASELINE_VECTOR_
#define _BASELINE_VECTOR_
#include "array.h"

// Stores a contiguous, dynamically-sized array of elements. 
typedef struct {
	Array buffer;
	size_t size;
} Vector;

// Initializer & cleaner.
int Vector_init(Vector *const, const size_t);
void Vector_clean(Vector *const);


int Vector_append(Vector *const, const void *const);
int Vector_prepend(Vector*const, const void *const);
int Vector_insert(Vector *const, const unsigned, const void *const);
void Vector_remove(Vector *const, const int);
Vector *Vector_clone(const Vector *const);
void Vector_popf(Vector *const);
void Vector_pop(Vector *const);
void Vector_clear(Vector *const);
void Vector_set(Vector *const, const unsigned, const void *const);
void Vector_each(Vector *const, void (*)(void*));
void Vector_reverse(Vector *const);
void Vector_sort(Vector *const, int (*)(const void*, const void*));
void Vector_fetch(const Vector *const, const unsigned, void *const);
void Vector_fetchLast(const Vector *const, void *const);
void Vector_fetchHead(const Vector *const, void *const);
void *Vector_hook(const Vector *const, const unsigned);
void *Vector_hookLast(const Vector *const);
void *Vector_hookHead(const Vector *const);
int Vector_setCapacity(Vector *const, const size_t);
int Vector_reserve(Vector *const, const size_t);
int Vector_expand(Vector *const);
int Vector_contract(Vector *const);


void *Vector_toBuffer(const Vector *self);


#endif