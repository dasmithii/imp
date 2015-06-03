#include <stdio.h>

#include "base.h"
#include "number.h"
#include "string.h"
#include "vector.h"
#include "general.h"
#include "../toolbox/vector.h"

static Object *ImpBase_hasKeyShallow_(Runtime *runtime
	                                        , Object *context
	                                        , Object *caller
	                                        , int argc
	                                        , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Object:hasKeyShallow requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:hasKeyShallow requires a String as its argument" );
	}
	bool rawr = Object_hasKeyShallow(caller, ImpString_getRaw(argv[0]));
	Object *r = Runtime_cloneField(runtime, "Number");
	if(rawr){
		ImpNumber_setRaw(r, 1);
	} else {
		ImpNumber_setRaw(r, 0);
	}
	return r;
}


static Object *ImpBase_hasKeyDeep_(Runtime *runtime
	                                      , Object *context
	                                      , Object *caller
	                                      , int argc
	                                      , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Object:hasKeyDeep requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:hasKeyDeep requires a String as its argument" );
	}
	bool rawr = Object_hasKeyDeep(caller, ImpString_getRaw(argv[0]));
	Object *r = Runtime_cloneField(runtime, "Number");
	if(rawr){
		ImpNumber_setRaw(r, 1);
	} else {
		ImpNumber_setRaw(r, 0);
	}
	return r;
}

static Object *ImpBase_removeKeyShallow_(Runtime *runtime
	                                        , Object *context
	                                        , Object *caller
	                                        , int argc
	                                        , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Object:removeKeyShallow requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:removeKeyShallow requires a String as its argument" );
	}
	Object_remShallow(caller, ImpString_getRaw(argv[0]));
	return NULL;
}


static Object *ImpBase_getDeep_(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Object:getDeep requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:getDeep requires a String as its argument" );
	}
	char *raw = ImpString_getRaw(argv[0]);
	if(!Object_hasKeyDeep(caller, raw)){
		Runtime_throwFormatted(runtime, "Object:getDeep - field does not exist %s", raw);
	}
	return Object_getDeep(caller, raw);
}



static Object *ImpBase_getShallow_(Runtime *runtime
	                                     , Object *context
	                                     , Object *caller
	                                     , int argc
	                                     , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Object:getShallow requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:getShallow requires a String as its argument" );
	}
	char *raw = ImpString_getRaw(argv[0]);
	if(!Object_hasKeyShallow(caller, raw)){
		Runtime_throwFormatted(runtime, "Object:getShallow - field does not exist %s", raw);
	}
	return Object_getShallow(caller, raw);
}



static Object *ImpBase_putDeep_(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "Object:putDeep requires exactly 2 arguments" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:putDeep requires a String as its first argument" );
	}
	Object_putDeep(caller, ImpString_getRaw(argv[0]), argv[1]);
	return NULL;
}



static Object *ImpBase_putShallow_(Runtime *runtime
	                                     , Object *context
	                                     , Object *caller
	                                     , int argc
	                                     , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "Object:putShallow requires exactly 2 arguments" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:putShallow requires a String as its first argument" );
	}
	Object_putShallow(caller, ImpString_getRaw(argv[0]), argv[1]);
	return NULL;
}


static Object *ImpBase_slotNames_(Runtime *runtime
	                            , Object *context
	                            , Object *caller
	                            , int argc
	                            , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Object:slotNames does not accept arguments");
	}
	Object *r = Runtime_cloneField(runtime, "Vector");
	Object_reference(r);
	Vector *v = ImpVector_getRaw(r);
	for(int i = 0; i < caller->slotCount; i++){
		Object *item = Runtime_cloneField(runtime, "String");
		ImpString_setRaw(item, caller->slots[i].key);
		Vector_append(v, &item);
	}
	Object_unreference(r);
	return r;
}


static Object *ImpBase_slotCount_(Runtime *runtime
	                            , Object *context
	                            , Object *caller
	                            , int argc
	                            , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Object:slotCount does not accept arguments");
	}
	Object *r = Runtime_cloneField(runtime, "Number");
	ImpNumber_setRaw(r, (double) caller->slotCount);
	return r;
}


