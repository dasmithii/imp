#include <stdio.h>
#include <string.h>

#include "general.h"
#include "number.h"
#include "string.h"




bool ImpString_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_STRING;
}


char *ImpString_getRaw(Object *self){
	assert(ImpString_isValid(self));
	return (char*) Object_getDataDeep(self, "__data");
}


void ImpString_setRaw(Object *self, char *text){
	assert(ImpString_isValid(self));
	assert(text);
	Object_putDataDeep(self, "__data", strdup(text));
}


void ImpString_print(Object *self){
	assert(ImpString_isValid(self));
	printf("%s", ImpString_getRaw(self));
}


static Object *ImpString_print_(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(ImpString_isValid(caller));

	if(argc > 0){
		Runtime_throwString(runtime, "String:print does not accept arguments.");
	} else {
		ImpString_print(caller);
	}

	return NULL;	
}


static Object *ImpString_clone_(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	assert(runtime);
	assert(ImpString_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "String:~ does not accept arguments.");
		return NULL;
	}

	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "_prototype", caller);
	Object_putDataShallow(r, "__data", strdup(ImpString_getRaw(caller))); // TODO: check pointer from strdup

	return r;	
}


void ImpString_concatenateRaw(Object *self, char *s2){
	char *s1 = ImpString_getRaw(self);
	char *s3 = malloc((1 + strlen(s1) + strlen(s2)) * sizeof(char));
	if(!s3){
		abort();
	}
	strcpy(s3, s1);
	strcat(s3, s2);
	ImpString_setRaw(self, s3);
	free(s3);
}


Object *ImpString_concatenate_(Runtime *runtime
	                         , Object *context
	                         , Object *caller
	                         , int argc
	                         , Object **argv){
	assert(runtime);
	assert(ImpString_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "String:concatenate requires exactly one argument");
		return NULL;
	}

	Object *ro = NULL;
	if(BuiltIn_id(argv[0]) == BUILTIN_STRING){
		ro = argv[0];
	} else if(Object_hasMethod(argv[0], "asString")){
		ro = Runtime_callMethod(runtime
			                  , context
			                  , argv[0]
			                  , "asString"
			                  , 0
			                  , NULL);
		if(BuiltIn_id(ro) != BUILTIN_STRING){
			Runtime_throwString(runtime, ":asString did not return string");
		}
	} else {
		Runtime_throwString(runtime, "String:concatenate requires stringifiable argument");
	}
	ImpString_concatenateRaw(caller, ImpString_getRaw(ro));
	return NULL;
}


static Object *ImpString_asBoolean_(Runtime *runtime
	                              , Object *context
	                              , Object *caller
	                              , int argc
	                              , Object **argv){
	assert(runtime);
	assert(ImpString_isValid(caller));

	if(argc != 0){
		Runtime_throwString(runtime, "String:? does not accept arguments.");
		return NULL;
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	if(*ImpString_getRaw(caller) == 0){
		ImpNumber_setRaw(r, 0);
	} else {
		ImpNumber_setRaw(r, 1);
	}
	return r;	
}


static Object *ImpString_hashCode_(Runtime *runtime
	                           , Object *context
	                           , Object *self
	                           , int argc
	                           , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:hashCode does not accept arguments");
	}

	Object *r = Runtime_cloneField(runtime, "Number");

	// some prime numbers
	const unsigned long p1 = 54059; //
	const unsigned long p2 = 13;
	const unsigned long p3 = 961748941;
	unsigned long hash = 86969;

	// raw string
	const char *str = ImpString_getRaw(self);
	const int len = strlen(str);

	while (*str) {
		hash = (hash * p1) ^ (str[0] * p2);
		str++;
	}

	ImpNumber_setRaw(r, (double) (hash % p3));
	return r;
} 


void ImpString_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_STRING);

	Object_registerCMethod(self, "__print", ImpString_print_);
	Object_registerCMethod(self, "__~", ImpString_clone_);
	Object_registerCMethod(self, "__concatenate", ImpString_concatenate_);
	Object_registerCMethod(self, "__?", ImpString_asBoolean_);

	Object_registerCMethod(self, "__hashCode", ImpString_hashCode_);

	ImpString_setRaw(self, "");
}