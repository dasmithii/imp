#include <stdio.h>
#include <string.h>

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
		Runtime_throwString(runtime, "#:hasKeyShallow requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:hasKeyShallow requires a String as its argument" );
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
		Runtime_throwString(runtime, "#:hasKeyDeep requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:hasKeyDeep requires a String as its argument" );
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
		Runtime_throwString(runtime, "#:removeKeyShallow requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:removeKeyShallow requires a String as its argument" );
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
		Runtime_throwString(runtime, "#:getDeep requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:getDeep requires a String as its argument" );
	}
	char *raw = ImpString_getRaw(argv[0]);
	if(!Object_hasKeyDeep(caller, raw)){
		Runtime_throwFormatted(runtime, "#:getDeep - field does not exist %s", raw);
	}
	return Object_getDeep(caller, raw);
}



static Object *ImpBase_getShallow_(Runtime *runtime
	                                     , Object *context
	                                     , Object *caller
	                                     , int argc
	                                     , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:getShallow requires exactly one argument" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:getShallow requires a String as its argument" );
	}
	char *raw = ImpString_getRaw(argv[0]);
	if(!Object_hasKeyShallow(caller, raw)){
		Runtime_throwFormatted(runtime, "#:getShallow - field does not exist %s", raw);
	}
	return Object_getShallow(caller, raw);
}



static Object *ImpBase_putDeep_(Runtime *runtime
	                                  , Object *context
	                                  , Object *caller
	                                  , int argc
	                                  , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "#:putDeep requires exactly 2 arguments" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:putDeep requires a String as its first argument" );
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
		Runtime_throwString(runtime, "#:putShallow requires exactly 2 arguments" );
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:putShallow requires a String as its first argument" );
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
		Runtime_throwString(runtime, "#:slotNames does not accept arguments");
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
		Runtime_throwString(runtime, "#:slotCount does not accept arguments");
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
		Runtime_throwString(runtime, "#:asString does not accept arguments");
	}
	char temp[128];
	sprintf(temp, "#[%p]", caller);
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
		Runtime_throwString(runtime, "#:hasMethod requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:hasMethod requires its argument to be a string");
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
		Runtime_throwString(runtime, "#:hasSpecialMethod requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:hasSpecialMethod requires its argument to be a string");
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
		Runtime_throwString(runtime, "#:callMethod requires arguments");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:callMethod requires its argument to be a string");
	}
	char *methodName = ImpString_getRaw(argv[0]);
	if(!Object_hasMethod(caller, methodName)){
		Runtime_throwFormatted(runtime, "#:callMethod could not find method: '%s'", methodName);
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
		Runtime_throwString(runtime, "#:callSpecialMethod requires arguments");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "#:callSpecialMethod requires its argument to be a string");
	}
	char *methodName = ImpString_getRaw(argv[0]);
	if(!Object_hasMethod(caller, methodName)){
		Runtime_throwFormatted(runtime, "#:callSpecialMethod could not find method: '%s'", methodName);
	}
	return Runtime_callSpecialMethod(runtime
		                           , context
		                           , caller
		                           , methodName
		                           , argc - 1
		                           , argv + 1);
} 


static Object *compare(Runtime *runtime
	                 , Object *context
	                 , Object *lo
                     , Object *ro){
	if(Object_hasMethod(lo, "<>")){
		return Runtime_callMethod(runtime
			                    , context
			                    , lo
			                    , "<>"
			                    , 1
			                    , &ro);
	} else if(Object_hasMethod(ro, "<>")){
		Object *c = Runtime_callMethod(runtime
			                         , context
			                         , ro
			                         , "<>"
			                         , 1
			                         , &lo);
		ImpNumber_setRaw(c, -1 * ImpNumber_getRaw(c));
		return c;
	} else {
		Runtime_throwString(runtime, "<> method not found");
	}
	return NULL;
}


static Object *ImpBase_above_(Runtime *runtime
	                        , Object *context
	                        , Object *self
	                        , int argc
	                        , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:> requires exactly 1 argument");
	}
	Object *c = compare(runtime, context, self, argv[0]);
	ImpNumber_setRaw(c, ImpNumber_getRaw(c) > 0? 1:0);
	return c;
} 

static Object *ImpBase_aboveEq_(Runtime *runtime
	                          , Object *context
	                          , Object *self
	                          , int argc
	                          , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:>= requires exactly 1 argument");
	}
	Object *c = compare(runtime, context, self, argv[0]);
	ImpNumber_setRaw(c, ImpNumber_getRaw(c) >= 0? 1:0);
	return c;
} 

