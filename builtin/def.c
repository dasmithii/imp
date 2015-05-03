#include "def.h"
#include "../object.h"
#include "general.h"
#include "../c.h"
#include "atom.h"

static Object *ImpDef_activate_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	// TODO: check arguments (should only be two, and first
	// should be an atom).
	char *field = ImpAtom_getRaw(argv[0]);
	Object_putShallow(context, field, argv[1]);
	return argv[1];
}

void ImpDef_init(Object *self){
	BuiltIn_setId(self, BUILTIN_DEFINER);
	Object_registerCMethod(self, "__activate", ImpDef_activate_internal);
}