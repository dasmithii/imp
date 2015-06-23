#include "vector.h"
#include "debug.h"
#include <stdlib.h>
#include <string.h>
#define DEFAULT_CAPACITY 25

// --------------------------------------------------------------- //
int Vector_init(Vector *const vec, const size_t elem_size)
{
	assume_ptr(vec);
	assume_m(elem_size > 0, "Attempted to initialize Vector with elem_size 0.");

	// attempt inits
	vec->size = 0;
	int rc = Array_init(&vec->buffer, DEFAULT_CAPACITY, elem_size);

	// check for allocation error
	if(rc)
		return -1;
	return 0;
}

// --------------------------------------------------------------- //
void Vector_clean(Vector *const vec)
{
	assume_ptr(vec);
	Array_clean(&vec->buffer);
	vec->size = 0;
}

// --------------------------------------------------------------- //
int Vector_append(Vector *const vec, const void *const ptr)
{
	assume_ptrs(vec, ptr);

	// check capacity
	if(vec->size == vec->buffer.num_elems){
		int rc = Vector_expand(vec);
		if(rc)
			return -1;
	}

	// insert element
	Vector_set(vec, vec->size, ptr);
	vec->size++;
	return 0;
}

// --------------------------------------------------------------- //
int Vector_prepend(Vector *const vec, const void *const ptr)
{
	return Vector_insert(vec, 0, ptr);
}

// --------------------------------------------------------------- //
void Vector_fetch(const Vector *const vec, const unsigned i, void *const to)
{
	assume_ptrs(vec, to);
	assume_ceil(i, vec->size);

	void *ptr = Vector_hook(vec, i);
	memcpy(to, ptr, vec->buffer.elem_size);
}

// --------------------------------------------------------------- //
void Vector_fetchLast(const Vector *const vec, void *const to)
{
	assume_pos(vec->size);
	void *ptr = Vector_hookLast(vec);
	memcpy(to, ptr, vec->buffer.elem_size);
}

// --------------------------------------------------------------- //
void Vector_fetchHead(const Vector *const vec, void *const to)
{
	assume_ptrs(vec, to);
	assume_pos(vec->size);
	memcpy(to, vec->buffer.data, vec->buffer.elem_size);
}

// --------------------------------------------------------------- //
void *Vector_hook(const Vector *const vec, const unsigned i)
{
	assume_ptr(vec);
	assume_max(i, vec->size);
	return Array_hook(&vec->buffer, i);
}

// --------------------------------------------------------------- //
int Vector_insert(Vector *const vec, const unsigned i, const void *const ptr)
{
	assume_ptrs(vec, ptr);
	assume_max(i, vec->size + 1);

	// ensure capacity
	if(vec->size == vec->buffer.num_elems){
		int rc = Vector_expand(vec);
		if(rc)
			return -1;
	}

	// shift elements
	char *spot = Vector_hook(vec, i);
	size_t size = (vec->size - i) * vec->buffer.elem_size;
	memmove(spot + vec->buffer.elem_size, spot, size);
	memcpy(spot, ptr, vec->buffer.elem_size);
	vec->size++;
	return 0;
}

// --------------------------------------------------------------- //
void Vector_remove(Vector *const vec, const int i)
{
	assume_ptr(vec);
	assume_ceil(i, vec->size);

	void *elem = Vector_hook(vec, i);
	void *next = elem + vec->buffer.elem_size;
	size_t blocks = vec->size - i - 1;
	size_t bytes  = blocks * vec->buffer.elem_size;
	memmove(elem, next, bytes);
	vec->size--;
}

// --------------------------------------------------------------- //
void Vector_reverse(Vector *const vec)
{
	assume_ptr(vec);

	// hook pivot point
	int piv = vec->size / 2;

	// swap blocks
	char *lb = vec->buffer.data;
	for(int i = 0; i < piv; ++i){
		char *rb = Vector_hook(vec, vec->size - i - 1);
		for(int b = 0; b < vec->buffer.elem_size; ++b){
			char tmp = *lb;
			*lb = *rb;
			*rb = tmp;
			++rb;
			++lb;
		}
	}

}

// --------------------------------------------------------------- //
Vector *Vector_clone(const Vector *const vec)
{
	assume_ptr(vec);

	// allocate new Vector
	Vector *ret = malloc(sizeof(Vector));
	if(!ret)
		return NULL;

	// allocate & copy internal memory
	int rc = Array_copy(&vec->buffer, &ret->buffer);
	if(rc){
		free(ret);
		return NULL;
	}

	ret->size = vec->size;
	return ret;
}

// --------------------------------------------------------------- //
void Vector_popf(Vector *const vec)
{
	assume_ptr(vec);
	assume_pos(vec->size);

	// shift elements
	for(int i = 0; i < vec->size - 1; i++){
		void *to = Vector_hook(vec, i);
		Vector_fetch(vec, i + 1, to);
	}
}

// --------------------------------------------------------------- //
void Vector_pop(Vector *const vec)
{
	assume_ptr(vec);
	assume_pos(vec->size);
	vec->size--;
}

// --------------------------------------------------------------- //
void Vector_clear(Vector *const vec)
{
	assume_ptr(vec);
	vec->size = 0;
	Vector_setCapacity(vec, DEFAULT_CAPACITY);
}

// --------------------------------------------------------------- //
void Vector_each(Vector *const vec, void (*func)(void*))
{
	assume_ptrs(vec, func);
	char *elem = vec->buffer.data;
	for(int i = 0; i < vec->size; ++i){
		func((void*) elem);
		elem += vec->buffer.elem_size;
	}
}

// --------------------------------------------------------------- //
int Vector_setCapacity(Vector *const vec, const size_t n)
{
	assume_ptr(vec);
	assume_pos(n);
	assume_m(n >= vec->size, "Capacity below size.");
	return Array_setCapacity(&vec->buffer, n);
}

// --------------------------------------------------------------- //
int Vector_reserve(Vector * const vec, const size_t n)
{
	assume_ptr(vec);
	assume_pos(n);
	if(vec->buffer.num_elems < n)
		return Vector_setCapacity(vec, n);
	return 0;
}

// --------------------------------------------------------------- //
void Vector_set(Vector *const vec, const unsigned i, const void *const elem)
{
	assume_max(i, vec->size);
	void *dest = Vector_hook(vec, i);
	if(elem)
		memcpy(dest, elem, vec->buffer.elem_size);
	else
		memset(dest, 0, vec->buffer.elem_size);
}

// --------------------------------------------------------------- //
int Vector_expand(Vector *const vec)
{
	assume_ptr(vec);
	int cap = vec->buffer.num_elems * 2;
	return Vector_setCapacity(vec, cap);
}

// --------------------------------------------------------------- //
int Vector_contract(Vector *const vec)
{
	assume_ptr(vec);
	return Vector_setCapacity(vec, vec->size);
}

// --------------------------------------------------------------- //
void *Vector_hookLast(const Vector *const vec)
{
	if(vec->size)
		return Vector_hook(vec, vec->size - 1);
	return NULL;
}

// --------------------------------------------------------------- //
void *Vector_hookHead(const Vector *const vec)
{
	return vec->buffer.data;
}

// --------------------------------------------------------------- //
void Vector_sort(Vector *const vec, int (*cmp)(const void*, const void*))
{
	Array_sort(&vec->buffer, cmp);
}


void *Vector_toBuffer(const Vector *self)
{
	size_t size = self->size * self->buffer.elem_size;
	void *ret = malloc(size);
	if(!ret)
		return NULL;
	memcpy(ret, self->buffer.data, size);
	return ret;
}