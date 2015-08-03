#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "../runtime.h"

#include "general.h"
#include "number.h"
#include "route.h"
#include "string.h"




bool iNumber_isValid(iObject *self){
	return iObject_isValid(self) &&
	       iBuiltin_id(self) == iBUILTIN_NUMBER;
}


static double *iNumber_data(iObject *self){
	assert(iNumber_isValid(self));
	return (double*) iObject_getDataDeep(self, "__data");
}


static iObject *add_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	assert(runtime);
	assert(iNumber_isValid(caller));

	if(argc == 0){
		iRuntime_throwString(runtime, context, "Number:add requires at least one argument.");
		return NULL;
	}

	double val = iNumber_getRaw(caller);
	for(int i = 0; i < argc; i++){
		if(!iNumber_isValid(argv[i])){
			iRuntime_throwString(runtime, context, "Number:add accepts only numbers as arguments");
			return NULL;
		}
		val += iNumber_getRaw(argv[i]);
	}
	iNumber_setRaw(caller, val);

	return NULL;
}


void iNumber_sub(iObject *self, iObject *other){
	assert(iNumber_isValid(self));
	assert(iNumber_isValid(other));
	double *data = iNumber_data(self);
	*data = (*data) - iNumber_getRaw(other);
}


static iObject *sub_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iNumber_isValid(caller));

	if(argc < 1){
		iRuntime_throwString(runtime, context, "Number:sub requires at least 1 argument.");
	}

	for(int i = 0; i < argc; i++){
		if(!iNumber_isValid(argv[i])){
			iRuntime_throwString(runtime, context, "Number:sub argument must be of number type.");
		} else {
			iNumber_sub(caller, argv[i]);
		}
	}

	return NULL;
}


void iNumber_mult(iObject *self, iObject *other){
	assert(iNumber_isValid(self));
	assert(iNumber_isValid(other));

	double *data = iNumber_data(self);
	*data = (*data) * iNumber_getRaw(other);
}


static iObject *mult_(iRuntime *runtime
	                , iObject *context
	                , iObject *caller
	                , int argc
	                , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iNumber_isValid(caller));

	if(argc == 0){
		iRuntime_throwString(runtime, context, "Number:mult requires at least one argument.");
	}

	for(int i = 0; i < argc; i++){
		if(!iNumber_isValid(argv[i])){
			iRuntime_throwString(runtime, context, "Number:mult accepts only numbers as arguments");
		}
		iNumber_mult(caller, argv[i]);
	}

	return NULL;
}


void iNumber_div(iObject *self, iObject *other){
	assert(iNumber_isValid(self));
	assert(iNumber_isValid(other));
	double *data = iNumber_data(self);
	*data = (*data) / iNumber_getRaw(other);
}


static iObject *div_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iNumber_isValid(caller));


	if(argc != 1){
		iRuntime_throwString(runtime, context, "Number:div requires exactly one argument.");
	} else{
		if(!iNumber_isValid(argv[0])){
			iRuntime_throwString(runtime, context, "Number:div argument must be of number type.");
		} else {
			iNumber_div(caller, argv[0]);
		}
	}



	return NULL;
}


void iNumber_print(iObject *self){
	assert(iNumber_isValid(self));
	printf("%f", iNumber_getRaw(self));
}


static iObject *print_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(iNumber_isValid(caller));

	if(argc > 0){
		iRuntime_throwString(runtime, context, "Number:print does not accept arguments.");
	}

	iNumber_print(caller);
	return NULL;
}


static iObject *setEq_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iObject_isValid(caller));

	if(argc != 1){
		iRuntime_throwString(runtime, context, "Number:set requires exactly one argument.");
	} 

	if(!iNumber_isValid(argv[0])){
		iRuntime_throwString(runtime, context, "Number:set requires an argument of type number");
	} else{
		iNumber_setRaw(caller, iNumber_getRaw(argv[0]));
	}

	return NULL;
}


