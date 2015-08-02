#ifndef IMP_C_H_
#define IMP_C_H_

#include <imp/object.h>
#include <imp/runtime.h>




typedef iObject *(*CFunction)(iRuntime *runtime
	                        , iObject *context
	                        , iObject *caller
	                        , int argc
	                        , iObject **argv);


void iRuntime_registerCMethod(iRuntime *runtime
	                        , iObject *object
                            , char *methodName
                            , CFunction method);
 

// Privileged methods, when called, are passed raw argument
// argument lists, which may contain variables pre-
// dereferencing. In contrast, regular (non-priveleged)
// methods do not have such access. Instead, they are passed
// values only (not variables/slots).
void iRuntime_registerPrivelegedCMethod(iRuntime *runtime
	                                  , iObject *object
                                      , char *methodName
                                      , CFunction method);


void iObject_registerCActivator(iObject *object, CFunction method);




#endif