#include "break.h"
#include "../runtime.h"
#include "general.h"


static Object *ImpContinue_activate_(Runtime *runtime
	                    , Object *context
	                    , Object *self
	                    , int argc
	                    , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "continue does not accept arguments");
	}
	Runtime_setReturnValue(runtime, self);
	return NULL;
}


void ImpContinue_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_CONTINUE);
	Object_registerCMethod(self, "__activate", ImpContinue_activate_);
}