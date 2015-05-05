#include "c.h"
#include <string.h>
#include <assert.h>

void Object_setCData(Object *self, void *data){
	assert(self);
	Object_putDataDeep(self, "__cdata", data);
}


void *Object_getCData(Object *self){
	assert(self);
	return Object_getDataDeep(self, "__cdata");
}



void Object_registerCMethod(Object *self
	                      , char *name
	                      , CFunction f){
	assert(self);
	assert(name);
	assert(strstr(name, "__") == name);

	void *pointer = malloc(sizeof(CFunction));
	memcpy(pointer, &f, sizeof(CFunction));
	Object_putDataShallow(self, name, pointer);
}


CFunction Object_getCMethod(Object *self, char *name){
	assert(self);
	assert(name);
	assert(strstr(name, "__") == name);

	// TODO:  check that <name> is prefixed with '__'
	CFunction *r = Object_getDataDeep(self, name);
	return *r;
}
