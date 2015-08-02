#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <imp/c.h>


void iObject_registerCActivator(iObject *object, CFunction f){
	void *fp = malloc(sizeof(CFunction));
	if(!fp){
		abort();
	}
	memcpy(fp, &f, sizeof(CFunction));
	iObject_putDataShallow(object, "__activate", fp);
}


static inline void iRuntime_registerCMethod_(iRuntime *runtime
	                                       , iObject *object
                                           , char *methodName
                                           , CFunction method
                                           , bool privileged){
	assert(runtime);
	assert(iObject_isValid(object));

	if(iObject_hasKeyShallow(object, methodName)){
		iRuntime_throwFormatted(runtime, object, "registerCMethod failed: slot '%s' already exists (would overwrite)", methodName);
	}

	iObject *methodiObject = iRuntime_MAKE(runtime, Object);
	iObject_putShallow(object, methodName, methodiObject);
	if(privileged){
		iObject_putDataShallow(methodiObject, "__privilege", NULL);
	}

	iObject_registerCActivator(methodiObject, method);
}


void iRuntime_registerCMethod(iRuntime *runtime
	                        , iObject *object
                            , char *methodName
                            , CFunction method){
	iRuntime_registerCMethod_(runtime
		                    , object
		                    , methodName
		                    , method
		                    , false);
}


void iRuntime_registerPrivelegedCMethod(iRuntime *runtime
	                                  , iObject *object
                                      , char *methodName
                                      , CFunction method){
	iRuntime_registerCMethod_(runtime
		                    , object
		                    , methodName
		                    , method
		                    , true);
}
