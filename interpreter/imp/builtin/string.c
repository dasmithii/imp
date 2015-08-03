#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "general.h"
#include "number.h"
#include "string.h"




bool iString_isValid(iObject *self){
	return iObject_isValid(self) &&
	       iBuiltin_id(self) == iBUILTIN_STRING;
}


char *iString_getRaw(iObject *self){
	assert(iString_isValid(self));
	return (char*) iObject_getDataDeep(self, "__data");
}


void iString_setRawPointer(iObject *self, char *text){
	assert(iString_isValid(self));
	assert(text);
	iObject_putDataDeep(self, "__data", text);
	iObject *size = iObject_getDeep(self, "size");
	iNumber_setRaw(size, (double) strlen(text));
}

void iString_setRaw(iObject *self, char *text){
	iString_setRawPointer(self, strdup(text));
}


void iString_print(iObject *self){
	assert(iString_isValid(self));
	printf("%s", iString_getRaw(self));
}


static iObject *print_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iString_isValid(caller));

	if(argc > 0){
		iRuntime_throwString(runtime, context, "String:print does not accept arguments.");
	} else {
		iString_print(caller);
	}

	return NULL;	
}


static iObject *clone_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iString_isValid(caller));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "String:~ does not accept arguments.");
		return NULL;
	}

	iObject *r = iRuntime_simpleClone(runtime, caller);
	iObject_reference(r);
	char *callerRaw = iString_getRaw(caller);
	if(!callerRaw){
		callerRaw = "";
	}
	iObject_putDataShallow(r, "__data", strdup(callerRaw)); // TODO: check pointer from strdup
	iObject *size = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(size, 0);
	iObject_putShallow(r, "size", size);
	iObject_unreference(r);
	return r;	
}


void iString_concatenateRaw(iObject *self, char *s2){
	char *s1 = iString_getRaw(self);
	char *s3 = malloc((1 + strlen(s1) + strlen(s2)) * sizeof(char));
	if(!s3){
		abort();
	}
	strcpy(s3, s1);
	strcat(s3, s2);
	iString_setRaw(self, s3);
	free(s3);
}


iObject *concatenate_(iRuntime *runtime
	                , iObject *context
	                , iObject *caller
	                , int argc
	                , iObject **argv){
	assert(runtime);
	assert(iString_isValid(caller));

	if(argc != 1){
		iRuntime_throwString(runtime, context, "String:concatenate requires exactly one argument");
		return NULL;
	}

	iObject *ro = NULL;
	if(iBuiltin_id(argv[0]) == iBUILTIN_STRING){
		ro = argv[0];
	} else if(iObject_hasMethod(argv[0], "asString")){
		ro = iRuntime_callMethod(runtime
			                  , context
			                  , argv[0]
			                  , "asString"
			                  , 0
			                  , NULL);
		if(iBuiltin_id(ro) != iBUILTIN_STRING){
			iRuntime_throwString(runtime, context, ":asString did not return string");
		}
	} else {
		iRuntime_throwString(runtime, context, "String:concatenate requires stringifiable argument");
	}
	iString_concatenateRaw(caller, iString_getRaw(ro));
	return NULL;
}


static iObject *asBoolean_(iRuntime *runtime
	                     , iObject *context
	                     , iObject *caller
	                     , int argc
	                     , iObject **argv){
	assert(runtime);
	assert(iString_isValid(caller));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "String:? does not accept arguments.");
		return NULL;
	}

	iObject *r = iRuntime_cloneField(runtime, "Number");
	if(*iString_getRaw(caller) == 0){
		iNumber_setRaw(r, 0);
	} else {
		iNumber_setRaw(r, 1);
	}
	return r;	
}


static iObject *hashCode_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *self
	                    , int argc
	                    , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "String:hashCode does not accept arguments");
	}

	iObject *r = iRuntime_cloneField(runtime, "Number");

	// some prime numbers
	const uint64_t p1 = 54059; //
	const uint64_t p2 = 13;
	const uint64_t p3 = 961748941;
	uint64_t hash = 86969;

	// raw string
	const char *str = iString_getRaw(self);

	while (*str) {
		hash = (hash * p1) ^ (str[0] * p2);
		str++;
	}

	hash = hash % p3;

	iNumber_setRaw(r, *((double*) &hash));
	return r;
} 

static iObject *compare_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *self
	                   , int argc
	                   , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "String:<> requires one argument");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "String:<> requires string argument");
	}

	iObject *r = iRuntime_cloneField(runtime, "Number");
	iNumber_setRaw(r, (double) strcmp(iString_getRaw(self),
		                                iString_getRaw(argv[0])));
	return r;
} 


static iObject *value_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "String:$ does not accept arguments");
	}

	iObject *r = iRuntime_MAKE(runtime, String);
	iString_setRaw(r, iString_getRaw(self));
	return r;
} 


static iObject *duplicate_(iRuntime *runtime
	                     , iObject *context
	                     , iObject *self
	                     , int argc
	                     , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "String:*= requires exactly 1 argument");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "String:*= requires numeric argument");
	}

	int coef = iNumber_getRawRounded(argv[0]);
	char *raw = iString_getRaw(self);
	const size_t rawLen = strlen(raw);

	if(coef == 0){
		iString_setRaw(self, "");
		return NULL;
	} else if(coef < 0){
		for(size_t i = 0; i < rawLen/2; i++){
			const char tmp = raw[rawLen - 1 - i];
			raw[rawLen - 1 - i] = raw[i];
			raw[i] = tmp;
		}
		coef *= -1;
	}

	char *newRaw = malloc(coef * rawLen + 1);
	if(!newRaw){
		abort();
	}
	newRaw[coef * rawLen] = 0;
	for(int i = 0; i < coef; i++){
		memcpy(newRaw + i * rawLen, raw, rawLen);
	}
	iString_setRawPointer(self, newRaw);

	return NULL;
}


void iString_init(iObject *self, iRuntime *runtime){
	assert(self);
	iBuiltin_setId(self, iBUILTIN_STRING);

	iRuntime_registerCMethod(runtime, self, "print", print_);
	iRuntime_registerCMethod(runtime, self, "asString", value_);
	iRuntime_registerCMethod(runtime, self, "$", value_);
	iRuntime_registerCMethod(runtime, self, "~", clone_);
	iRuntime_registerCMethod(runtime, self, "+=", concatenate_);
	iRuntime_registerCMethod(runtime, self, "*=", duplicate_);
	iRuntime_registerCMethod(runtime, self, "?", asBoolean_);

	iRuntime_registerCMethod(runtime, self, "<>", compare_);

	iRuntime_registerCMethod(runtime, self, "_hashCode", hashCode_);
}
