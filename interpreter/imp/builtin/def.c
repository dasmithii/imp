#include "../object.h"
#include "../c.h"

#include "def.h"
#include "general.h"
#include "route.h"




static Object *ImpDef_activate_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(caller));
	assert(Object_isValid(argv[0]));

	Object *route = argv[0];
	Object *value = unrouteInContext(argv[1], context);
	Object_reference(value);

	if(argc != 2){
		Runtime_throwString(runtime, "def accepts exactly 2 arguments.");
	} else if(!ImpRoute_isValid(route)){
		Runtime_throwString(runtime, "def requires a route as first argument.");
	}


	Object *par = context;
	int rargc = ImpRoute_argc(route);

	for(int i = 0; i < rargc - 1; i++){
		char buf[64];
		ImpRoute_argv(route, i, buf);
		Object *ppar = par;
		par = Object_getDeep(par, buf);
		if(!par){
			par = Runtime_newObject(runtime);
			Object_putShallow(ppar, buf, par);
		}
	}
	char fbuf[64];
	ImpRoute_argv(route, rargc - 1, fbuf);
	if(Object_hasKeyShallow(par, fbuf)){
		Runtime_throwString(runtime, "def would overwrite value");
	}
	Object_putShallow(par, fbuf, value);

	Object_unreference(value);
	return NULL;
}


void ImpDef_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_DEFINER);
	Object_registerCMethod(self, "__activate", ImpDef_activate_internal);
}