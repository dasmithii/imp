#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>


Object *Runtime_clone(Runtime *runtime
	                , Object *context
	                , Object *base
	                , int argc
	                , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "Runtime:clone does not accept arguments");
	}
	Object *r = Runtime_simpleClone(runtime, base);
	Object_putDataShallow(r, "__data", runtime);
	return r;
}


Object *Runtime_collect(Runtime *runtime
	                  , Object *context
	                  , Object *base
	                  , int argc
	                  , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, context, "Runtime:collect does not accept arguments");
	}
	// get rid of runtime pointer so its not freed by gc
	Runtime *ptr = Object_getDataShallow(r, "__data");
	Runtime_
	Object_putDataShallow(r, "__data", NULL);
	Object_remShallow(r, "__data");
	return NULL;
}


Object *runtime_rawObject(Runtime *runtime
	              , Object *call_context
	              , Object *module
	              , int argc
                  , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "runtime:rawObject does not accept arguments");
	}
	return Runtime_rawObject(runtime);
}


Object *runtime_executeSource(Runtime *runtime
	                        , Object *call_context
	                        , Object *module
	                        , int argc
                            , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, context, "runtime:executeSource requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
				Runtime_throwString(runtime, context, "runtime:executeSource requires a string argument");
	}
	return Runtime_executeSource(runtime, ImpString_getRaw(argv[0]));
}


Object *runtime_throw(Runtime *runtime
	                , Object *call_context
	                , Object *module
	                , int argc
                    , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, context, "runtime:throw requires exactly one argument");
	}
	Runtime_throw(runtime, argv[0]);
	return NULL;
}


Object *runtime_objectCount(Runtime *runtime
	                      , Object *call_context
	                      , Object *module
	                      , int argc
                          , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "runtime:objectCount does not accept arguments");
	}
	Object *r = Runtime_cloneField(runtime, "Number");
	ImpNumber_setRaw(r, (double) Runtime_objectCount(runtime));
	return r;
}
