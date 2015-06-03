#include <assert.h>
#include <string.h>

#include <imp/c.h>




void Object_registerCMethod(Object *self
	                      , char *name
	                      , CFunction f){
	assert(self);
	assert(name);
	assert(strstr(name, "__") == name);

	void *pointer = malloc(sizeof(CFunction));
	if(!pointer){
		abort();
	}
	memcpy(pointer, &f, sizeof(CFunction));
	Object_putDataShallow(self, name, pointer);
}
