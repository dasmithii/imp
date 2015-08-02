#include <stdio.h>
#include <stdlib.h> 
#include <time.h>  

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>


iObject *random_onImport(iRuntime *runtime
	                   , iObject *context
	                   , iObject *module
	                   , int argc
	                   , iObject **argv){
	srand(time(NULL));
	return NULL;
}


iObject *random_activate(iRuntime *runtime
	                   , iObject *context
	                   , iObject *module
	                   , int argc
	                   , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "random:Number does not accept arguments");
	}
	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, (double) rand());
	return r;
}


iObject *random_between(iRuntime *runtime
	                  , iObject *context
	                  , iObject *module
	                  , int argc
	                  , iObject **argv){
	if(argc != 2){
		iRuntime_throwString(runtime, context, "random:between requires exactly 2 arguments");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER ||
	   iBuiltin_id(argv[1]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "random:between requires numbers as arguments");
	}

	const double n0 = iNumber_getRaw(argv[0]);
	const double n1 = iNumber_getRaw(argv[1]);

	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, n0 + (n1-n0) * (double) rand() / (double) RAND_MAX);
	return r;
}


iObject *random_unit(iRuntime *runtime
	               , iObject *context
	               , iObject *module
	               , int argc
	               , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "random:unit does not accept arguments");
	}
	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, (double) rand() / (double) RAND_MAX);
	return r;
}
