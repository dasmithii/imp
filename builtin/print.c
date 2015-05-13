#include "print.h"
#include "general.h"
#include "route.h"
#include <stdio.h>
#include "boolean.h"

static void print(Runtime *runtime
	            , Object *context
	            , Object *object){
	if(!object){
		printf("NULL");
		return;
	}

	switch(BuiltIn_protoId(object)){
	case BUILTIN_ROUTE:
		print(runtime, context, Object_getDeep(context, ImpRoute_getRaw(object)));
		break;
	case BUILTIN_STRING:
		printf("%s", (char*) Object_getDataDeep(object, "__data"));
		break;
	case BUILTIN_NUMBER:
		printf("%f", *((double*) Object_getDataDeep(object, "__data")));
		break;
	default:
		// TODO
		break;
	}
}


static Object *ImpPrinter_activate_internal(Runtime *runtime
	                             , Object *context
	                             , Object *caller
	                             , int argc
	                             , Object **argv){
	// TODO: search for specially-defined tostring method
	// Object *special = Object_getDeep(subs[0], "_toString");
	// if(special){
	// }
	for(int i = 0; i < argc; i++){
		print(runtime, context, argv[i]);
		if(argc > 1 && i < argc - 1){
			printf(" ");
		}
	}
	printf("\n");
	return NULL;
}

void ImpPrinter_init(Object *self){
	BuiltIn_setId(self, BUILTIN_PRINTER);
	Object_registerCMethod(self, "__activate", ImpPrinter_activate_internal);
}
