#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/number.h>
#include <imp/builtin/importer.h>
#include <imp/c.h>
#include <string.h>
#include <stdbool.h>

static Object *T = NULL;
static Object *F = NULL;

static Object *boolean_flip(Object *b){
	if(b == T){
		return F; 
	} else if(b == F){
		return T;
	} else {
		abort();
	}
}


static Object *clone(Runtime *runtime
	                     , Object *context
	                     , Object *caller
	                     , int argc
                         , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "clone (~) requires exactly 1 argument");
	}
	return Runtime_clone(runtime, argv[0]);
}

static Object *not(Runtime *runtime
	             , Object *context
	             , Object *caller
	             , int argc
                 , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "! accepts only one argument");
	}
	if(argv[0] == T){
		return F;
	} else if(argv[0] == F){
		return T;
	} else if(Object_hasMethod(argv[0], "?")){
		Object *b = Runtime_callMethod(runtime
			                         , context
			                         , argv[0]
			                         , "?"
			                         , 0
			                         , NULL);
		Object_reference(b);
		Object *r = not(runtime, context, NULL, 1, &b);
		Object_unreference(b);
		return r;
	} else {
		Runtime_throwString(runtime, "! failed");
		return NULL;
	}
}

static Object *go(Runtime *runtime
	                     , Object *context
	                     , Object *caller
	                     , int argc
                         , Object **argv){
	// TODO
	Runtime_throwString(runtime, "@ operator not yet implemented");
	return NULL;
}


static Object *raw(Runtime *runtime
	             , Object *context
	             , Object *caller
	             , int argc
                 , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "# accepts no arguments");
	}	
	return Runtime_rawObject(runtime);
}


static Object *copy(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
                   , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "$ requires exactly one argument");
	}	

	if(Object_hasMethod(argv[0], "$")){
		return Runtime_callSpecialMethod(runtime
			                           , context
			                           , argv[0]
			                           , "$"
			                           , 0
			                           , NULL);
	}

	// check if arg is able be copied using default methods
	for(int i = 0; i < argv[0]->slotCount; i++){
		Slot *s = argv[0]->slots + i;
		if(Slot_isPrimitive(s) && strcmp(s->key, "__referenceCount") != 0){
			Runtime_throwFormatted(runtime, "object not copy-able (contains primitive data at: '%s')", s->key);
		}
	}

	Object *r = Runtime_rawObject(runtime);
	Object_reference(r);
	for(int i = 0; i < argv[0]->slotCount; i++){
		Slot *s = argv[0]->slots + i;
		Object *o = Slot_object(s);
		if(strcmp(s->key, "_data") == 0){
			Object_putShallow(r, "_data", copy(runtime  // TODO: investigate
				                             , context
				                             , caller
				                             , 1
				                             , &o));
		} else {
			Object_putShallow(r, s->key, o);
		}
	}
	Object_unreference(r);
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

	Object *cp = copy(runtime, context, NULL, 1, &lo);
	if(!cp){
		Runtime_throwString(runtime, "copy returned null");
	}
	Object_reference(cp);
	Runtime_callMethod(runtime, context, cp, op, 1, &ro);
	Object_unreference(cp);
	return cp;
}


static Object *mod(Runtime *runtime
	             , Object *context
	             , Object *caller
	             , int argc
                 , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "% requires exactly two arguments");
	}
	return copyAndDo(runtime, context, argv[0], argv[1], "%=");
}

static Object *times(Runtime *runtime
	                     , Object *context
	                     , Object *caller
	                     , int argc
                         , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "* requires exactly two arguments");
	}
	return copyAndDo(runtime, context, argv[0], argv[1], "*=");
}

static Object *minus(Runtime *runtime
	                     , Object *context
	                     , Object *caller
	                     , int argc
                         , Object **argv){
	if(argc == 1){
		Object *zero = Runtime_cloneField(runtime, "Number");
		Object_reference(zero);
		ImpNumber_setRaw(zero, 0);
		Object *r = copyAndDo(runtime, context, zero, argv[0], "-=");
		Object_unreference(zero);
		return r;
	} else if(argc == 2){
		return copyAndDo(runtime, context, argv[0], argv[1], "-=");
	} else {
		Runtime_throwString(runtime, "- requires one or two arguments");
		return NULL;
	}
}

static Object *plus(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "+ requires exactly two arguments");
	}
	return copyAndDo(runtime, context, argv[0], argv[1], "+=");
}


static Object *dividedBy(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "/ requires exactly two arguments");
	}
	return copyAndDo(runtime, context, argv[0], argv[1], "/=");
}


static Object *toBool(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
                    , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "? requires exactly one arguments");
	}
	if(argv[0] == T){
		return T;
	} else if(argv[0] == F){
		return F;
	} else if(Object_hasMethod(argv[0], "?")){
		return Runtime_callMethod(runtime
			                    , context
			                    , argv[0]
			                    , "?"
			                    , 0
			                    , NULL);
	} else {
		Runtime_throwString(runtime, "? failed");
		return NULL;
	}
}



