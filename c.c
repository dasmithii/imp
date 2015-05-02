#include "c.h"
#include <string.h>

void Object_setCData(Object *self, void *data){
	Object_putDataDeep(self, "__cdata", data);
}


void *Object_getCData(Object *self){
	return Object_getDataDeep(self, "__cdata");
}



void Object_registerCMethod(Object *self
	                      , char *name
	                      , CFunction f){
	// TODO:  check that <name> is prefixed with '__'
	void *pointer = malloc(sizeof(CFunction));
	memcpy(pointer, &f, sizeof(CFunction));
	Object_putDataShallow(self, name, pointer);
}


CFunction Object_getCMethod(Object *self, char *name){
	// TODO:  check that <name> is prefixed with '__'
	CFunction *r = Object_getDataDeep(self, name);
	return *r;
}
