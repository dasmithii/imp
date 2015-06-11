#include "break.h"
#include "../runtime.h"
#include "general.h"

static Object *ImpBreak_activate_(Runtime *runtime
	                    , Object *context
	                    , Object *self
	                    , int argc
	                    , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "break does not accept arguments");
	}
	Runtime_setReturnValue(runtime, self);
	return NULL;
}


void ImpBreak_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_BREAK);
	Object_registerCMethod(self, "__activate", ImpBreak_activate_);
}