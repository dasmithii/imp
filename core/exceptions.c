#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h> 
#include <time.h>  

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/string.h>
#include <imp/builtin/general.h>



iObject *exceptions_try(iRuntime *runtime
	                  , iObject *context
	                  , iObject *module
	                  , int argc
	                  , iObject **argv){
	if(argc != 2){
		iRuntime_throwString(runtime, context,  "try requires exactly 2 inputs");
	}

	jmp_buf *jbptr = malloc(sizeof(jmp_buf));  
	if(!jbptr){
		abort();
	}

	if(!setjmp(*jbptr)){
		iObject_putDataShallow(context, "__try", jbptr);  // note: jbptr will be cleaned when tryctx is collected
		iObject_putShallow(context, "_catch", argv[1]);
		iRuntime_activate(runtime
			           , context
			           , argv[0]
			           , 0
			           , NULL);
	}

	iObject_remShallow(context, "__try");
	iObject_remShallow(context, "_catch");

	return NULL;
}


iObject *exceptions_throw(iRuntime *runtime
	                    , iObject *context
	                    , iObject *module
	                    , int argc
	                    , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "throw requires an argument");
	}

	// Check for __try field, which, if present, will contain
	// a pointer to a jmp_buf. 
	jmp_buf *jbuf = iObject_getDataDeep(context, "__try");
	if(jbuf){
		iObject *catchFunction = iObject_getDeep(context, "_catch");
		if(!catchFunction){
			perror("no catch function");
		}
		iRuntime_activate(runtime
			           , context
			           , catchFunction
			           , 1
			           , argv);
		longjmp(*jbuf, 1);
	}

	// If said field isn't present, report the error and 
	// exit.
	iObject *asString = iRuntime_callMethod(runtime
		                                 , context
		                                 , argv[0]
		                                 , "asString"
		                                 , 0
		                                , NULL);
	iObject *sfile = iObject_getDeep(context, "_FILE");
	if(sfile){
		if(iBuiltin_id(sfile) != iBUILTIN_STRING){
			iRuntime_throwString(runtime, context, "_FILE should be string");
		}
		fprintf(stderr, "Uncaught exception in '%s': %s.\n", iString_getRaw(sfile), iString_getRaw(asString));
	} else {
		fprintf(stderr, "Uncaught exception: %s.\n", iString_getRaw(asString));
	}
	exit(1);
}