void iNumber_setRaw(iObject *self, double value){
	assert(iNumber_isValid(self));

	double *existing = iNumber_data(self);
	if(existing){
		*existing = value;
		return;
	}
	void *data = malloc(sizeof(double));
	if(!data){
		abort();
	}
	memcpy(data, &value, sizeof(double));
	iObject_putDataDeep(self, "__data", data);
}


double iNumber_getRaw(iObject *self){
	assert(iNumber_isValid(self));
	return *((double*) iObject_getDataDeep(self, "__data"));
}


int iNumber_getRawRounded(iObject *self){
	double raw = iNumber_getRaw(self);
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

static iObject *clone_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(self);
	assert(argc == 0);

	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:~ does not accept arguments");
	}

	iObject *r = iRuntime_simpleClone(runtime, self);
	void *data = malloc(sizeof(double));
	if(!data){
		abort();
	}
	iObject_putDataShallow(r, "__data", data);
	iNumber_setRaw(r, iNumber_getRaw(self));
	return r;
}



static iObject *asString_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *caller
	                    , int argc
	                    , iObject **argv){
	assert(runtime);
	assert(caller);
	assert(argc == 0);

	iObject *r = iRuntime_cloneField(runtime, "String");
	char buf[32];
	sprintf(buf, "%f", iNumber_getRaw(caller));
	iString_setRaw(r, buf);
	return r;
}



static iObject *cmp_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Number:<> requires exactly one argument");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "Number:<> requires another number as its argument");
	}

	iObject *r = iRuntime_cloneField(runtime, "Number");
	const double n0 = iNumber_getRaw(caller);
	const double n1 = iNumber_getRaw(argv[0]);
	if(n0 == n1){
		iNumber_setRaw(r, 0);
	} else if(n0 > n1){
		iNumber_setRaw(r, 1);
	} else if(n0 < n1){
		iNumber_setRaw(r, -1);
	}
	return r;
}


static iObject *mod_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Number:%= requires exactly one argument");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "Number:%= requires another number as its argument");
	}

	iNumber_setRaw(caller, fmod(iNumber_getRaw(caller), iNumber_getRaw(argv[0])));
	return NULL;
}


static iObject *powEq_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Number:**= requires exactly one argument");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "Number:**= requires another number as its argument");
	}

	iNumber_setRaw(caller, pow(iNumber_getRaw(caller), iNumber_getRaw(argv[0])));
	return NULL;
}


static iObject *inc_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:++ does not accept");
	}

	iNumber_setRaw(caller, iNumber_getRaw(caller) + 1);
	return caller;
}

static iObject *dec_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:-- does not accept");
	}

	iNumber_setRaw(caller, iNumber_getRaw(caller) - 1);
	return caller;
}

static iObject *copy_(iRuntime *runtime
	                , iObject *context
	                , iObject *self
	                , int argc
	                , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:$ does not accept arguments");
	}

	iObject *r = iRuntime_simpleClone(runtime, self); 
	double *data = malloc(sizeof(double));
	*data = iNumber_getRaw(self);
	iObject_putDataShallow(r, "__data", data);
	return r;
}


static iObject *asBoolean_(iRuntime *runtime
	                     , iObject *context
	                     , iObject *self
	                     , int argc
	                     , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:? does not accept arguments");
	}

	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, iNumber_getRaw(self)? 1:0);
	return r;
}


static iObject *squareRoot_(iRuntime *runtime
	                      , iObject *context
	                      , iObject *self
	                      , int argc
	                      , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:squareRoot does not accept arguments");
	}
	double raw = iNumber_getRaw(self);
	if(raw < 0){
		iRuntime_throwString(runtime, context, "cannot take square root of negative number");
	}
	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, sqrt(raw));
	return r;
}


static iObject *floor_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	iNumber_setRaw(self, floor(iNumber_getRaw(self)));
	return NULL;
}


static iObject *floored_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *self
	                   , int argc
	                   , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	iObject *r = iRuntime_callMethod(runtime
		                          , context
		                          , self
		                          , "$"
		                          , 0
		                          , NULL);
	iRuntime_callMethod(runtime
		             , context
		             , r
		             , "floor"
		             , 0
		             , NULL);
	return r;
}


