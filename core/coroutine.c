#include <imp/object.h>
#include <imp/runtime.h>
#include <unistd.h>

#include <imp/builtin/general.h>
#include <imp/builtin/number.h>



Object *coroutine_sleep(Runtime *runtime
	                  , Object *context
	                  , Object *module
	                  , int argc
	                  , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "coroutine:sleep requires an argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "coroutine:sleep requires numeric argument");
	}
	double raw = ImpNumber_getRaw(argv[0]);
	usleep((unsigned int) raw);
	return NULL;
}


// TODO:
// + yield
// + exit
// + more