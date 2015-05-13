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
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(caller));
	assert(argc == 2);
	assert(ImpRoute_isValid(argv[0]));
	assert(Object_isValid(argv[1]));
	Object *route = argv[0];
	Object *value = argv[1];


	Object *par = context;
	int rargc = ImpRoute_argc(route);

	for(int i = 0; i < rargc - 1; i++){
		char buf[64];
		ImpRoute_argv(route, i, buf);
		par = Object_getDeep(par, buf);
		if(!par){
			char err[128];
			sprintf(err, "variable '%s' does not exist.", ImpRoute_getRaw(route));
			Runtime_throwString(runtime, err);
			return NULL;
		}
	}
	char fbuf[64];
	ImpRoute_argv(route, rargc - 1, fbuf);
	if(!Object_hasKeyShallow(par, fbuf)){
		char err[128];
		sprintf(err, "variable '%s' does not exist.", ImpRoute_getRaw(route));
		Runtime_throwString(runtime, err);
		return NULL;
	}
	Object_putDeep(par, fbuf, value);
	return NULL;
}

void ImpSet_init(Object *self){
	BuiltIn_setId(self, BUILTIN_SETTER);
	Object_registerCMethod(self, "__activate", ImpSet_activate_internal);
}