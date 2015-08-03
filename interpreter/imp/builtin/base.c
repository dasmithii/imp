#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../c.h"
#include "base.h"
#include "route.h"
#include "number.h"
#include "string.h"
#include "general.h"



 
static iObject *hasKeyShallow_(iRuntime *runtime
	                         , iObject *context
	                         , iObject *caller
	                         , int argc
	                         , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:hasKeyShallow requires exactly one argument" );
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:hasKeyShallow requires a String as its argument" );
	}
	bool rawr = iObject_hasKeyShallow(caller, iString_getRaw(argv[0]));
	iObject *r = iRuntime_cloneField(runtime, "Number");
	if(rawr){
		iNumber_setRaw(r, 1);
	} else {
		iNumber_setRaw(r, 0);
	}
	return r;
}


static iObject *hasKeyDeep_(iRuntime *runtime
	                      , iObject *context
	                      , iObject *caller
	                      , int argc
	                      , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:hasKeyDeep requires exactly one argument" );
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:hasKeyDeep requires a String as its argument" );
	}
	bool rawr = iObject_hasKeyDeep(caller, iString_getRaw(argv[0]));
	iObject *r = iRuntime_cloneField(runtime, "Number");
	if(rawr){
		iNumber_setRaw(r, 1);
	} else {
		iNumber_setRaw(r, 0);
	}
	return r;
}

static iObject *removeKeyShallow_(iRuntime *runtime
	                            , iObject *context
	                            , iObject *caller
	                            , int argc
	                            , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:removeKeyShallow requires exactly one argument" );
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:removeKeyShallow requires a String as its argument" );
	}
	iObject_remShallow(caller, iString_getRaw(argv[0]));
	return NULL;
}


static iObject *getDeep_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *caller
	                   , int argc
	                   , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:getDeep requires exactly one argument" );
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:getDeep requires a String as its argument" );
	}
	char *raw = iString_getRaw(argv[0]);
	if(!iObject_hasKeyDeep(caller, raw)){
		iRuntime_throwFormatted(runtime, context, "#:getDeep - field does not exist %s", raw);
	}
	return iObject_getDeep(caller, raw);
}



static iObject *getShallow_(iRuntime *runtime
	                      , iObject *context
	                      , iObject *caller
	                      , int argc
	                      , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:getShallow requires exactly one argument" );
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:getShallow requires a String as its argument" );
	}
	char *raw = iString_getRaw(argv[0]);
	if(!iObject_hasKeyShallow(caller, raw)){
		iRuntime_throwFormatted(runtime, context, "#:getShallow - field does not exist %s", raw);
	}
	return iObject_getShallow(caller, raw);
}



static iObject *putDeep_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *caller
	                   , int argc
	                   , iObject **argv){
	if(argc != 2){
		iRuntime_throwString(runtime, context, "#:putDeep requires exactly 2 arguments" );
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:putDeep requires a String as its first argument" );
	}
	iObject_putDeep(caller, iString_getRaw(argv[0]), argv[1]);
	return NULL;
}



static iObject *putShallow_(iRuntime *runtime
	                      , iObject *context
	                      , iObject *caller
	                      , int argc
	                      , iObject **argv){
	if(argc != 2){
		iRuntime_throwString(runtime, context, "#:putShallow requires exactly 2 arguments" );
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:putShallow requires a String as its first argument" );
	}
	iObject_putShallow(caller, iString_getRaw(argv[0]), argv[1]);
	return NULL;
}


static iObject *slotNames_(iRuntime *runtime
	                     , iObject *context
	                     , iObject *caller
	                     , int argc
	                     , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "#:slotNames does not accept arguments");
	}

	iObject *size = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(size, caller->slotCount);

	iObject *r = iRuntime_callMethod(runtime
		                         , context
		                         , runtime->Array
		                         , "withSize"
		                         , 1
		                         , &size);

	iObject_reference(r);

	iObject *index = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(index, 0);
	iObject_reference(index);

	for(int i = 0; i < caller->slotCount; i++){
		iObject *args[2];
		args[0] = index;
		args[1] = iRuntime_MAKE(runtime, String);
		iString_setRaw(args[1], caller->slots[i].key);
		iRuntime_callMethod(runtime
			             , context
			             , r
			             , "at="
			             , 2
			             , args);
		iNumber_setRaw(index, 1 + iNumber_getRaw(index));
	}

	iObject_reference(index);
	iObject_unreference(r);
	return r;
}


