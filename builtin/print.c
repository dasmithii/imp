#include <stdio.h>

#include "print.h"
#include "general.h"
#include "route.h"
#include "boolean.h"




bool ImpPrinter_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_PRINTER;
}


static Object *ImpPrinter_activate_internal(Runtime *runtime
	                             , Object *context
	                             , Object *caller
	                             , int argc
	                             , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpPrinter_isValid(caller));

	// TODO: search for specially-defined tostring method
	// Object *special = Object_getDeep(subs[0], "_toString");
	// if(special){
	// }
	for(int i = 0; i < argc; i++){
		Object *obj = argv[i];
		if(BuiltIn_id(obj) == BUILTIN_ROUTE){
			obj = ImpRoute_mapping(obj, context);
			if(!obj){
				Runtime_throwString(runtime, "variable does not exist.");
			}
		}

		if(Object_hasKeyShallow(obj, "__id")){
			Object_print(obj);
		} else {
			Runtime_print(runtime, context, obj);
		}
		if(argc > 1 && i < argc - 1){
			printf(" ");
		}
	}
	printf("\n");
	return NULL;
}


void ImpPrinter_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_PRINTER);
	Object_registerCMethod(self, "__activate", ImpPrinter_activate_internal);
}
