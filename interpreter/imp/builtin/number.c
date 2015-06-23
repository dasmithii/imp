#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../runtime.h"

#include "general.h"
#include "number.h"
#include "route.h"
#include "string.h"



bool ImpNumber_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_NUMBER;
}


static double *ImpNumber_data(Object *self){
	assert(ImpNumber_isValid(self));
	return (double*) Object_getDataDeep(self, "__data");
}


static Object *add_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(ImpNumber_isValid(caller));

	if(argc == 0){
		Runtime_throwString(runtime, "Number:add requires at least one argument.");
		return NULL;
	}

	double val = ImpNumber_getRaw(caller);
	for(int i = 0; i < argc; i++){
		Object *arg = unrouteInContext(argv[i], context);
		if(!ImpNumber_isValid(arg)){
			Runtime_throwString(runtime, "Number:add accepts only numbers as arguments");
			return NULL;
		}
		val += ImpNumber_getRaw(argv[i]);
	}
	ImpNumber_setRaw(caller, val);

	return NULL;
}


void ImpNumber_sub(Object *self, Object *other){
	assert(ImpNumber_isValid(self));
	assert(ImpNumber_isValid(other));
	double *data = ImpNumber_data(self);
	*data = (*data) - ImpNumber_getRaw(other);
}


static Object *sub_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpNumber_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "Number:sub requires exactly one argument.");
	}

	if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "Number:sub argument must be of number type.");
	} else {
		ImpNumber_sub(caller, argv[0]);
	}

	return NULL;
}


void ImpNumber_mult(Object *self, Object *other){
	assert(ImpNumber_isValid(self));
	assert(ImpNumber_isValid(other));

	double *data = ImpNumber_data(self);
	*data = (*data) * ImpNumber_getRaw(other);
}


static Object *mult_(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpNumber_isValid(caller));

	if(argc == 0){
		Runtime_throwString(runtime, "Number:mult requires at least one argument.");
	}

	for(int i = 0; i < argc; i++){
		Object *arg = unrouteInContext(argv[i], context);
		if(!ImpNumber_isValid(arg)){
			Runtime_throwString(runtime, "Number:mult accepts only numbers as arguments");
		}
		ImpNumber_mult(caller, arg);
	}

	return NULL;
}


void ImpNumber_div(Object *self, Object *other){
	assert(ImpNumber_isValid(self));
	assert(ImpNumber_isValid(other));
	double *data = ImpNumber_data(self);
	*data = (*data) / ImpNumber_getRaw(other);
}


static Object *div_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpNumber_isValid(caller));


	if(argc != 1){
		Runtime_throwString(runtime, "Number:div requires exactly one argument.");
	} else{
		Object *arg = unrouteInContext(argv[0], context);

		if(!ImpNumber_isValid(arg)){
			Runtime_throwString(runtime, "Number:div argument must be of number type.");
		} else {
			ImpNumber_div(caller, arg);
		}
	}



	return NULL;
}


void ImpNumber_print(Object *self){
	assert(ImpNumber_isValid(self));
	printf("%f", ImpNumber_getRaw(self));
}


static Object *print_(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	assert(ImpNumber_isValid(caller));

	if(argc > 0){
		Runtime_throwString(runtime, "Number:print does not accept arguments.");
	}

	ImpNumber_print(caller);
	return NULL;
}


static Object *set_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "Number:set requires exactly one argument.");
	} 

	Object *arg = unrouteInContext(argv[0], context);
	if(!ImpNumber_isValid(arg)){
		Runtime_throwString(runtime, "Number:set requires an argument of type number");
	} else{
		ImpNumber_setRaw(caller, ImpNumber_getRaw(argv[0]));
	}

	return NULL;
}


void ImpNumber_setRaw(Object *self, double value){
	assert(ImpNumber_isValid(self));

	double *existing = ImpNumber_data(self);
	if(existing){
		*existing = value;
		return;
	}
	void *data = malloc(sizeof(double));
	if(!data){
		abort();
	}
	memcpy(data, &value, sizeof(double));
	Object_putDataDeep(self, "__data", data);
}


double ImpNumber_getRaw(Object *self){
	assert(ImpNumber_isValid(self));
	return *((double*) Object_getDataDeep(self, "__data"));
}

