#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h> 
#include <time.h>  

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/string.h>



Object *exceptions_try(Runtime *runtime
	                 , Object *context
	                 , Object *module
	                 , int argc
	                 , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "try requires exactly 2 inputs");
	}

	jmp_buf *jbptr = malloc(sizeof(jmp_buf));  
	if(!jbptr){
		abort();
	}

	if(!setjmp(*jbptr)){
		Object_putDataShallow(context, "__try", jbptr);  // note: jbptr will be cleaned when tryctx is collected
		Object_putShallow(context, "_catch", argv[1]);
		Runtime_activate(runtime
			           , context
			           , argv[0]
			           , 0
			           , NULL);
	}

	Object_remShallow(context, "__try");
	Object_remShallow(context, "_catch");

	return NULL;
}


Object *exceptions_throw(Runtime *runtime
	                   , Object *context
	                   , Object *module
	                   , int argc
	                   , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "throw requires an argument");
	}

	// Check for __try field, which, if present, will contain
	// a pointer to a jmp_buf. 
	jmp_buf *jbuf = Object_getDataDeep(context, "__try");
	if(jbuf){
		Object *catchFunction = Object_getDeep(context, "_catch");
		if(!catchFunction){
			perror("no catch function");
		}
		Runtime_activate(runtime
			           , context
			           , catchFunction
			           , 1
			           , argv);
		longjmp(*jbuf, 1);
	}

	// If said field isn't present, report the error and 
	// exit.
	Object *asString = Runtime_callMethod(runtime
		                                , context
		                                , argv[0]
		                                , "asString"
		                                , 0
		                                , NULL);
	fprintf(stderr, "Uncaught exception: %s.\n", ImpString_getRaw(asString));
	exit(1);
}
