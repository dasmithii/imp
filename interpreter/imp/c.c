#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <imp/c.h>


void Object_registerCActivator(Object *object, CFunction f){
	void *fp = malloc(sizeof(CFunction));
	if(!fp){
		abort();
	}
	memcpy(fp, &f, sizeof(CFunction));
	Object_putDataShallow(object, "__activate", fp);
}


static inline
void Runtime_registerCMethod_(Runtime *runtime
	                        , Object *object
                            , char *methodName
                            , CFunction method
                            , bool privileged){
	assert(runtime);
	assert(Object_isValid(object));
	assert(methodName && *methodName && *methodName != '_');

	char methodSlotKey[64];
	sprintf(methodSlotKey, "_%s", methodName);
	if(Object_hasKeyShallow(object, methodSlotKey)){
		Runtime_throwFormatted(runtime, "registerCMethod failed: slot '%s' already exists (would overwrite)", methodSlotKey);
	}

	Object *methodObject = Runtime_make(runtime, Object);
	Object_putShallow(object, methodSlotKey, methodObject);
	if(privileged){
		Object_putDataShallow(methodObject, "__privilege", NULL);
	}

	Object_registerCActivator(methodObject, method);
}


void Runtime_registerCMethod(Runtime *runtime
	                       , Object *object
                           , char *methodName
                           , CFunction method){
	Runtime_registerCMethod_(runtime
		                   , object
		                   , methodName
		                   , method
		                   , false);
}


void Runtime_registerPrivelegedCMethod(Runtime *runtime
	                                 , Object *object
                                     , char *methodName
                                     , CFunction method){
	Runtime_registerCMethod_(runtime
		                   , object
		                   , methodName
		                   , method
		                   , true);
}
