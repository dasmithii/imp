#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "general.h"
#include "route.h"
#include <imp/lexer.h>




char *ImpRoute_getRaw(Object *self){
	assert(Object_isValid(self));
	return (char*) Object_getDataDeep(self, "__data");
}





bool ImpRoute_isValid(Object *obj){
	return Object_isValid(obj)                      &&
	       BuiltIn_protoId(obj) == BUILTIN_ROUTE    &&
	       isValidRouteText(ImpRoute_getRaw(obj));
}


void ImpRoute_setRaw(Object *self, char *text){
	assert(self);
	assert(isValidRouteText(text));
	Object_putDataDeep(self, "__data", strdup(text));
}


void ImpRoute_print(Object *self){
	assert(ImpRoute_isValid(self));
	char *data = Object_getDataDeep(self, "__data");
	printf("%s", data);
}


static Object *ImpRoute_print_internal(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpRoute_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "route:print does not accept arguments");
	} else {
		Runtime_print(runtime, context, ImpRoute_mapping(caller, context));
	}

	return NULL;	
}


void ImpRoute_set(Object *self, Object *value){
	assert(ImpRoute_isValid(self));
	assert(ImpRoute_isValid(value));
	ImpRoute_setRaw(self, ImpRoute_getRaw(value));
}


static Object *ImpRoute_clone_internal(Runtime *runtime
	                                 , Object *context
	                                 , Object *caller
	                                 , int argc
	                                 , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "route:clone does not accept arguments");
		return NULL;
	}

	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	Object_putDataShallow(r, "__data", strdup(ImpRoute_getRaw(caller)));
	return r;
}


static Object *ImpRoute_activate_internal(Runtime *runtime
	                                    , Object *context
	                                    , Object *caller
	                                    , int argc
	                                    , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpRoute_isValid(caller));

	Object *r = NULL;
	bool found = true;

	Object *mapping = ImpRoute_mapping(caller, context);
	Object *submapping = ImpRoute_submapping(caller, context);
	if(!submapping){
		submapping = mapping;
	}

	// if maps to an object directly, activate said object
	if(mapping){
		r = Runtime_activateOn(runtime, context, mapping, argc, argv, submapping);
	} else if(submapping){
		// otherwise, try activating special or internal methods
		char method[32];
		ImpRoute_argv(caller
			        , ImpRoute_argc(caller) - 1
			        , method);
		return Runtime_callMethod(runtime, context, submapping, method, argc, argv);
	}
	
	// if activation successful, return
	if(found){
		return r;
	}

	Runtime_throwFormatted(runtime
		                 , "variable '%s' does not exist"
		                 , ImpRoute_getRaw(caller));
	return NULL;
}


void ImpRoute_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_ROUTE);
	Object_registerCMethod(self, "__print", ImpRoute_print_internal);
	Object_registerCMethod(self, "__clone", ImpRoute_clone_internal);
	Object_registerCMethod(self, "__activate", ImpRoute_activate_internal);
	ImpRoute_setRaw(self, "defaultRoute");
}


int ImpRoute_argc_(char *raw){
	assert(raw);
	int r = 1;
	while(*raw){
		if(*raw == ':'){
			++r;
		}
		++raw;
	}
	return r;
}


int ImpRoute_argc(Object *self){
	assert(ImpRoute_isValid(self));
	return ImpRoute_argc_(ImpRoute_getRaw(self));
}


void ImpRoute_argv_(char *raw, int i, char *dest){
	assert(raw);
	assert(i < ImpRoute_argc_(raw));
	assert(dest);

	int c = 0;
	while(c < i){
		if(*raw == ':'){
			++c;
		}
		++raw;
	}
	while(*raw && *raw != ':'){
		*dest = *raw;
		++raw;
		++dest;
	}
	*dest = 0;
}


void ImpRoute_argv(Object *self, int i, char *dest){
	assert(ImpRoute_isValid(self));
	assert(dest);
	return ImpRoute_argv_(ImpRoute_getRaw(self), i, dest);
}


Object *ImpRoute_submapping_(char *self, Object *context){
	int argc = ImpRoute_argc_(self);

	if(argc == 1){
		return context;
	}

	Object *r = context;
	for(int i = 0; i < argc - 1; i++){
		char cho[64];
		ImpRoute_argv_(self, i, cho);
		r = Object_getDeep(r, cho);
		if(!r){
			return NULL;
		}
	}
	if(r == context){
		return NULL;
	}
	return r;
}


Object *ImpRoute_mapping_(char *self, Object *context){
	Object *r = context;
	int argc = ImpRoute_argc_(self);
	for(int i = 0; i < argc; i++){
		char cho[64];
		ImpRoute_argv_(self, i, cho);
		r = Object_getDeep(r, cho);
		if(!r){
			return NULL;
		}
	}
	return r;
}


Object *ImpRoute_mapping(Object *self, Object *context){
	assert(ImpRoute_isValid(self));
	assert(Object_isValid(context));
	return ImpRoute_mapping_(ImpRoute_getRaw(self), context);
}


Object *ImpRoute_submapping(Object *self, Object *context){
	assert(ImpRoute_isValid(self));
	assert(Object_isValid(context));
	return ImpRoute_submapping_(ImpRoute_getRaw(self), context);
}


Object *unrouteInContext(Object *obj, Object *ctx){
	if(!obj){
		return NULL;
	}
	assert(Object_isValid(obj));
	assert(Object_isValid(ctx));

	if(BuiltIn_id(obj) == BUILTIN_ROUTE){
		return ImpRoute_mapping(obj, ctx);
	}
	return obj;
}

