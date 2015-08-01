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
		Runtime_throwString(runtime, context, "Number:add requires at least one argument.");
		return NULL;
	}

	double val = ImpNumber_getRaw(caller);
	for(int i = 0; i < argc; i++){
		if(!ImpNumber_isValid(argv[i])){
			Runtime_throwString(runtime, context, "Number:add accepts only numbers as arguments");
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

	if(argc < 1){
		Runtime_throwString(runtime, context, "Number:sub requires at least 1 argument.");
	}

	for(int i = 0; i < argc; i++){
		if(!ImpNumber_isValid(argv[i])){
			Runtime_throwString(runtime, context, "Number:sub argument must be of number type.");
		} else {
			ImpNumber_sub(caller, argv[i]);
		}
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
		Runtime_throwString(runtime, context, "Number:mult requires at least one argument.");
	}

	for(int i = 0; i < argc; i++){
		if(!ImpNumber_isValid(argv[i])){
			Runtime_throwString(runtime, context, "Number:mult accepts only numbers as arguments");
		}
		ImpNumber_mult(caller, argv[i]);
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
		Runtime_throwString(runtime, context, "Number:div requires exactly one argument.");
	} else{
		if(!ImpNumber_isValid(argv[0])){
			Runtime_throwString(runtime, context, "Number:div argument must be of number type.");
		} else {
			ImpNumber_div(caller, argv[0]);
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
		Runtime_throwString(runtime, context, "Number:print does not accept arguments.");
	}

	ImpNumber_print(caller);
	return NULL;
}


static Object *setEq_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, context, "Number:set requires exactly one argument.");
	} 

	if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, context, "Number:set requires an argument of type number");
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
	double raw = ImpNumber_getRaw(self);
	const bool sign = raw < 0;
	if(sign){
		raw *= -1;
	}
	int r = (int) raw + .5;
	if(sign){
		r *= -1;
	}
	return r;
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
		Runtime_throwString(runtime, context, "Number:~ does not accept arguments");
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
		Runtime_throwString(runtime, context, "Number:<> requires exactly one argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "Number:<> requires another number as its argument");
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
		Runtime_throwString(runtime, context, "Number:%= requires exactly one argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "Number:%= requires another number as its argument");
	}

	ImpNumber_setRaw(caller, fmod(ImpNumber_getRaw(caller), ImpNumber_getRaw(argv[0])));
	return NULL;
}


static Object *powEq_(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, context, "Number:**= requires exactly one argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "Number:**= requires another number as its argument");
	}

	ImpNumber_setRaw(caller, pow(ImpNumber_getRaw(caller), ImpNumber_getRaw(argv[0])));
	return NULL;
}


static Object *inc_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:++ does not accept");
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
		Runtime_throwString(runtime, context, "Number:-- does not accept");
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
		Runtime_throwString(runtime, context, "Number:$ does not accept arguments");
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
		Runtime_throwString(runtime, context, "Number:? does not accept arguments");
	}

	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, ImpNumber_getRaw(self)? 1:0);
	return r;
}


static Object *squareRoot_(Runtime *runtime
	                     , Object *context
	                     , Object *self
	                     , int argc
	                     , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:squareRoot does not accept arguments");
	}
	double raw = ImpNumber_getRaw(self);
	if(raw < 0){
		Runtime_throwString(runtime, context, "cannot take square root of negative number");
	}
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, sqrt(raw));
	return r;
}


static Object *floor_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	ImpNumber_setRaw(self, floor(ImpNumber_getRaw(self)));
	return NULL;
}


static Object *floored_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	Object *r = Runtime_callMethod(runtime
		                         , context
		                         , self
		                         , "$"
		                         , 0
		                         , NULL);
	Runtime_callMethod(runtime
		             , context
		             , r
		             , "floor"
		             , 0
		             , NULL);
	return r;
}


static Object *round_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	ImpNumber_setRaw(self, ImpNumber_getRawRounded(self));
	return NULL;
}

static Object *rounded_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	Object *r = Runtime_callMethod(runtime
		                         , context
		                         , self
		                         , "$"
		                         , 0
		                         , NULL);
	Runtime_callMethod(runtime
		             , context
		             , r
		             , "round"
		             , 0
		             , NULL);
	return r;
}


static Object *square_(Runtime *runtime
	                 , Object *context
	                 , Object *self
	                 , int argc
	                 , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:square does not accept arguments");
	}
	const double raw = ImpNumber_getRaw(self);
	ImpNumber_setRaw(self, raw * raw);
	return NULL;
}

static Object *squared_(Runtime *runtime
	                  , Object *context
	                  , Object *self
	                  , int argc
	                  , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:squared does not accept arguments");
	}
	Object *r = Runtime_callMethod(runtime
		                         , context
		                         , self
		                         , "$"
		                         , 0
		                         , NULL);
	Runtime_callMethod(runtime
		             , context
		             , r
		             , "square"
		             , 0
		             , NULL);
	return r;
}



static Object *cube_(Runtime *runtime
	               , Object *context
	               , Object *self
	               , int argc
	               , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	const double raw = ImpNumber_getRaw(self);
	ImpNumber_setRaw(self, raw * raw * raw);
	return NULL;
}

static Object *cubed_(Runtime *runtime
	               , Object *context
	               , Object *self
	               , int argc
	               , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	Object *r = Runtime_callMethod(runtime
		                         , context
		                         , self
		                         , "$"
		                         , 0
		                         , NULL);
	Runtime_callMethod(runtime
		             , context
		             , r
		             , "cube"
		             , 0
		             , NULL);
	return r;
}




static Object *hashCode_(Runtime *runtime
	                   , Object *context
	                   , Object *self
	                   , int argc
	                   , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "#:hashCode does not accept arguments");
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
	Runtime_registerCMethod(runtime, self, "^=", powEq_);
	Runtime_registerCMethod(runtime, self, "<>", cmp_);

	Runtime_registerCMethod(runtime, self, "squareRoot", squareRoot_);

	Runtime_registerCMethod(runtime, self, "++", inc_);
	Runtime_registerCMethod(runtime, self, "--", dec_);

	Runtime_registerCMethod(runtime, self, "$", copy_);
	Runtime_registerCMethod(runtime, self, "?", asBoolean_);

	Runtime_registerCMethod(runtime, self, "print", print_);
	Runtime_registerCMethod(runtime, self, "=", setEq_);

	Runtime_registerCMethod(runtime, self, "~", clone_);
	Runtime_registerCMethod(runtime, self, "asString", asString_);

	Runtime_registerCMethod(runtime, self, "_hashCode", hashCode_);

	Runtime_registerCMethod(runtime, self, "floor", floor_);
	Runtime_registerCMethod(runtime, self, "floored", floored_);
	Runtime_registerCMethod(runtime, self, "round", round_);
	Runtime_registerCMethod(runtime, self, "rounded", rounded_);

	Runtime_registerCMethod(runtime, self, "square", square_);
	Runtime_registerCMethod(runtime, self, "squared", squared_);
	Runtime_registerCMethod(runtime, self, "cube", cube_);
	Runtime_registerCMethod(runtime, self, "cubed", cubed_);

	ImpNumber_setRaw(self, 0);
}
