// #include <stdbool.h>
// #include <stdio.h>

// #include "../object.h"
// #include "../c.h"
// #include "../runtime.h"

// #include "boolean.h"
// #include "general.h"
// #include "while.h"




// bool ImpWhile_isValid(Object *self){
// 	return Object_isValid(self) &&
// 	       BuiltIn_id(self) == BUILTIN_WHILE;
// }


// static Object *ImpWhile_activate_internal(Runtime *runtime
// 	                                    , Object *context
// 	                                    , Object *caller
// 	                                    , int argc
// 	                                    , Object **argv){
// 	assert(runtime);
// 	assert(Object_isValid(context));
// 	assert(ImpWhile_isValid(caller));

// 	Object *condition = argv[0];
// 	Object *step = argv[1];

// 	if(argc != 2){
// 		Runtime_throwString(runtime, "while accepts exactly 2 arguments.");
// 	} else if(!Object_canBeActivated(condition)){
// 		Runtime_throwString(runtime, "while condition must be activatable");
// 	} else if(!Object_canBeActivated(step)){
// 		Runtime_throwString(runtime, "while step must be activatable");
// 	}

// 	for(;;){
// 		// check condition
// 		Runtime_activate(runtime, context, condition, 0, NULL);
// 		if(Runtime_returnValue(runtime) == NULL                         ||
// 		   BuiltIn_id(Runtime_returnValue(runtime)) != BUILTIN_BOOLEAN  ||
// 		   ImpBoolean_getRaw(Runtime_returnValue(runtime)) == false){
// 			break;
// 		}

// 		// execute block
// 		Object *subcontext = Runtime_clone(runtime, context);
// 		Runtime_activate(runtime, subcontext, step, 0, NULL);
// 	}

// 	return NULL;
// }


// void ImpWhile_init(Object *self){
// 	assert(self);
// 	BuiltIn_setId(self, BUILTIN_WHILE);
// 	Object_registerCMethod(self, "__activate", ImpWhile_activate_internal);
// }