static iObject *asString_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *caller
	                    , int argc
	                    , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "#:asString does not accept arguments");
	}
	char temp[128];
	sprintf(temp, "#[%p]", caller);
	iObject *r = iRuntime_cloneField(runtime, "String");	
	iString_setRaw(r, temp);
	return r;
}

static iObject *hasMethod_(iRuntime *runtime
	                     , iObject *context
	                     , iObject *caller
	                     , int argc
	                     , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:hasMethod requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:hasMethod requires its argument to be a string");
	}
	bool rawr = iObject_hasMethod(caller, iString_getRaw(argv[0]));
	iObject *r = iRuntime_cloneField(runtime, "Number");
	if(rawr){
		iNumber_setRaw(r, 1);
	} else {
		iNumber_setRaw(r, 0);
	}
	return r;
}


static iObject *callMethod_(iRuntime *runtime
	                      , iObject *context
	                      , iObject *caller
	                      , int argc
	                      , iObject **argv){
	if(argc == 0){
		iRuntime_throwString(runtime, context, "#:callMethod requires arguments");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "#:callMethod requires its argument to be a string");
	}
	char *methodName = iString_getRaw(argv[0]);
	if(!iObject_hasMethod(caller, methodName)){
		iRuntime_throwFormatted(runtime, context, "#:callMethod could not find method: '%s'", methodName);
	}
	return iRuntime_callMethod(runtime
		                    , context
		                    , caller
		                    , methodName
		                    , argc - 1
		                    , argv + 1);
}


static iObject *compare(iRuntime *runtime
	                  , iObject *context
	                  , iObject *lo
                      , iObject *ro){
	if(iObject_hasMethod(lo, "<>")){
		return iRuntime_callMethod(runtime
			                    , context
			                    , lo
			                    , "<>"
			                    , 1
			                    , &ro);
	} else if(iObject_hasMethod(ro, "<>")){
		iObject *c = iRuntime_callMethod(runtime
			                         , context
			                         , ro
			                         , "<>"
			                         , 1
			                         , &lo);
		iNumber_setRaw(c, -1 * iNumber_getRaw(c));
		return c;
	} else {
		iRuntime_throwString(runtime, context, "<> method not found");
	}
	return NULL;
}


static iObject *above_(iRuntime *runtime
	                , iObject *context
	                , iObject *self
	                , int argc
	                , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:> requires exactly 1 argument");
	}
	iObject *c = compare(runtime, context, self, argv[0]);
	iNumber_setRaw(c, iNumber_getRaw(c) > 0? 1:0);
	return c;
} 

static iObject *aboveEq_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *self
	                   , int argc
	                   , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:>= requires exactly 1 argument");
	}
	iObject *c = compare(runtime, context, self, argv[0]);
	iNumber_setRaw(c, iNumber_getRaw(c) >= 0? 1:0);
	return c;
} 

static iObject *below_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:< requires exactly 1 argument");
	}
	iObject *c = compare(runtime, context, self, argv[0]);
	iNumber_setRaw(c, iNumber_getRaw(c) < 0? 1:0);
	return c;
} 

static iObject *belowEq_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *self
	                   , int argc
	                   , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:<= requires exactly 1 argument");
	}
	iObject *c = compare(runtime, context, self, argv[0]);
	iNumber_setRaw(c, iNumber_getRaw(c) <= 0? 1:0);
	return c;
} 

static iObject *not_(iRuntime *runtime
	               , iObject *context
	               , iObject *self
	               , int argc
	               , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "#:! accepts no arguments");
	}
	iObject *b = iRuntime_callMethod(runtime
		                          , context
		                          , self
		                          , "?"
		                          , 0
		                          , NULL);
	iNumber_setRaw(b, iNumber_getRaw(b)? 0:1);
	return b;
} 

static iObject *clone_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "#:~ accepts no arguments");
	}

	return iRuntime_simpleClone(runtime, self);
} 



