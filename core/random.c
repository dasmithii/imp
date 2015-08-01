#include <stdio.h>
#include <stdlib.h> 
#include <time.h>  

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>


Object *random_onImport(Runtime *runtime
	                  , Object *context
	                  , Object *module
	                  , int argc
	                  , Object **argv){
	srand(time(NULL));
	return NULL;
}


Object *random_activate(Runtime *runtime
	                  , Object *context
	                  , Object *module
	                  , int argc
	                  , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "random:Number does not accept arguments");
	}
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, (double) rand());
	return r;
}


Object *random_between(Runtime *runtime
	                 , Object *context
	                 , Object *module
	                 , int argc
	                 , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, context, "random:between requires exactly 2 arguments");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER ||
	   BuiltIn_id(argv[1]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "random:between requires numbers as arguments");
	}

	const double n0 = ImpNumber_getRaw(argv[0]);
	const double n1 = ImpNumber_getRaw(argv[1]);

	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, n0 + (n1-n0) * (double) rand() / (double) RAND_MAX);
	return r;
}


Object *random_unit(Runtime *runtime
	              , Object *context
	              , Object *module
	              , int argc
	              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, context, "random:unit does not accept arguments");
	}
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, (double) rand() / (double) RAND_MAX);
	return r;
}