static iObject *round_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	iNumber_setRaw(self, iNumber_getRawRounded(self));
	return NULL;
}

static iObject *rounded_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *self
	                   , int argc
	                   , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	iObject *r = iRuntime_callMethod(runtime
		                          , context
		                          , self
		                          , "$"
		                          , 0
		                          , NULL);
	iRuntime_callMethod(runtime
		             , context
		             , r
		             , "round"
		             , 0
		             , NULL);
	return r;
}


static iObject *square_(iRuntime *runtime
	                  , iObject *context
	                  , iObject *self
	                  , int argc
	                  , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:square does not accept arguments");
	}
	const double raw = iNumber_getRaw(self);
	iNumber_setRaw(self, raw * raw);
	return NULL;
}

static iObject *squared_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *self
	                   , int argc
	                   , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:squared does not accept arguments");
	}
	iObject *r = iRuntime_callMethod(runtime
		                          , context
		                          , self
		                          , "$"
		                          , 0
		                          , NULL);
	iRuntime_callMethod(runtime
		             , context
		             , r
		             , "square"
		             , 0
		             , NULL);
	return r;
}



static iObject *cube_(iRuntime *runtime
	                , iObject *context
	                , iObject *self
	                , int argc
	                , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	const double raw = iNumber_getRaw(self);
	iNumber_setRaw(self, raw * raw * raw);
	return NULL;
}

static iObject *cubed_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Number:floor does not accept arguments");
	}
	iObject *r = iRuntime_callMethod(runtime
		                          , context
		                          , self
		                          , "$"
		                          , 0
		                          , NULL);
	iRuntime_callMethod(runtime
		             , context
		             , r
		             , "cube"
		             , 0
		             , NULL);
	return r;
}




static iObject *hashCode_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *self
	                    , int argc
	                    , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "#:hashCode does not accept arguments");
	}

	iObject *r = iRuntime_cloneField(runtime, "Number");
	uint64_t hc = 654188429 ^ ((uint64_t) iNumber_getRaw(self));
	iNumber_setRaw(r, *((double*) &hc));
	return r;
} 


void iNumber_init(iObject *self, iRuntime *runtime){
	assert(self);
	iBuiltin_setId(self, iBUILTIN_NUMBER);

	iRuntime_registerCMethod(runtime, self, "+=", add_);
	iRuntime_registerCMethod(runtime, self, "-=", sub_);
	iRuntime_registerCMethod(runtime, self, "*=", mult_);
	iRuntime_registerCMethod(runtime, self, "/=", div_);
	iRuntime_registerCMethod(runtime, self, "%=", mod_);
	iRuntime_registerCMethod(runtime, self, "^=", powEq_);
	iRuntime_registerCMethod(runtime, self, "<>", cmp_);

	iRuntime_registerCMethod(runtime, self, "squareRoot", squareRoot_);

	iRuntime_registerCMethod(runtime, self, "++", inc_);
	iRuntime_registerCMethod(runtime, self, "--", dec_);

	iRuntime_registerCMethod(runtime, self, "$", copy_);
	iRuntime_registerCMethod(runtime, self, "?", asBoolean_);

	iRuntime_registerCMethod(runtime, self, "print", print_);
	iRuntime_registerCMethod(runtime, self, "=", setEq_);

	iRuntime_registerCMethod(runtime, self, "~", clone_);
	iRuntime_registerCMethod(runtime, self, "asString", asString_);

	iRuntime_registerCMethod(runtime, self, "_hashCode", hashCode_);

	iRuntime_registerCMethod(runtime, self, "floor", floor_);
	iRuntime_registerCMethod(runtime, self, "floored", floored_);
	iRuntime_registerCMethod(runtime, self, "round", round_);
	iRuntime_registerCMethod(runtime, self, "rounded", rounded_);

	iRuntime_registerCMethod(runtime, self, "square", square_);
	iRuntime_registerCMethod(runtime, self, "squared", squared_);
	iRuntime_registerCMethod(runtime, self, "cube", cube_);
	iRuntime_registerCMethod(runtime, self, "cubed", cubed_);

	iNumber_setRaw(self, 0);
}