static iObject *copyAndDo(iRuntime *runtime
	                    , iObject *context
	                    , iObject *lo
	                    , char *op
	                    , int argc
	                    , iObject **argv){
	if(!iObject_hasMethod(lo, op)){
		iRuntime_throwFormatted(runtime, context, "'%s' method not found", op);
	}

	iObject *cp = iRuntime_callMethod(runtime
		                           , context
		                           , lo
		                           , "$"
		                           , 0
		                           , NULL);
	if(!cp){
		iRuntime_throwString(runtime, context, "copy returned NULL");
	}
	iObject_reference(cp);
	iRuntime_callMethod(runtime, context, cp, op, argc, argv);
	iObject_unreference(cp);
	return cp;
}


static iObject *plus_(iRuntime *runtime
	                , iObject *context
	                , iObject *self
	                , int argc
	                , iObject **argv){
	return copyAndDo(runtime, context, self, "+=", argc, argv);
} 

static iObject *minus_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	return copyAndDo(runtime, context, self, "-=", argc, argv);
} 

static iObject *div_(iRuntime *runtime
	               , iObject *context
	               , iObject *self
	               , int argc
	               , iObject **argv){
	return copyAndDo(runtime, context, self, "/=", argc, argv);
} 

static iObject *times_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	return copyAndDo(runtime, context, self, "*=", argc, argv);
} 

static iObject *mod_(iRuntime *runtime
	               , iObject *context
	               , iObject *self
	               , int argc
	               , iObject **argv){
	return copyAndDo(runtime, context, self, "%=", argc, argv);
} 


static iObject *exp_(iRuntime *runtime
	               , iObject *context
	               , iObject *self
	               , int argc
	               , iObject **argv){
	return copyAndDo(runtime, context, self, "^=", argc, argv);
} 


static iObject *value_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "$ does not accept arguments");
	}

	// check if arg is able be copied using default methods
	for(int i = 0; i < self->slotCount; i++){
		iSlot *s = self->slots + i;
		if(iSlot_isPrimitive(s) && strcmp(s->key, "__referenceCount") != 0){
			iRuntime_throwFormatted(runtime, context, "object not copy-able (contains primitive data at: '%s')", s->key);
		}
	}

	iObject *r = iRuntime_rawObject(runtime);
	iObject_reference(r);
	for(int i = 0; i < self->slotCount; i++){
		iSlot *s = self->slots + i;
		iObject *o = iSlot_object(s);
		if(strcmp(s->key, "_data") == 0){
			iObject_putShallow(r, "_data", iRuntime_callMethod(runtime
				                              , context
				                              , o
				                              , "$"
				                              , 0
				                              , NULL));
		} else {
			iObject_putShallow(r, s->key, o);
		}
	}
	iObject_unreference(r);
	return r;
} 

static bool slotsAreEqual(iRuntime *runtime
	                    , iObject *context
	                    , iSlot *s0
	                    , iSlot *s1){
	if(s0 == s1){
		return true;
	}

	if(iSlot_isPrimitive(s0)){
		if(!iSlot_isPrimitive(s1)){
			return false;
		}
		return iSlot_data(s0) == iSlot_data(s1);
	} else {
		iObject *arg = iSlot_object(s1);
		return iRuntime_callMethod(runtime
			                    , context
			                    , iSlot_object(s0)
			                    , "=="
			                    , 1
			                    , &arg);
	}
}


static bool objectsAreEqual(iRuntime *runtime
	                      , iObject *context
	                      , iObject *o0
	                      , iObject *o1){
	if(o0 == o1){
		return true;
	}

	if(iObject_hasMethod(o0, "<>")){
		iObject *n = iRuntime_callMethod(runtime, context, o0, "<>", 1, &o1);
		return iNumber_getRaw(n) == 0;
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

static iObject *equals_(iRuntime *runtime
	                  , iObject *context
	                  , iObject *self
	                  , int argc
	                  , iObject **argv){
	if(argc < 1){
		iRuntime_throwString(runtime, context, "#:== requires arguments");
	}

	iObject *r = iRuntime_cloneField(runtime, "Number");
	iNumber_setRaw(r, 1);
	iObject_reference(r);
	for(int i = 0; i < argc; i++){
		if(!objectsAreEqual(runtime
			              , context
			              , self
			              , argv[i])){
			iNumber_setRaw(r, 0);
			break;
		}
	}
	iObject_unreference(r);
	return r;
} 

static iObject *notEq_(iRuntime *runtime
	                 , iObject *context
	                 , iObject *self
	                 , int argc
	                 , iObject **argv){
	iObject *equals = iRuntime_callMethod(runtime
		                              , context
		                              , self
		                              , "=="
		                              , argc
		                              , argv);
	return iRuntime_callMethod(runtime
		                    , context
		                    , equals
		                    , "!"
		                    , 0
		                    , NULL);
} 


static iObject *is_(iRuntime *runtime
	              , iObject *context
	              , iObject *caller
	              , int argc
	              , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "#:is requires exactly one argument");
	}
	iObject *r = iRuntime_cloneField(runtime, "Number");
	iNumber_setRaw(r, caller == argv[0]? 1:0);
	return r;
} 

