#include "print.h"
#include "general.h"


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
		switch(BuiltIn_protoId(argv[i])){
		case BUILTIN_STRING:
			printf("%s", (char*) Object_getDataDeep(argv[i], "__data"));
			break;
		case BUILTIN_NUMBER:
			printf("%f", *((double*) Object_getDataDeep(argv[i], "__data")));
			break;
		default:
			// TODO
			break;
		}
		if(argc > 1 && i < argc - 1){
			printf(" ");
		}
	}
	return NULL;
}

void ImpPrinter_init(Object *self){
	BuiltIn_setId(self, BUILTIN_PRINTER);
	Object_registerCMethod(self, "__activate", ImpPrinter_activate_internal);
}
