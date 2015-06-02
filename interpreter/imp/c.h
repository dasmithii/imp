#ifndef IMP_C_H_
#define IMP_C_H_

#include <imp/object.h>
#include <imp/runtime.h>




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
void Object_registerCMethodAlias(Object *self
	                     , char *name
	                     , CFunction f
	                     , Runtime *runtime);
CFunction toCFunction(void *addr);




#endif