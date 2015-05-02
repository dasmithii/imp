#ifndef IMP_C_H_
#define IMP_C_H_
#include "object.h"
#include "runtime.h"

typedef Object *(*CFunction)(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv);

void Object_setCData(Object *self, void *data);
void *Object_getCData(Object *self);

void Object_registerCMethod(Object *self
	                     , char *name
	                     , CFunction f);

CFunction Object_getCMethod(Object *self, char *name);

CFunction toCFunction(void *addr);

#endif