static Object *ImpBase_below_(Runtime *runtime
	                        , Object *context
	                        , Object *self
	                        , int argc
	                        , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:< requires exactly 1 argument");
	}
	Object *c = compare(runtime, context, self, argv[0]);
	ImpNumber_setRaw(c, ImpNumber_getRaw(c) < 0? 1:0);
	return c;
} 

static Object *ImpBase_belowEq_(Runtime *runtime
	                          , Object *context
	                          , Object *self
	                          , int argc
	                          , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:<= requires exactly 1 argument");
	}
	Object *c = compare(runtime, context, self, argv[0]);
	ImpNumber_setRaw(c, ImpNumber_getRaw(c) <= 0? 1:0);
	return c;
} 

static Object *ImpBase_not_(Runtime *runtime
	                      , Object *context
	                      , Object *self
	                      , int argc
	                      , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:! accepts no arguments");
	}
	Object *b = Runtime_callMethod(runtime
		                         , context
		                         , self
		                         , "?"
		                         , 0
		                         , NULL);
	ImpNumber_setRaw(b, ImpNumber_getRaw(b)? 0:1);
	return b;
} 

static Object *ImpBase_clone_(Runtime *runtime
	                        , Object *context
	                        , Object *self
	                        , int argc
	                        , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:~ accepts no arguments");
	}

	if(Object_hasSpecialMethod(self, "clone")){
		return Runtime_callSpecialMethod(runtime
			                           , context
			                           , self
			                           , "clone"
			                           , 0
			                           , NULL);
	}

	Object *r = Runtime_rawObject(runtime);
	Object_putShallow(r, "#", self);
	return r;
} 



static Object *copyAndDo(Runtime *runtime
	                   , Object *context
	                   , Object *lo
	                   , Object *ro
	                   , char *op){
	if(!Object_hasMethod(lo, op)){
		Runtime_throwFormatted(runtime, "'%s' method not found", op);
	}

	Object *cp = Runtime_callMethod(runtime
		                          , context
		                          , lo
		                          , "$"
		                          , 0
		                          , NULL);
	if(!cp){
		Runtime_throwString(runtime, "copy returned NULL");
	}
	Object_reference(cp);
	Runtime_callMethod(runtime, context, cp, op, 1, &ro);
	Object_unreference(cp);
	return cp;
}


static Object *ImpBase_plus_(Runtime *runtime
	                       , Object *context
	                       , Object *self
	                       , int argc
	                       , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:+ requires exactly 1 arguments");
	}
	return copyAndDo(runtime, context, self, argv[0], "+=");
} 

static Object *ImpBase_minus_(Runtime *runtime
	                        , Object *context
	                        , Object *self
	                        , int argc
	                        , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:- requires exactly 1 arguments");
	}
	return copyAndDo(runtime, context, self, argv[0], "-=");
} 

static Object *ImpBase_div_(Runtime *runtime
	                      , Object *context
	                      , Object *self
	                      , int argc
	                      , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:/ requires exactly 1 arguments");
	}
	return copyAndDo(runtime, context, self, argv[0], "/=");
} 

static Object *ImpBase_times_(Runtime *runtime
	                      , Object *context
	                      , Object *self
	                      , int argc
	                      , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:* requires exactly 1 arguments");
	}
	return copyAndDo(runtime, context, self, argv[0], "*=");
} 

static Object *ImpBase_mod_(Runtime *runtime
	                      , Object *context
	                      , Object *self
	                      , int argc
	                      , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:% requires exactly 1 arguments");
	}
	return copyAndDo(runtime, context, self, argv[0], "%=");
} 

static Object *ImpBase_value_(Runtime *runtime
	                        , Object *context
	                        , Object *self
	                        , int argc
	                        , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "$ does not accept arguments");
	}

	// check if arg is able be copied using default methods
	for(int i = 0; i < self->slotCount; i++){
		Slot *s = self->slots + i;
		if(Slot_isPrimitive(s) && strcmp(s->key, "__referenceCount") != 0){
			Runtime_throwFormatted(runtime, "object not copy-able (contains primitive data at: '%s')", s->key);
		}
	}

	Object *r = Runtime_rawObject(runtime);
	Object_reference(r);
	for(int i = 0; i < self->slotCount; i++){
		Slot *s = self->slots + i;
		Object *o = Slot_object(s);
		if(strcmp(s->key, "_data") == 0){
			Object_putShallow(r, "_data", Runtime_callMethod(runtime
				                              , context
				                              , o
				                              , "$"
				                              , 0
				                              , NULL));
		} else {
			Object_putShallow(r, s->key, o);
		}
	}
	Object_unreference(r);
	return r;
} 