static Object *compare(Runtime *runtime
	                 , Object *context
	                 , Object *module
	                 , int argc
                     , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "<> requires exactly two arguments");
	}
	if(Object_hasMethod(argv[0], "<>")){
		return Runtime_callMethod(runtime
			                    , context
			                    , argv[0]
			                    , "<>"
			                    , 1
			                    , &argv[1]);
	} else if(Object_hasMethod(argv[1], "<>")){
		Object *c = Runtime_callMethod(runtime
			                         , context
			                         , argv[1]
			                         , "<>"
			                         , 1
			                         , &argv[0]);
		ImpNumber_setRaw(c, -1 * ImpNumber_getRaw(c));
		return c;
	} else {
		Runtime_throwString(runtime, "<> method not found");
	}
	return NULL;
}

static Object *belowEq(Runtime *runtime
	                 , Object *context
	                 , Object *module
	                 , int argc
                     , Object **argv){
	Object *c = compare(runtime, context, module, argc, argv);
	return (ImpNumber_getRaw(c) <= 0)? T:F;
}




static Object *aboveEq(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	Object *c = compare(runtime, context, module, argc, argv);
	return (ImpNumber_getRaw(c) >= 0)? T:F;
}


static Object *above(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	Object *c = compare(runtime, context, module, argc, argv);
	return (ImpNumber_getRaw(c) > 0)? T:F;
}

static Object *below(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	Object *c = compare(runtime, context, module, argc, argv);
	return (ImpNumber_getRaw(c) < 0)? T:F;
}

static Object *equals(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
                    , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "== requires exactly two arguments");
	}

	if(argv[0] == argv[1]){
		return T;
	}

	if(Object_hasMethod(argv[0], "<>") ||
	   Object_hasMethod(argv[1], "<>")){
		Object *c = compare(runtime, context, caller, argc, argv);
		return (ImpNumber_getRaw(c) == 0)? T:F;
	}

	if(Object_hasMethod(argv[0], "==")){
		return Runtime_callMethod(runtime
			                   , context
			                   , argv[0]
			                   , "=="
			                   , 1
			                   , &argv[1]);
	}

	if(Object_hasMethod(argv[1], "==")){
		return boolean_flip(Runtime_callMethod(runtime
			                                , context
			                                , argv[1]
			                                , "=="
			                                , 1
			                                , &argv[0]));
	}

	Runtime_throwString(runtime, "== failed");
	return NULL;
}

static Object *is(Runtime *runtime
	            , Object *context
	            , Object *module
	            , int argc
                , Object **argv){
	if(argc < 2){
		Runtime_throwString(runtime, "is requires two argument");
	}
	for(int i = 1; i < argc; i++){
		if(argv[0] != argv[i]){
			return F;
		}
	}
	return T;
}


static Object *and(Runtime *runtime
	             , Object *context
	             , Object *module
	             , int argc
                 , Object **argv){
	if(argc < 2){
		Runtime_throwString(runtime, "&& requires two argument");
	}
	for(int i = 0; i < argc; i++){
		if(F == toBool(runtime, context, module, 1, &argv[i])){
			return F;
		}
	}
	return T;
}

static Object *or(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	if(argc < 2){
		Runtime_throwString(runtime, "&& requires two argument");
	}
	for(int i = 0; i < argc; i++){
		if(T == toBool(runtime, context, module, 1, &argv[i])){
			return T;
		}
	}
	return F;
}

Object *operators_onImport(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	Object_registerCMethod(module, "__~", clone);
	Object_registerCMethod(module, "__!", not);
	Object_registerCMethod(module, "__@", go);
	Object_registerCMethod(module, "__#", raw);
	Object_registerCMethod(module, "__$", copy);
	Object_registerCMethod(module, "__%", mod);
	Object_registerCMethod(module, "__*", times);
	Object_registerCMethod(module, "__-", minus);
	Object_registerCMethod(module, "__+", plus);
	Object_registerCMethod(module, "__/", dividedBy);
	Object_registerCMethod(module, "__?", toBool);
	Object_registerCMethod(module, "__<>", compare);
	Object_registerCMethod(module, "__<=", belowEq);
	Object_registerCMethod(module, "__>=", aboveEq);
	Object_registerCMethod(module, "__>", above);
	Object_registerCMethod(module, "__<", below);
	Object_registerCMethod(module, "__==", equals);
	Object_registerCMethod(module, "__is", is);

	// TODO '^', '&' and '|' operators


	Object_registerCMethod(module, "__&&", and);
	Object_registerCMethod(module, "__||", or);

	Object *boolean_module = Imp_import(runtime, "core/Boolean");
	T = Object_getShallow(boolean_module, "true");
	F = Object_getShallow(boolean_module, "false");
	return NULL;
}
