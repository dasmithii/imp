#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../c.h"
#include "base.h"
#include "route.h"
#include "number.h"
#include "string.h"
#include "general.h"



 
static Object *hasKeyShallow_(Runtime *runtime
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


static Object *hasKeyDeep_(Runtime *runtime
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

static Object *removeKeyShallow_(Runtime *runtime
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


static Object *getDeep_(Runtime *runtime
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



static Object *getShallow_(Runtime *runtime
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



static Object *putDeep_(Runtime *runtime
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



static Object *putShallow_(Runtime *runtime
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


static Object *slotNames_(Runtime *runtime
	                    , Object *context
	                    , Object *caller
	                    , int argc
	                    , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:slotNames does not accept arguments");
	}

	Object *size = Runtime_make(runtime, Number);
	ImpNumber_setRaw(size, caller->slotCount);

	Object *r = Runtime_callMethod(runtime
		                         , context
		                         , runtime->Array
		                         , "withSize"
		                         , 1
		                         , &size);

	Object_reference(r);

	Object *index = Runtime_make(runtime, Number);
	ImpNumber_setRaw(index, 0);
	Object_reference(index);

	for(int i = 0; i < caller->slotCount; i++){
		Object *args[2];
		args[0] = index;
		args[1] = Runtime_make(runtime, String);
		ImpString_setRaw(args[1], caller->slots[i].key);
		Runtime_callMethod(runtime
			             , context
			             , r
			             , "at="
			             , 2
			             , args);
		ImpNumber_setRaw(index, 1 + ImpNumber_getRaw(index));
	}

	Object_reference(index);
	Object_unreference(r);
	return r;
}


static Object *asString_(Runtime *runtime
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

static Object *hasMethod_(Runtime *runtime
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


static Object *callMethod_(Runtime *runtime
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


static Object *above_(Runtime *runtime
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

static Object *aboveEq_(Runtime *runtime
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

static Object *below_(Runtime *runtime
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

static Object *belowEq_(Runtime *runtime
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

static Object *not_(Runtime *runtime
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

static Object *clone_(Runtime *runtime
	                        , Object *context
	                        , Object *self
	                        , int argc
	                        , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "#:~ accepts no arguments");
	}

	return Runtime_simpleClone(runtime, self);
} 



static Object *copyAndDo(Runtime *runtime
	                   , Object *context
	                   , Object *lo
	                   , char *op
	                   , int argc
	                   , Object **argv){
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
	Runtime_callMethod(runtime, context, cp, op, argc, argv);
	Object_unreference(cp);
	return cp;
}


static Object *plus_(Runtime *runtime
	                       , Object *context
	                       , Object *self
	                       , int argc
	                       , Object **argv){
	return copyAndDo(runtime, context, self, "+=", argc, argv);
} 

static Object *minus_(Runtime *runtime
	                        , Object *context
	                        , Object *self
	                        , int argc
	                        , Object **argv){
	return copyAndDo(runtime, context, self, "-=", argc, argv);
} 

static Object *div_(Runtime *runtime
	                      , Object *context
	                      , Object *self
	                      , int argc
	                      , Object **argv){
	return copyAndDo(runtime, context, self, "/=", argc, argv);
} 

static Object *times_(Runtime *runtime
	                      , Object *context
	                      , Object *self
	                      , int argc
	                      , Object **argv){
	return copyAndDo(runtime, context, self, "*=", argc, argv);
} 

static Object *mod_(Runtime *runtime
	              , Object *context
	              , Object *self
	              , int argc
	              , Object **argv){
	return copyAndDo(runtime, context, self, "%=", argc, argv);
} 


static Object *exp_(Runtime *runtime
	              , Object *context
	              , Object *self
	              , int argc
	              , Object **argv){
	return copyAndDo(runtime, context, self, "^=", argc, argv);
} 


static Object *value_(Runtime *runtime
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

static Object *equals_(Runtime *runtime
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

static Object *notEq_(Runtime *runtime
	                , Object *context
	                , Object *self
	                , int argc
	                , Object **argv){
	Object *equals = Runtime_callMethod(runtime
		                              , context
		                              , self
		                              , "=="
		                              , argc
		                              , argv);
	return Runtime_callMethod(runtime
		                    , context
		                    , equals
		                    , "!"
		                    , 0
		                    , NULL);
} 


static Object *is_(Runtime *runtime
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

static Object *asBoolean_(Runtime *runtime
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
static Object *hashCode_(Runtime *runtime
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


static Object *def_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(argv[0]));
	assert(Object_isValid(argv[1]));

	Object *route = argv[0];
	Object *value = unroute(argv[1]);
	if(!value){
		if(BuiltIn_id(argv[1]) == BUILTIN_ROUTE){
			Runtime_throwFormatted(runtime, "variable does not exist '%s'", ImpRoute_getRaw(argv[1]));
		} else {
			Runtime_throwFormatted(runtime, "cannot assign '%s' to NULL", ImpRoute_getRaw(argv[0]));
		}
		return NULL;
	}
	Object_reference(value);

	if(argc != 2){
		Runtime_throwString(runtime, "def accepts exactly 2 arguments.");
	} else if(!ImpRoute_isValid(route)){
		Runtime_throwString(runtime, "def requires a route as first argument.");
	}

	// TODO: use ImpRoute_submapping here

	Object *par = context;
	int rargc = ImpRoute_argc(route);

	for(int i = 0; i < rargc - 1; i++){
		char buf[64];
		ImpRoute_argv(route, i, buf);
		Object *ppar = par;
		par = Object_getDeep(par, buf);
		if(!par){
			par = Runtime_make(runtime, Object);
			Object_putShallow(ppar, buf, par);
		}
	}
	char fbuf[64];
	ImpRoute_argv(route, rargc - 1, fbuf);
	if(strcmp(fbuf, "#") != 0 && Object_hasKeyShallow(par, fbuf)){
		Runtime_throwFormatted(runtime, "def would overwrite value '%s'", fbuf);
	}
	Object_putShallow(par, fbuf, value);

	Object_unreference(value);
	return NULL;
}


static Object *set_(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc != 2){
		Runtime_throwString(runtime, "set requires exactly 2 arguments.");
		return NULL;
	} else if(!ImpRoute_isValid(argv[0])){
		Runtime_throwString(runtime, "set requires a route for argument 1.");
		return NULL;
	}

	Object *route = argv[0];
	Object *value = unroute(argv[1]);
	if(!value){
		if(BuiltIn_id(argv[1]) == BUILTIN_ROUTE){
			Runtime_throwFormatted(runtime, "variable does not exist '%s'", ImpRoute_getRaw(argv[1]));
		} else {
			Runtime_throwFormatted(runtime, "cannot assign '%s' to NULL", ImpRoute_getRaw(argv[0]));
		}
		return NULL;
	}

	Object *submapping = ImpRoute_submapping(route);
	char field[32];
	ImpRoute_argv(route, ImpRoute_argc(route) - 1, field);
	
	if(submapping && Object_hasKeyDeep(submapping, field)){
		Object_putDeep(submapping, field, value);
		return NULL;
	}

	Runtime_throwFormatted(runtime, "set failed on '%s'; try def.", ImpRoute_getRaw(route));
	return NULL;
}


static Object *markRecursively_(Runtime *runtime
	               , Object *context
	               , Object *self
	               , int argc
	               , Object **argv){
	if(!self || self->gc_mark){
		return NULL;
	}

	// mark this object
	self->gc_mark = true;

	// mark all non-primitive fields recursively
	for(int i = 0; i < self->slotCount; i++){
		if(!Slot_isPrimitive(self->slots + i)){
			markRecursively_(runtime
				           , context
				           , Slot_object(self->slots + i)
				           , 0
				           , NULL);
		}
	}

	// if object has special internals to mark
	if(Object_hasMethod(self, "_markInternalsRecursively")){
		Runtime_callMethod(runtime, context, self, "_markInternalsRecursively", 0, NULL);
	}
	return NULL;
}


void ImpBase_init(Object *self, Runtime *runtime){
	assert(self);
	BuiltIn_setId(self, BUILTIN_OBJECT);
	Runtime_registerCMethod(runtime, self, "hasMethod", hasMethod_);
	Runtime_registerCMethod(runtime, self, "callMethod", callMethod_);
	Runtime_registerCMethod(runtime, self, "hasKeyShallow", hasKeyShallow_);
	Runtime_registerCMethod(runtime, self, "hasKeyDeep", hasKeyDeep_);
	Runtime_registerCMethod(runtime, self, "removeKeyShallow", removeKeyShallow_);
	Runtime_registerCMethod(runtime, self, "getDeep", getDeep_);
	Runtime_registerCMethod(runtime, self, "getShallow", getShallow_);
	Runtime_registerCMethod(runtime, self, "putDeep", putDeep_);
	Runtime_registerCMethod(runtime, self, "putShallow", putShallow_);
	Runtime_registerCMethod(runtime, self, "slotNames", slotNames_);
	Runtime_registerCMethod(runtime, self, "asString", asString_);

	Runtime_registerCMethod(runtime, self, "?", asBoolean_);

	Runtime_registerCMethod(runtime, self, ">", above_);
	Runtime_registerCMethod(runtime, self, ">=", aboveEq_);
	Runtime_registerCMethod(runtime, self, "<", below_);
	Runtime_registerCMethod(runtime, self, "<=", belowEq_);
	Runtime_registerCMethod(runtime, self, "!", not_);
	Runtime_registerCMethod(runtime, self, "~", clone_);
	Runtime_registerCMethod(runtime, self, "+", plus_);
	Runtime_registerCMethod(runtime, self, "-", minus_);
	Runtime_registerCMethod(runtime, self, "*", times_);
	Runtime_registerCMethod(runtime, self, "/", div_);
	Runtime_registerCMethod(runtime, self, "%", mod_);
	Runtime_registerCMethod(runtime, self, "^", exp_);
	Runtime_registerCMethod(runtime, self, "$", value_);
	Runtime_registerCMethod(runtime, self, "==", equals_);
	Runtime_registerCMethod(runtime, self, "!=", notEq_);
	Runtime_registerCMethod(runtime, self, "is", is_);

	Runtime_registerPrivelegedCMethod(runtime, self, "def", def_);
	Runtime_registerPrivelegedCMethod(runtime, self, "set", set_);

	Runtime_registerCMethod(runtime, self, "hashCode", hashCode_);

	Runtime_registerCMethod(runtime, self, "_markRecursively", markRecursively_);
}


