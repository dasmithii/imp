#include "number.h"
#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "general.h"


static double *ImpNumber_data(Object *self){
	return (double*) Object_getDataDeep(self, "__data");
}


void ImpNumber_add(Object *self, Object *other){
	double *data = ImpNumber_data(self);
	*data = (*data) + ImpNumber_getRaw(other);
}

static Object *ImpNumber_add_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	// TODO: check arguments
	ImpNumber_add(caller, argv[0]);
	return caller;
}


void ImpNumber_sub(Object *self, Object *other){
	double *data = ImpNumber_data(self);
	*data = (*data) - ImpNumber_getRaw(other);
}

static Object *ImpNumber_sub_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	// TODO: check arguments
	ImpNumber_sub(caller, argv[0]);
	return caller;
}


void ImpNumber_mult(Object *self, Object *other){
	double *data = ImpNumber_data(self);
	*data = (*data) * ImpNumber_getRaw(other);
}

static Object *ImpNumber_mult_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	// TODO: check arguments
	ImpNumber_mult(caller, argv[0]);
	return caller;
}


void ImpNumber_div(Object *self, Object *other){
	double *data = ImpNumber_data(self);
	*data = (*data) / ImpNumber_getRaw(other);
}

static Object *ImpNumber_div_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	// TODO: check arguments
	ImpNumber_div(caller, argv[0]);
	return caller;
}

void ImpNumber_print(Object *self){
	printf("%f", ImpNumber_getRaw(self));
}


static Object *ImpNumber_print_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	// TODO: check arguments
	ImpNumber_print(caller);
	return caller;
}


void ImpNumber_set(Object *self, Object *value){
	ImpNumber_setRaw(self, ImpNumber_getRaw(value));
}

static Object *ImpNumber_set_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	// TODO: check arguments
	ImpNumber_set(caller, argv[0]);
	return caller;
}


void ImpNumber_setRaw(Object *self, double value){
	double *existing = ImpNumber_data(self);
	if(existing){
		*existing = value;
		return;
	}
	void *data = malloc(sizeof(double));
	memcpy(data, &value, sizeof(double));
	Object_putDataDeep(self, "__data", data);
}


double ImpNumber_getRaw(Object *self){
	return *((double*) Object_getDataDeep(self, "__data"));
}




static Object *ImpNumber_clone_internal(Runtime *runtime
	                       , Object *context
	                       , Object *caller
	                       , int argc
	                       , Object **argv){
	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);

	Object_putDataShallow(r, "__data", malloc(sizeof(double)));
	ImpNumber_setRaw(r, ImpNumber_getRaw(caller));
	return r;
}

void ImpNumber_init(Object *self){
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