static bool slotsAreEqual(Runtime *runtime
	                    , Object *context
	                    , Slot *s0
	                    , Slot *s1){
	if(s0 == s1){
		return true;
	}

	if(Slot_isPrimitive(s0)){
		if(!Slot_isPrimitive(s1)){
			return false;
		}
		return Slot_data(s0) == Slot_data(s1);
	} else {
		Object *arg = Slot_object(s1);
		return Runtime_callMethod(runtime
			                    , context
			                    , Slot_object(s0)
			                    , "=="
			                    , 1
			                    , &arg);
	}
}


static bool objectsAreEqual(Runtime *runtime
	                      , Object *context
	                      , Object *o0
	                      , Object *o1){
	if(o0 == o1){
		return true;
	}

	if(Object_hasMethod(o0, "<>")){
		Object *n = Runtime_callMethod(runtime, context, o0, "<>", 1, &o1);
		return ImpNumber_getRaw(n) == 0;
	}

	if(!o0 || !o1){
		return false;
	}
	if(o0->slotCount != o1->slotCount){
		return false;
	}
	for(int i = 0; i < o0->slotCount; i++){
		if(!slotsAreEqual(runtime
			            , context
			            , o0->slots + i
			            , o1->slots + i)){
			return false;
		}
	}
	return true;
}

static Object *ImpBase_equals_(Runtime *runtime
	                         , Object *context
	                         , Object *self
	                         , int argc
	                         , Object **argv){
	if(argc < 1){
		Runtime_throwString(runtime, "#:== requires arguments");
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	ImpNumber_setRaw(r, 1);
	Object_reference(r);
	for(int i = 0; i < argc; i++){
		if(!objectsAreEqual(runtime
			              , context
			              , self
			              , argv[i])){
			ImpNumber_setRaw(r, 0);
			break;
		}
	}
	Object_unreference(r);
	return r;
} 

static Object *ImpBase_is_(Runtime *runtime
	                     , Object *context
	                     , Object *caller
	                     , int argc
	                     , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "#:is requires exactly one argument");
	}
	Object *r = Runtime_cloneField(runtime, "Number");
	ImpNumber_setRaw(r, caller == argv[0]? 1:0);
	return r;
} 

static Object *ImpBase_asBoolean_(Runtime *runtime
	                            , Object *context
	                            , Object *caller
	                            , int argc
	                            , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:? does not accept arguments");
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	Object_reference(r);

	if(Object_hasMethod(caller, "asNumber")){
		Object *n = Runtime_callMethod(runtime
			                         , context
			                         , caller
			                         , "asNumber"
			                         , 0
			                         , NULL);
		ImpNumber_setRaw(r, ImpNumber_getRaw(n)? 1:0);
	} else {
		ImpNumber_setRaw(r, 1); // default to 1 on non-NULL objects
	}

	Object_unreference(r);
	return r;
} 



// Default hashCode implementation: returns a non-zero
// deterministic value based on address in memory.
static Object *ImpBase_hashCode_(Runtime *runtime
	                           , Object *context
	                           , Object *self
	                           , int argc
	                           , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:hashCode does not accept arguments");
	}

	Object *r = Runtime_cloneField(runtime, "Number");
	const unsigned long constant = 2147483647;
	const unsigned long address  = (unsigned long) &self;
	ImpNumber_setRaw(r, (double) (constant ^ address));
	return r;
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

	Object_registerCMethod(self, "__?", ImpBase_asBoolean_);

	Object_registerCMethod(self, "__>", ImpBase_above_);
	Object_registerCMethod(self, "__>=", ImpBase_aboveEq_);
	Object_registerCMethod(self, "__<", ImpBase_below_);
	Object_registerCMethod(self, "__<=", ImpBase_belowEq_);
	Object_registerCMethod(self, "__!", ImpBase_not_);
	Object_registerCMethod(self, "__~", ImpBase_clone_);
	Object_registerCMethod(self, "__+", ImpBase_plus_);
	Object_registerCMethod(self, "__-", ImpBase_minus_);
	Object_registerCMethod(self, "__*", ImpBase_times_);
	Object_registerCMethod(self, "__/", ImpBase_div_);
	Object_registerCMethod(self, "__%", ImpBase_mod_);
	Object_registerCMethod(self, "__$", ImpBase_value_);
	Object_registerCMethod(self, "__==", ImpBase_equals_);
	Object_registerCMethod(self, "__is", ImpBase_is_);

	Object_registerCMethod(self, "__hashCode", ImpBase_hashCode_);
}