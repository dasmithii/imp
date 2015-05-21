#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../runtime.h"

#include "general.h"
#include "number.h"




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


static Object *ImpNumber_add_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(ImpNumber_isValid(caller));

	if(argc == 0){
		Runtime_throwString(runtime, "number:add requires at least one argument.");
		return NULL;
	}

	for(int i = 0; i < argc; i++){
		if(!ImpNumber_isValid(argv[i])){
			Runtime_throwString(runtime, "number:add accepts only numbers as arguments");
			return NULL;
		}
		ImpNumber_add(caller, argv[i]);
	}

	return NULL;
}


void ImpNumber_sub(Object *self, Object *other){
	assert(ImpNumber_isValid(self));
	assert(ImpNumber_isValid(other));
	double *data = ImpNumber_data(self);
	*data = (*data) - ImpNumber_getRaw(other);
}


static Object *ImpNumber_sub_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(ImpNumber_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "number:sub requires exactly one argument.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "number:sub argument must be of number type.");
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


static Object *ImpNumber_mult_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(ImpNumber_isValid(caller));

	if(argc == 0){
		Runtime_throwString(runtime, "number:mult requires at least one argument.");
		return NULL;
	}

	for(int i = 0; i < argc; i++){
		if(!ImpNumber_isValid(argv[i])){
			Runtime_throwString(runtime, "number:mult accepts only numbers as arguments");
			return NULL;
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


static Object *ImpNumber_div_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(ImpNumber_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "number:div requires exactly one argument.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "number:div argument must be of number type.");
	} else {
		ImpNumber_div(caller, argv[0]);
	}

	return NULL;
}


void ImpNumber_print(Object *self){
	assert(ImpNumber_isValid(self));
	printf("%f", ImpNumber_getRaw(self));
}


static Object *ImpNumber_print_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(ImpNumber_isValid(caller));
	ImpNumber_print(caller);
	return NULL;
}


void ImpNumber_set(Object *self, Object *value){
	assert(ImpNumber_isValid(self));
	assert(ImpNumber_isValid(value));
	ImpNumber_setRaw(self, ImpNumber_getRaw(value));
}


static Object *ImpNumber_set_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(caller);

	if(argc != 1){
		Runtime_throwString(runtime, "number:set requires exactly one argument.");
	} else if(!ImpNumber_isValid(argv[0])){
		Runtime_throwString(runtime, "number:set requires an argument of type number");
	} else{
		ImpNumber_set(caller, argv[0]);
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


static Object *ImpNumber_clone_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	assert(runtime);
	assert(caller);
	assert(argc == 0);

	Object_reference(caller);

	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	void *data = malloc(sizeof(double));
	if(!data){
		abort();
	}
	Object_putDataShallow(r, "__data", data);
	ImpNumber_setRaw(r, ImpNumber_getRaw(caller));

	Object_unreference(caller);
	return r;
}


void ImpNumber_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_NUMBER);

	Object_registerCMethod(self, "__add", ImpNumber_add_internal);
	Object_registerCMethod(self, "__sub", ImpNumber_sub_internal);
	Object_registerCMethod(self, "__mult", ImpNumber_mult_internal);
	Object_registerCMethod(self, "__div", ImpNumber_div_internal);

	Object_registerCMethod(self, "__print", ImpNumber_print_internal);
	Object_registerCMethod(self, "__set", ImpNumber_set_internal);

	Object_registerCMethod(self, "__clone", ImpNumber_clone_internal);
	ImpNumber_setRaw(self, 0);
}