static iObject *asBoolean_(iRuntime *runtime
	                     , iObject *context
	                     , iObject *caller
	                     , int argc
	                     , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "#:? does not accept arguments");
	}

	iObject *r = iRuntime_cloneField(runtime, "Number");
	iObject_reference(r);

	if(iObject_hasMethod(caller, "asNumber")){
		iObject *n = iRuntime_callMethod(runtime
			                         , context
			                         , caller
			                         , "asNumber"
			                         , 0
			                         , NULL);
		iNumber_setRaw(r, iNumber_getRaw(n)? 1:0);
	} else {
		iNumber_setRaw(r, 1); // default to 1 on non-NULL objects
	}

	iObject_unreference(r);
	return r;
} 



// Default hashCode implementation: returns a non-zero
// deterministic value based on address in memory.
static iObject *hashCode_(iRuntime *runtime
	                    , iObject *context
	                    , iObject *self
	                    , int argc
	                    , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "#:hashCode does not accept arguments");
	}

	iObject *r = iRuntime_MAKE(runtime, Number);
	uint64_t hc = 2147483647 ^ ((uint64_t) &self);
	iNumber_setRaw(r, *((double*) &hc));
	return r;
} 


static iObject *def_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iObject_isValid(argv[0]));
	assert(iObject_isValid(argv[1]));

	iObject *route = argv[0];
	iObject *value = unroute(argv[1]);
	if(!value){
		if(iBuiltin_id(argv[1]) == iBUILTIN_ROUTE){
			iRuntime_throwFormatted(runtime, context, "variable does not exist '%s'", iRoute_getRaw(argv[1]));
		} else {
			iRuntime_throwFormatted(runtime, context, "cannot assign '%s' to NULL", iRoute_getRaw(argv[0]));
		}
		return NULL;
	}
	iObject_reference(value);

	if(argc != 2){
		iRuntime_throwString(runtime, context, "def accepts exactly 2 arguments.");
	} else if(!iRoute_isValid(route)){
		iRuntime_throwString(runtime, context, "def requires a route as first argument.");
	}

	// TODO: use iRoute_submapping here

	iObject *par = context;
	int rargc = iRoute_argc(route);

	for(int i = 0; i < rargc - 1; i++){
		char buf[64];
		iRoute_argv(route, i, buf);
		iObject *ppar = par;
		par = iObject_getDeep(par, buf);
		if(!par){
			par = iRuntime_MAKE(runtime, Object);
			iObject_putShallow(ppar, buf, par);
		}
	}
	char fbuf[64];
	iRoute_argv(route, rargc - 1, fbuf);
	if(strcmp(fbuf, "#") != 0 && iObject_hasKeyShallow(par, fbuf)){
		iRuntime_throwFormatted(runtime, context, "def would overwrite value '%s'", fbuf);
	}
	iObject_putShallow(par, fbuf, value);

	iObject_unreference(value);
	return NULL;
}


