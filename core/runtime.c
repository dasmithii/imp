#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>


Object *runtime_rawObject(Runtime *runtime
	              , Object *call_context
	              , Object *module
	              , int argc
                  , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "runtime:rawObject does not accept arguments");
	}
	return Runtime_rawObject(runtime);
}


Object *runtime_cloneObject(Runtime *runtime
	                      , Object *call_context
	                      , Object *module
	                      , int argc
                          , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "runtime:cloneObject requires exactly one argument");
	}
	return Runtime_clone(runtime, argv[0]);
}


Object *runtime_executeSource(Runtime *runtime
	                        , Object *call_context
	                        , Object *module
	                        , int argc
                            , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "runtime:executeSource requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
				Runtime_throwString(runtime, "runtime:executeSource requires a string argument");
	}
	return Runtime_executeSource(runtime, ImpString_getRaw(argv[0]));
}


Object *runtime_throw(Runtime *runtime
	                , Object *call_context
	                , Object *module
	                , int argc
                    , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "runtime:throw requires exactly one argument");
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
		Runtime_throwString(runtime, "runtime:objectCount does not accept arguments");
	}
	Object *r = Runtime_cloneField(runtime, "Number");
	ImpNumber_setRaw(r, (double) Runtime_objectCount(runtime));
	return r;
}
