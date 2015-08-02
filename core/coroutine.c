#include <imp/object.h>
#include <imp/runtime.h>
#include <unistd.h>

#include <imp/builtin/general.h>
#include <imp/builtin/number.h>



iObject *coroutine_sleep(iRuntime *runtime
	                   , iObject *context
	                   , iObject *module
	                   , int argc
	                   , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "coroutine:sleep requires an argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "coroutine:sleep requires numeric argument");
	}
	double raw = iNumber_getRaw(argv[0]);
	usleep((unsigned int) raw);
	return NULL;
}


// TODO:
// + yield
// + exit
// + more