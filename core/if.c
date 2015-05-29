#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/boolean.h>


// If accepts one or more condition-executable pairs as
// arguments, plus an optional final argument, the 
// 'else' block. Conditions are scanned until one is
// met, at which point, the corresponding executable is
// executed. In the case that no conditions are met,
// the final argument is executed. 
Object *if_activate(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc == 0){
		Runtime_throwString(runtime, "if given no arguments");
	}

	for(int i = 0; i < argc - 1; i += 2){
		Runtime_activate(runtime, context, argv[i], 0, NULL);
		if(Runtime_returnValue(runtime) == NULL                         ||
		   BuiltIn_id(Runtime_returnValue(runtime)) != BUILTIN_BOOLEAN  ||
		   ImpBoolean_getRaw(Runtime_returnValue(runtime)) == true){
			Runtime_activate(runtime, context, argv[i+1], 0, NULL);
			return NULL;
		}
	}
	
	if(argc % 2 == 1){
		Runtime_activate(runtime, context, argv[argc-1], 0, NULL);
	}
}