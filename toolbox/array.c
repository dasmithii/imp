#include "array.h"
#include <string.h>
#include "debug.h"
#include "memory.h"

// --------------------------------------------------------------- //
int Array_init(Array *const arr
	            , const size_t num_elems
	            , const size_t elem_size){
	assume_ptr(arr);
	assume_m(elem_size > 0, "Non-positive Array element size.");
	arr->num_elems = num_elems;
	arr->elem_size = elem_size;
	arr->data = calloc(num_elems, elem_size);
	return arr->data? 0:1;
}

// --------------------------------------------------------------- //
void Array_clean(Array *const arr){
	assume_ptr(arr);
	if(arr->data)
		free(arr->data);
}

// --------------------------------------------------------------- //
Array *Array_new(const size_t num_elems
	                 , const size_t elem_size){
	Array *ret = malloc(sizeof(Array));
	if(!ret)
		return NULL;
	Array_init(ret, num_elems, elem_size);
	return ret;
}

// --------------------------------------------------------------- //
void Array_free(Array *const arr){
	assume_ptr(arr);
	Array_clean(arr);
	free(arr);
}

// --------------------------------------------------------------- //
void *Array_hook(const Array *const arr
	              , const size_t i){
	assume_ptr(arr);
	assume_m(i < arr->num_elems, "Index out of bounds.");
	return arr->data + arr->elem_size * i;
}

// --------------------------------------------------------------- //
void Array_set(Array *const arr
	            , const size_t i
	            , const void *const ptr){
	assume_ptrs(arr, ptr);
	assume_m(i < arr->num_elems, "Index out of bounds.");
	memmove(Array_hook(arr, i), ptr, arr->elem_size);
}

// --------------------------------------------------------------- //
void Array_fetch(const Array *const arr
	              , const size_t i
	              , void *const dest){
	assume_ptrs(arr, dest);
	memmove(dest, Array_hook(arr, i), arr->elem_size);
}

// --------------------------------------------------------------- //
void Array_zero(Array *const arr){
	assume_ptr(arr);
	memset(arr->data, 0, arr->elem_size * arr->num_elems);
}

// --------------------------------------------------------------- //
void Array_fill(Array *const arr
	              , const void *const val){
	assume_ptrs(arr, val);
	for(int i = 0; i < arr->num_elems; ++i)
		Array_set(arr, i, val);
}

// --------------------------------------------------------------- //
void Array_reverse(Array *const arr){
	assume_ptr(arr);
	for(int i = 0; i < arr->num_elems / 2; ++i){
		void *const p1 = Array_hook(arr, i);
		void *const p2 = Array_hook(arr, arr->num_elems - 1 - i);
		swap(p1, p2, arr->elem_size);
	}
}

// --------------------------------------------------------------- //
void Array_sort(Array *const arr
	             , int (*cmp)(const void*, const void*)){
	assume_ptrs(arr, cmp);
	qsort(arr->data, arr->num_elems, arr->elem_size, cmp);
}

// --------------------------------------------------------------- //
void Array_each(Array *const arr
	             , void (*func)(void*)){
	for(int i = 0; i < arr->num_elems; ++i)
		func(Array_hook(arr, i));
}

// --------------------------------------------------------------- //
int Array_copy(const Array *const src
	            , Array *const dest){
	assume_ptrs(src, dest);
	dest->num_elems = src->num_elems;
	dest->elem_size = src->elem_size;
	dest->data = malloc(src->elem_size * src->num_elems);
	if(!dest->data)
		return 1;
	memcpy(dest->data, src->data, src->elem_size * src->num_elems);
	return 0;
}

// --------------------------------------------------------------- //
Array *Array_clone(const Array *const arr){
	Array *ret = malloc(sizeof(Array));
	if(!ret)
		return NULL;
	int rc = Array_copy(arr, ret);
	if(rc){
		free(ret);
		return NULL;
	}
	return ret;
}

// --------------------------------------------------------------- //
size_t Array_innerSize(const Array *const arr){
	return arr->elem_size * arr->num_elems;
}

// --------------------------------------------------------------- //
int Array_setCapacity(Array *const arr, const size_t elems)
{
	void *np = realloc(arr->data, arr->elem_size * elems);
	if(!np)
		return -1;
	arr->data = np;
	arr->num_elems = elems;
	return 0;
}


