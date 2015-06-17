#ifndef IMP_C_H_
#define IMP_C_H_

#include <imp/object.h>
#include <imp/runtime.h>




typedef Object *(*CFunction)(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv);


void Runtime_registerCMethod(Runtime *runtime
	                       , Object *object
                           , char *methodName
                           , CFunction method);
 

// Privileged methods, when called, are passed raw argument
// argument lists, which may contain variables pre-
// dereferencing. In contrast, regular (non-priveleged)
// methods do not have such access. Instead, they are passed
// values only (not variables/slots).
void Runtime_registerPrivelegedCMethod(Runtime *runtime
	                                 , Object *object
                                     , char *methodName
                                     , CFunction method);

void Object_registerCActivator(Object *object, CFunction method);

#endif