#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "general.h"
#include "route.h"
#include "../c.h"
#include <imp/lexer.h>



typedef struct {
	char *text;
	Object *context;
} Internal;


static Internal *getInternal(Object *self){
	assert(Object_isValid(self));
	return (Internal*) Object_getDataDeep(self, "__data");
}


char *ImpRoute_getRaw(Object *self){
	return getInternal(self)->text;
}


bool ImpRoute_isValid(Object *obj){
	return Object_isValid(obj)                      &&
	       BuiltIn_protoId(obj) == BUILTIN_ROUTE    &&
	       isValidRouteText(ImpRoute_getRaw(obj));
}


void ImpRoute_setRaw(Object *self, char *text){
	assert(self);
	assert(isValidRouteText(text));
	Internal *i = getInternal(self);
	if(i->text){
		free(i->text);
	}
	i->text = strdup(text);
}


void ImpRoute_print(Object *self){
	assert(ImpRoute_isValid(self));
	printf("%s", ImpRoute_getRaw(self));
}


static Object *print_(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpRoute_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, context, "Route:print does not accept arguments");
	} else {
		Runtime_print(runtime, context, ImpRoute_mapping(caller));
	}

	return NULL;	
}


void ImpRoute_set(Object *self, Object *value){
	assert(ImpRoute_isValid(self));
	assert(ImpRoute_isValid(value));
	ImpRoute_setRaw(self, ImpRoute_getRaw(value));
}


static Object *clone_(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, context, "Route:~ does not accept arguments");
		return NULL;
	}

	Object *r = Runtime_simpleClone(runtime, caller);
	Internal *i = malloc(sizeof(Internal));
	i->text = NULL;
	i->context = NULL;
	Object_putDataShallow(r, "__data", i);
	return r;
}


static Object *mark_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(Object_isValid(self));

	if(argc != 0){
		Runtime_throwString(runtime, context, "Route:_mark does not accept arguments");
		return NULL;
	}

	Internal *i = getInternal(self);
	if(i){
		Runtime_callMethod(runtime
			             , context
			             , i->context
			             , "_markRecursively"
			             , 0
			             , NULL);
	}
	return NULL;
}


static Object *activate_(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpRoute_isValid(caller));

	Object *r = NULL;
	bool found = true;

	Object *mapping = ImpRoute_mapping(caller);
	Object *submapping = ImpRoute_submapping(caller);
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
		                 , context
		                 , "variable '%s' does not exist"
		                 , ImpRoute_getRaw(caller));
	return NULL;
}


void ImpRoute_init(Object *self, Runtime *runtime){
	assert(self);
	BuiltIn_setId(self, BUILTIN_ROUTE);
	Runtime_registerCMethod(runtime, self, "~", clone_);
	Runtime_registerCMethod(runtime, self, "_markInternalsRecursively", mark_);
	Runtime_registerPrivelegedCMethod(runtime, self, "print", print_);
	Object_registerCActivator(self, activate_);
	Object_putDataShallow(self, "__privilege", NULL);
}


int ImpRoute_argc_(char *raw){
	assert(raw);

	while(*raw == ':'){
		++raw;
	}

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

	while(*raw == ':'){
		++raw;
	}

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
	// todo use submapping here to shorten
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


Object *ImpRoute_mapping(Object *self){
	assert(ImpRoute_isValid(self));
	const Internal *const i = getInternal(self);
	return ImpRoute_mapping_(i->text, i->context);
}


Object *ImpRoute_submapping(Object *self){
	assert(ImpRoute_isValid(self));
	const Internal *const i = getInternal(self);
	return ImpRoute_submapping_(i->text, i->context);
}


Object *unroute(Object *obj){
	if(!obj){
		return NULL;
	}
	assert(Object_isValid(obj));

	if(BuiltIn_id(obj) == BUILTIN_ROUTE){
		return ImpRoute_mapping(obj);
	}
	return obj;
}


void ImpRoute_setContext(Object *self, Object *context){
	getInternal(self)->context = context;
}
// TODO: mark 

