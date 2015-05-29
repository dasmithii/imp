#include <imp/runtime.h>
#include <imp/object.h>


Object *clone_activate(Runtime *runtime
	                 , Object *context
	                 , Object *caller
	                 , int argc
	                 , Object **argv){
	assert(runtime);

	if(argc != 1){
		Runtime_throwString(runtime, "clone requires exactly one argument");
	}

	if(argv[0] == NULL){
		Runtime_throwString(runtime, "cannot clone NULL");
	}

	return Runtime_clone(runtime, argv[0]);
}