#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/number.h>
#include <imp/builtin/general.h>


#define MICROSECONDS_PER_SECOND 1000000
#define MICROSECONDS_PER_MILLISECOND 1000
#define SECONDS_PER_MINUTE 60
#define MINUTE_PER_HOUR 60


iObject *time_seconds(iRuntime *runtime
	                , iObject *context
	                , iObject *module
	                , int argc
	                , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "time:seconds requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "time:seconds requires numeric argument");
	}
	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, MICROSECONDS_PER_SECOND * iNumber_getRaw(argv[0]));
	return r;
}


iObject *time_milliseconds(iRuntime *runtime
	                     , iObject *context
	                     , iObject *module
	                     , int argc
	                     , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "time:milliseconds requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "time:milliseconds requires numeric argument");
	}
	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, MICROSECONDS_PER_MILLISECOND * iNumber_getRaw(argv[0]));
	return r;
}


iObject *time_minutes(iRuntime *runtime
	                , iObject *context
	                , iObject *module
	                , int argc
	                , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "time:minutes requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "time:minutes requires numeric argument");
	}
	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, SECONDS_PER_MINUTE * MICROSECONDS_PER_SECOND * iNumber_getRaw(argv[0]));
	return r;
}


iObject *time_hours(iRuntime *runtime
	              , iObject *context
	              , iObject *module
	              , int argc
	              , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "time:hours requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "time:hours requires numeric argument");
	}
	iObject *r = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(r, MINUTE_PER_HOUR * SECONDS_PER_MINUTE * MICROSECONDS_PER_SECOND * iNumber_getRaw(argv[0]));
	return r;
}