static iObject *set_(iRuntime *runtime
	               , iObject *context
	               , iObject *caller
	               , int argc
	               , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));

	if(argc != 2){
		iRuntime_throwString(runtime, context, "set requires exactly 2 arguments.");
		return NULL;
	} else if(!iRoute_isValid(argv[0])){
		iRuntime_throwString(runtime, context, "set requires a route for argument 1.");
		return NULL;
	}

	iObject *route = argv[0];
	iObject *value = unroute(argv[1]);
	if(!value){
		if(iBuiltin_id(argv[1]) == iBUILTIN_ROUTE){
			iRuntime_throwFormatted(runtime, context, "variable does not exist '%s'", iRoute_getRaw(argv[1]));
		} else {
			iRuntime_throwFormatted(runtime, context, "cannot assign '%s' to NULL", iRoute_getRaw(argv[0]));
		}
		return NULL;
	}

	iObject *submapping = iRoute_submapping(route);
	char field[32];
	iRoute_argv(route, iRoute_argc(route) - 1, field);
	
	if(submapping && iObject_hasKeyDeep(submapping, field)){
		iObject_putDeep(submapping, field, value);
		return NULL;
	}

	iRuntime_throwFormatted(runtime, context, "set failed on '%s'; try def.", iRoute_getRaw(route));
	return NULL;
}


static iObject *markRecursively_(iRuntime *runtime
	                           , iObject *context
	                           , iObject *self
	                           , int argc
	                           , iObject **argv){
	if(!self || self->gc_mark){
		return NULL;
	}

	// mark this object
	self->gc_mark = true;

	// mark all non-primitive fields recursively
	for(int i = 0; i < self->slotCount; i++){
		if(!iSlot_isPrimitive(self->slots + i)){
			markRecursively_(runtime
				           , context
				           , iSlot_object(self->slots + i)
				           , 0
				           , NULL);
		}
	}

	// if object has special internals to mark
	if(iObject_hasMethod(self, "_markInternalsRecursively")){
		iRuntime_callMethod(runtime, context, self, "_markInternalsRecursively", 0, NULL);
	}
	return NULL;
}


void iBase_init(iObject *self, iRuntime *runtime){
	assert(self);
	iBuiltin_setId(self, iBUILTIN_OBJECT);
	iRuntime_registerCMethod(runtime, self, "hasMethod", hasMethod_);
	iRuntime_registerCMethod(runtime, self, "callMethod", callMethod_);
	iRuntime_registerCMethod(runtime, self, "hasKeyShallow", hasKeyShallow_);
	iRuntime_registerCMethod(runtime, self, "hasKeyDeep", hasKeyDeep_);
	iRuntime_registerCMethod(runtime, self, "removeKeyShallow", removeKeyShallow_);
	iRuntime_registerCMethod(runtime, self, "getDeep", getDeep_);
	iRuntime_registerCMethod(runtime, self, "getShallow", getShallow_);
	iRuntime_registerCMethod(runtime, self, "putDeep", putDeep_);
	iRuntime_registerCMethod(runtime, self, "putShallow", putShallow_);
	iRuntime_registerCMethod(runtime, self, "slotNames", slotNames_);
	iRuntime_registerCMethod(runtime, self, "asString", asString_);

	iRuntime_registerCMethod(runtime, self, "?", asBoolean_);

	iRuntime_registerCMethod(runtime, self, ">", above_);
	iRuntime_registerCMethod(runtime, self, ">=", aboveEq_);
	iRuntime_registerCMethod(runtime, self, "<", below_);
	iRuntime_registerCMethod(runtime, self, "<=", belowEq_);
	iRuntime_registerCMethod(runtime, self, "!", not_);
	iRuntime_registerCMethod(runtime, self, "~", clone_);
	iRuntime_registerCMethod(runtime, self, "+", plus_);
	iRuntime_registerCMethod(runtime, self, "-", minus_);
	iRuntime_registerCMethod(runtime, self, "*", times_);
	iRuntime_registerCMethod(runtime, self, "/", div_);
	iRuntime_registerCMethod(runtime, self, "%", mod_);
	iRuntime_registerCMethod(runtime, self, "^", exp_);
	iRuntime_registerCMethod(runtime, self, "$", value_);
	iRuntime_registerCMethod(runtime, self, "==", equals_);
	iRuntime_registerCMethod(runtime, self, "!=", notEq_);
	iRuntime_registerCMethod(runtime, self, "is", is_);

	iRuntime_registerPrivelegedCMethod(runtime, self, "def", def_);
	iRuntime_registerPrivelegedCMethod(runtime, self, "set", set_);

	iRuntime_registerCMethod(runtime, self, "_hashCode", hashCode_);

	iRuntime_registerCMethod(runtime, self, "_markRecursively", markRecursively_);
}