static Object *ImpBase_asString_(Runtime *runtime
	                           , Object *context
	                           , Object *caller
	                           , int argc
	                           , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Object:asString does not accept arguments");
	}
	char temp[128];
	sprintf(temp, "Object[%p]", caller);
	Object *r = Runtime_cloneField(runtime, "String");	
	ImpString_setRaw(r, temp);
	return r;
}

static Object *ImpBase_hasMethod_(Runtime *runtime
	                            , Object *context
	                            , Object *caller
	                            , int argc
	                            , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Object:hasMethod requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:hasMethod requires its argument to be a string");
	}
	bool rawr = Object_hasMethod(caller, ImpString_getRaw(argv[0]));
	Object *r = Runtime_cloneField(runtime, "Number");
	if(rawr){
		ImpNumber_setRaw(r, 1);
	} else {
		ImpNumber_setRaw(r, 0);
	}
	return r;
}

static Object *ImpBase_hasSpecialMethod_(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Object:hasSpecialMethod requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:hasSpecialMethod requires its argument to be a string");
	}
	bool rawr = Object_hasSpecialMethod(caller, ImpString_getRaw(argv[0]));
	Object *r = Runtime_cloneField(runtime, "Number");
	if(rawr){
		ImpNumber_setRaw(r, 1);
	} else {
		ImpNumber_setRaw(r, 0);
	}
	return r;
}


static Object *ImpBase_callMethod_(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	if(argc == 0){
		Runtime_throwString(runtime, "Object:callMethod requires arguments");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:callMethod requires its argument to be a string");
	}
	char *methodName = ImpString_getRaw(argv[0]);
	if(!Object_hasMethod(caller, methodName)){
		Runtime_throwFormatted(runtime, "Object:callMethod could not find method: '%s'", methodName);
	}
	return Runtime_callMethod(runtime
		                    , context
		                    , caller
		                    , methodName
		                    , argc - 1
		                    , argv + 1);
}


static Object *ImpBase_callSpecialMethod_(Runtime *runtime
	                                   , Object *context
	                                   , Object *caller
	                                   , int argc
	                                   , Object **argv){
	if(argc == 0){
		Runtime_throwString(runtime, "Object:callSpecialMethod requires arguments");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "Object:callSpecialMethod requires its argument to be a string");
	}
	char *methodName = ImpString_getRaw(argv[0]);
	if(!Object_hasMethod(caller, methodName)){
		Runtime_throwFormatted(runtime, "Object:callSpecialMethod could not find method: '%s'", methodName);
	}
	return Runtime_callSpecialMethod(runtime
		                           , context
		                           , caller
		                           , methodName
		                           , argc - 1
		                           , argv + 1);
}       


void ImpBase_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_OBJECT);
	Object_registerCMethod(self, "__hasMethod", ImpBase_hasMethod_);
	Object_registerCMethod(self, "__hasSpecialMethod", ImpBase_hasSpecialMethod_);
	Object_registerCMethod(self, "__callMethod", ImpBase_callMethod_);
	Object_registerCMethod(self, "__callSpecialMethod", ImpBase_callSpecialMethod_);
	Object_registerCMethod(self, "__hasMethod", ImpBase_hasMethod_);
	Object_registerCMethod(self, "__hasKeyShallow", ImpBase_hasKeyShallow_);
	Object_registerCMethod(self, "__kasKeyDeep", ImpBase_hasKeyDeep_);
	Object_registerCMethod(self, "__removeKeyShallow", ImpBase_removeKeyShallow_);
	Object_registerCMethod(self, "__getDeep", ImpBase_getDeep_);
	Object_registerCMethod(self, "__getShallow", ImpBase_getShallow_);
	Object_registerCMethod(self, "__putDeep", ImpBase_putDeep_);
	Object_registerCMethod(self, "__putShallow", ImpBase_putShallow_);
	Object_registerCMethod(self, "__slotNames", ImpBase_slotNames_);
	Object_registerCMethod(self, "__slotCount", ImpBase_slotCount_);
	Object_registerCMethod(self, "__asString", ImpBase_asString_);
}