#include "set.h"
#include "../object.h"
#include "general.h"
#include "../c.h"
#include "route.h"

static Object *ImpSet_activate_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	// TODO: check arguments (should only be two, and first
	// should be an atom).
	char *field = ImpRoute_getRaw(argv[0]);
	Object_putDeep(context, field, argv[1]);
	return NULL;
}

void ImpSet_init(Object *self){
	BuiltIn_setId(self, BUILTIN_SETTER);
	Object_registerCMethod(self, "__activate", ImpSet_activate_internal);
}