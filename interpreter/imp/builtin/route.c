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
	iObject *context;
} Internal;


static Internal *getInternal(iObject *self){
	assert(iObject_isValid(self));
	return (Internal*) iObject_getDataDeep(self, "__data");
}


char *iRoute_getRaw(iObject *self){
	return getInternal(self)->text;
}


bool iRoute_isValid(iObject *obj){
	return iObject_isValid(obj)                      &&
	       iBuiltin_protoId(obj) == iBUILTIN_ROUTE    &&
	       iIsValidRouteText(iRoute_getRaw(obj));
}


void iRoute_setRaw(iObject *self, char *text){
	assert(self);
	assert(iIsValidRouteText(text));
	Internal *i = getInternal(self);
	if(i->text){
		free(i->text);
	}
	i->text = strdup(text);
}


void iRoute_print(iObject *self){
	assert(iRoute_isValid(self));
	printf("%s", iRoute_getRaw(self));
}


static iObject *print_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iRoute_isValid(caller));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "Route:print does not accept arguments");
	} else {
		iRuntime_print(runtime, context, iRoute_mapping(caller));
	}

	return NULL;	
}


void iRoute_set(iObject *self, iObject *value){
	assert(iRoute_isValid(self));
	assert(iRoute_isValid(value));
	iRoute_setRaw(self, iRoute_getRaw(value));
}


static iObject *clone_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(caller));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "Route:~ does not accept arguments");
		return NULL;
	}

	iObject *r = iRuntime_simpleClone(runtime, caller);
	Internal *i = malloc(sizeof(Internal));
	i->text = NULL;
	i->context = NULL;
	iObject_putDataShallow(r, "__data", i);
	return r;
}


static iObject *mark_(iRuntime *runtime
	                , iObject *context
	                , iObject *self
	                , int argc
	                , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(self));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "Route:_mark does not accept arguments");
		return NULL;
	}

	Internal *i = getInternal(self);
	if(i){
		iRuntime_callMethod(runtime
			             , context
			             , i->context
			             , "_markRecursively"
			             , 0
			             , NULL);
	}
	return NULL;
}


static iObject *activate_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *caller
	                    , int argc
	                    , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iRoute_isValid(caller));

	iObject *r = NULL;
	bool found = true;

	iObject *mapping = iRoute_mapping(caller);
	iObject *submapping = iRoute_submapping(caller);
	if(!submapping){
		submapping = mapping;
	}

	// if maps to an object directly, activate said object
	if(mapping){
		r = iRuntime_activateOn(runtime, context, mapping, argc, argv, submapping);
	} else if(submapping){
		// otherwise, try activating special or internal methods
		char method[32];
		iRoute_argv(caller
			        , iRoute_argc(caller) - 1
			        , method);
		return iRuntime_callMethod(runtime, context, submapping, method, argc, argv);
	}
	
	// if activation successful, return
	if(found){
		return r;
	}

	iRuntime_throwFormatted(runtime
		                 , context
		                 , "variable '%s' does not exist"
		                 , iRoute_getRaw(caller));
	return NULL;
}


void iRoute_init(iObject *self, iRuntime *runtime){
	assert(self);
	iBuiltin_setId(self, iBUILTIN_ROUTE);
	iRuntime_registerCMethod(runtime, self, "~", clone_);
	iRuntime_registerCMethod(runtime, self, "_markInternalsRecursively", mark_);
	iRuntime_registerPrivelegedCMethod(runtime, self, "print", print_);
	iObject_registerCActivator(self, activate_);
	iObject_putDataShallow(self, "__privilege", NULL);
}


int iRoute_argc_(char *raw){
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


int iRoute_argc(iObject *self){
	assert(iRoute_isValid(self));
	return iRoute_argc_(iRoute_getRaw(self));
}


void iRoute_argv_(char *raw, int i, char *dest){
	assert(raw);
	assert(i < iRoute_argc_(raw));
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


void iRoute_argv(iObject *self, int i, char *dest){
	assert(iRoute_isValid(self));
	assert(dest);
	return iRoute_argv_(iRoute_getRaw(self), i, dest);
}


iObject *iRoute_submapping_(char *self, iObject *context){
	int argc = iRoute_argc_(self);

	if(argc == 1){
		return context;
	}

	iObject *r = context;
	for(int i = 0; i < argc - 1; i++){
		char cho[64];
		iRoute_argv_(self, i, cho);
		r = iObject_getDeep(r, cho);
		if(!r){
			return NULL;
		}
	}
	if(r == context){
		return NULL;
	}
	return r;
}


iObject *iRoute_mapping_(char *self, iObject *context){
	// todo use submapping here to shorten
	iObject *r = context;
	int argc = iRoute_argc_(self);
	for(int i = 0; i < argc; i++){
		char cho[64];
		iRoute_argv_(self, i, cho);
		r = iObject_getDeep(r, cho);
		if(!r){
			return NULL;
		}
	}
	return r;
}


iObject *iRoute_mapping(iObject *self){
	assert(iRoute_isValid(self));
	const Internal *const i = getInternal(self);
	return iRoute_mapping_(i->text, i->context);
}


iObject *iRoute_submapping(iObject *self){
	assert(iRoute_isValid(self));
	const Internal *const i = getInternal(self);
	return iRoute_submapping_(i->text, i->context);
}


iObject *unroute(iObject *obj){
	if(!obj){
		return NULL;
	}
	assert(iObject_isValid(obj));

	if(iBuiltin_id(obj) == iBUILTIN_ROUTE){
		return iRoute_mapping(obj);
	}
	return obj;
}


void iRoute_setContext(iObject *self, iObject *context){
	getInternal(self)->context = context;
}
// TODO: mark 

