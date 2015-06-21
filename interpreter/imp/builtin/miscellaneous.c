#include "miscellaneous.h"
#include "../c.h"
#include "general.h"
#include "route.h"
#include "string.h"
#include <string.h>



static Object *def_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(argv[0]));
	assert(Object_isValid(argv[1]));

	Object *route = argv[0];
	Object *value = unrouteInContext(argv[1], context);
	if(!value){
		if(BuiltIn_id(argv[1]) == BUILTIN_ROUTE){
			Runtime_throwFormatted(runtime, "variable does not exist '%s'", ImpRoute_getRaw(argv[1]));
		} else {
			Runtime_throwFormatted(runtime, "cannot assign '%s' to NULL", ImpRoute_getRaw(argv[0]));
		}
		return NULL;
	}
	Object_reference(value);

	if(argc != 2){
		Runtime_throwString(runtime, "def accepts exactly 2 arguments.");
	} else if(!ImpRoute_isValid(route)){
		Runtime_throwString(runtime, "def requires a route as first argument.");
	}

	// TODO: use ImpRoute_submapping here

	Object *par = context;
	int rargc = ImpRoute_argc(route);

	for(int i = 0; i < rargc - 1; i++){
		char buf[64];
		ImpRoute_argv(route, i, buf);
		Object *ppar = par;
		par = Object_getDeep(par, buf);
		if(!par){
			par = Runtime_make(runtime, Object);
			Object_putShallow(ppar, buf, par);
		}
	}
	char fbuf[64];
	ImpRoute_argv(route, rargc - 1, fbuf);
	if(strcmp(fbuf, "#") != 0 && Object_hasKeyShallow(par, fbuf)){
		Runtime_throwFormatted(runtime, "def would overwrite value '%s'", fbuf);
	}
	Object_putShallow(par, fbuf, value);

	Object_unreference(value);
	return NULL;
}


static Object *set_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc != 2){
		Runtime_throwString(runtime, "set requires exactly 2 arguments.");
		return NULL;
	} else if(!ImpRoute_isValid(argv[0])){
		Runtime_throwString(runtime, "set requires a route for argument 1.");
		return NULL;
	}

	Object *route = argv[0];
	Object *value = unrouteInContext(argv[1], context);
	if(!value){
		if(BuiltIn_id(argv[1]) == BUILTIN_ROUTE){
			Runtime_throwFormatted(runtime, "variable does not exist '%s'", ImpRoute_getRaw(argv[1]));
		} else {
			Runtime_throwFormatted(runtime, "cannot assign '%s' to NULL", ImpRoute_getRaw(argv[0]));
		}
		return NULL;
	}

	// if context defines a special setRoute method, use it
	if(Object_hasSpecialMethod(context, "setRoute")){
		Object *argv2[2];
		argv2[0] = Runtime_make(runtime, String);
		ImpString_setRaw(argv2[0], ImpRoute_getRaw(route));
		argv2[1] = value;
		Runtime_callMethod(runtime
			             , NULL
			             , context
			             , "setRoute"
			             , 2
			             , argv2);
		return NULL;
	}

	// otherwise, use default


	Object *parent = ImpRoute_submapping(route, context);
	char field[32];
	ImpRoute_argv(route, ImpRoute_argc(route) - 1, field);
	
	if(!parent || !Object_hasKeyDeep(parent, field)){
		Runtime_throwFormatted(runtime, "set failed on '%s'; try def.", ImpRoute_getRaw(route));
	}

	Object_putDeep(parent, field, value);

	return NULL;
}



static Object *break_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "break does not accept arguments");
	}
	Object *sig = Runtime_rawObject(runtime);
	BuiltIn_setId(sig, BUILTIN_BREAK);
	Runtime_setReturnValue(runtime, sig);
	return NULL;
}


static Object *continue_(Runtime *runtime
	                   , Object *context
	                   , Object *self
	                   , int argc
	                   , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "break does not accept arguments");
	}
	Object *sig = Runtime_rawObject(runtime);
	BuiltIn_setId(sig, BUILTIN_CONTINUE);
	Runtime_setReturnValue(runtime, sig);
	return NULL;
}


static Object *return_(Runtime *runtime
	                 , Object *context
	                 , Object *caller
	                 , int argc
	                 , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc > 1){
		Runtime_throwString(runtime, "return accepts only one parameter.");
	} else if(argc == 1){
		Runtime_setReturnValue(runtime, argv[0]);
		return argv[0];
	} else {
		Runtime_setReturnValue(runtime, NULL);
	}
	return NULL;
}


void ImpMisc_init(Object *self, Runtime *runtime){
	assert(self);
	Runtime_registerPrivelegedCMethod(runtime, self, "def", def_);
	Runtime_registerPrivelegedCMethod(runtime, self, "set", set_);
	Runtime_registerCMethod(runtime, self, "break", break_);
	Runtime_registerCMethod(runtime, self, "continue", continue_);
	Runtime_registerCMethod(runtime, self, "return", return_);

	Runtime_executeSourceInContext(runtime
		                         , "(def nil:asString 'nil')"
		                         , runtime->Object);
}