int ImpNumber_getRawRounded(Object *self){
	return (int) (ImpNumber_getRaw(self) + .5);
}

static Object *clone_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(self);
	assert(argc == 0);

	if(argc != 0){
		Runtime_throwString(runtime, "Number:~ does not accept arguments");
	}

	Object *r = Runtime_simpleClone(runtime, self);
	void *data = malloc(sizeof(double));
	if(!data){
		abort();
	}
	Object_putDataShallow(r, "__data", data);
	ImpNumber_setRaw(r, ImpNumber_getRaw(self));
	return r;
}



static Object *asString_(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	assert(runtime);
	assert(caller);
	assert(argc == 0);

	Object *r = Runtime_cloneField(runtime, "String");
	char buf[32];
	sprintf(buf, "%f", ImpNumber_getRaw(caller));
	ImpString_setRaw(r, buf);
	return r;
}



static Object *cmp_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Number:<> requires exactly one argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Number:<> requires another number as its argument");
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	const double n0 = ImpNumber_getRaw(caller);
	const double n1 = ImpNumber_getRaw(argv[0]);
	if(n0 == n1){
		ImpNumber_setRaw(r, 0);
	} else if(n0 > n1){
		ImpNumber_setRaw(r, 1);
	} else if(n0 < n1){
		ImpNumber_setRaw(r, -1);
	}
	return r;
}


static Object *mod_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Number:%= requires exactly one argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Number:%= requires another number as its argument");
	}

	ImpNumber_setRaw(caller, fmod(ImpNumber_getRaw(caller), ImpNumber_getRaw(argv[0])));
	return NULL;
}

static Object *inc_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Number:++ does not accept");
	}

	ImpNumber_setRaw(caller, ImpNumber_getRaw(caller) + 1);
	return caller;
}

static Object *dec_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Number:-- does not accept");
	}

	ImpNumber_setRaw(caller, ImpNumber_getRaw(caller) - 1);
	return caller;
}

static Object *copy_(Runtime *runtime
	               , Object *context
	               , Object *self
	               , int argc
	               , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Number:$ does not accept arguments");
	}

	Object *r = Runtime_simpleClone(runtime, self); 
	double *data = malloc(sizeof(double));
	*data = ImpNumber_getRaw(self);
	Object_putDataShallow(r, "__data", data);
	return r;
}


static Object *asBoolean_(Runtime *runtime
	                    , Object *context
	                    , Object *self
	                    , int argc
	                    , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Number:? does not accept arguments");
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	ImpNumber_setRaw(r, ImpNumber_getRaw(self)? 1:0);
	return r;
}


static Object *hashCode_(Runtime *runtime
	                   , Object *context
	                   , Object *self
	                   , int argc
	                   , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:hashCode does not accept arguments");
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	const unsigned long constant = 654188429;
	const unsigned long raw = (unsigned long) ImpNumber_getRaw(self);
	ImpNumber_setRaw(r, (double) ((constant ^ raw)));
	return r;
} 


void ImpNumber_init(Object *self, Runtime *runtime){
	assert(self);
	BuiltIn_setId(self, BUILTIN_NUMBER);

	Runtime_registerCMethod(runtime, self, "+=", add_);
	Runtime_registerCMethod(runtime, self, "-=", sub_);
	Runtime_registerCMethod(runtime, self, "*=", mult_);
	Runtime_registerCMethod(runtime, self, "/=", div_);
	Runtime_registerCMethod(runtime, self, "%=", mod_);
	Runtime_registerCMethod(runtime, self, "<>", cmp_);

	Runtime_registerCMethod(runtime, self, "++", inc_);
	Runtime_registerCMethod(runtime, self, "--", dec_);

	Runtime_registerCMethod(runtime, self, "$", copy_);
	Runtime_registerCMethod(runtime, self, "?", asBoolean_);

	Runtime_registerCMethod(runtime, self, "print", print_);
	Runtime_registerCMethod(runtime, self, "=", set_);

	Runtime_registerCMethod(runtime, self, "~", clone_);
	Runtime_registerCMethod(runtime, self, "asString", asString_);

	Runtime_registerCMethod(runtime, self, "hashCode", hashCode_);

	ImpNumber_setRaw(self, 0);
}
