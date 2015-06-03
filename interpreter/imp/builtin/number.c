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


void ImpNumber_add(Object *self, Object *other){
	assert(ImpNumber_isValid(self));
	assert(ImpNumber_isValid(other));

	double *data = ImpNumber_data(self);
	*data += ImpNumber_getRaw(other);
}


static Object *ImpNumber_add_(Runtime *runtime
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

	for(int i = 0; i < argc; i++){
		Object *arg = unrouteInContext(argv[i], context);
		if(!ImpNumber_isValid(arg)){
			Runtime_throwString(runtime, "Number:add accepts only numbers as arguments");
			return NULL;
		}
		ImpNumber_add(caller, arg);
	}

	return NULL;
}


void ImpNumber_sub(Object *self, Object *other){
	assert(ImpNumber_isValid(self));
	assert(ImpNumber_isValid(other));
	double *data = ImpNumber_data(self);
	*data = (*data) - ImpNumber_getRaw(other);
}


static Object *ImpNumber_sub_(Runtime *runtime
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


static Object *ImpNumber_mult_(Runtime *runtime
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


static Object *ImpNumber_div_(Runtime *runtime
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


static Object *ImpNumber_print_(Runtime *runtime
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


static Object *ImpNumber_set_(Runtime *runtime
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


static Object *ImpNumber_clone_(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(caller);
	assert(argc == 0);

	if(argc != 0){
		Runtime_throwString(runtime, "Number:clone does not accept arguments");
	}



	Object_reference(caller);
	Object_reference(context);

	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	void *data = malloc(sizeof(double));
	if(!data){
		abort();
	}
	Object_putDataShallow(r, "__data", data);
	ImpNumber_setRaw(r, ImpNumber_getRaw(caller));

	Object_unreference(caller);
	Object_reference(context);

	return r;
}



static Object *ImpNumber_asString_(Runtime *runtime
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



static Object *ImpNumber_cmp_(Runtime *runtime
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
	ImpNumber_setRaw(r, ImpNumber_getRaw(caller) - ImpNumber_getRaw(argv[0]));
	return r;
}


static Object *ImpNumber_mod_(Runtime *runtime
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

static Object *ImpNumber_inc_(Runtime *runtime
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

static Object *ImpNumber_dec_(Runtime *runtime
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

static Object *ImpNumber_copy_(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Number:$ does not accept arguments");
	}

	Object *r = Runtime_clone(runtime, caller); // TODO: fix clone
	double *data = malloc(sizeof(double));
	*data = ImpNumber_getRaw(caller);
	Object_putDataShallow(r, "__data", data);
	return r;
}


static Object *ImpNumber_asBoolean_(Runtime *runtime
	                              , Object *context
	                              , Object *caller
	                              , int argc
	                              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Number:? does not accept arguments");
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	if(ImpNumber_getRaw(caller) == 0){
		ImpNumber_setRaw(r, 0);
	} else {
		ImpNumber_setRaw(r, 1);
	}
	return r;
}


void ImpNumber_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_NUMBER);

	Object_registerCMethod(self, "__+=", ImpNumber_add_);
	Object_registerCMethod(self, "__-=", ImpNumber_sub_);
	Object_registerCMethod(self, "__*=", ImpNumber_mult_);
	Object_registerCMethod(self, "__/=", ImpNumber_div_);
	Object_registerCMethod(self, "__%=", ImpNumber_mod_);
	Object_registerCMethod(self, "__<>", ImpNumber_cmp_);

	Object_registerCMethod(self, "__++", ImpNumber_inc_);
	Object_registerCMethod(self, "__--", ImpNumber_dec_);

	Object_registerCMethod(self, "__$", ImpNumber_copy_);
	Object_registerCMethod(self, "__?", ImpNumber_asBoolean_);

	Object_registerCMethod(self, "__print", ImpNumber_print_);
	Object_registerCMethod(self, "__set", ImpNumber_set_);

	Object_registerCMethod(self, "__clone", ImpNumber_clone_);
	Object_registerCMethod(self, "__asString", ImpNumber_asString_);
	// Object_registerCMethod(self, "__equals", ImpNumber_equals_);
	ImpNumber_setRaw(self, 0);
}
