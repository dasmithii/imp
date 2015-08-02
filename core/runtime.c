#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>


iObject *iRuntime_clone(iRuntime *runtime
	                  , iObject *context
	                  , iObject *base
	                  , int argc
	                  , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "iRuntime:clone does not accept arguments");
	}
	iObject *r = iRuntime_simpleClone(runtime, base);
	iObject_putDataShallow(r, "__data", runtime);
	return r;
}


iObject *iRuntime_collect(iRuntime *runtime
	                   , iObject *context
	                   , iObject *base
	                   , int argc
	                   , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, context, "iRuntime:collect does not accept arguments");
	}
	// get rid of runtime pointer so its not freed by gc
	iRuntime *ptr = iObject_getDataShallow(r, "__data");
	iRuntime_
	iObject_putDataShallow(r, "__data", NULL);
	iObject_remShallow(r, "__data");
	return NULL;
}


iObject *runtime_rawiObject(iRuntime *runtime
	                      , iObject *call_context
	                      , iObject *module
	                      , int argc
                          , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "runtime:rawiObject does not accept arguments");
	}
	return iRuntime_rawiObject(runtime);
}


iObject *runtime_executeSource(iRuntime *runtime
	                         , iObject *call_context
	                         , iObject *module
	                         , int argc
                             , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "runtime:executeSource requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
				iRuntime_throwString(runtime, context, "runtime:executeSource requires a string argument");
	}
	return iRuntime_executeSource(runtime, iString_getRaw(argv[0]));
}


iObject *runtime_throw(iRuntime *runtime
	                 , iObject *call_context
	                 , iObject *module
	                 , int argc
                     , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "runtime:throw requires exactly one argument");
	}
	iRuntime_throw(runtime, argv[0]);
	return NULL;
}


iObject *runtime_objectCount(iRuntime *runtime
	                       , iObject *call_context
	                       , iObject *module
	                       , int argc
                           , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "runtime:objectCount does not accept arguments");
	}
	iObject *r = iRuntime_cloneField(runtime, "Number");
	iNumber_setRaw(r, (double) iRuntime_objectCount(runtime));
	return r;
}
