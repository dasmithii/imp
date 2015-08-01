#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/number.h>
#include <imp/builtin/general.h>


#define MICROSECONDS_PER_SECOND 1000000
#define MICROSECONDS_PER_MILLISECOND 1000
#define SECONDS_PER_MINUTE 60
#define MINUTE_PER_HOUR 60


Object *time_seconds(Runtime *runtime
	               , Object *context
	               , Object *module
	               , int argc
	               , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, context, "time:seconds requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "time:seconds requires numeric argument");
	}
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, MICROSECONDS_PER_SECOND * ImpNumber_getRaw(argv[0]));
	return r;
}


Object *time_milliseconds(Runtime *runtime
	                    , Object *context
	                    , Object *module
	                    , int argc
	                    , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, context, "time:milliseconds requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "time:milliseconds requires numeric argument");
	}
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, MICROSECONDS_PER_MILLISECOND * ImpNumber_getRaw(argv[0]));
	return r;
}


Object *time_minutes(Runtime *runtime
	               , Object *context
	               , Object *module
	               , int argc
	               , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, context, "time:minutes requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "time:minutes requires numeric argument");
	}
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, SECONDS_PER_MINUTE * MICROSECONDS_PER_SECOND * ImpNumber_getRaw(argv[0]));
	return r;
}


Object *time_hours(Runtime *runtime
	             , Object *context
	             , Object *module
	             , int argc
	             , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, context, "time:hours requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, context, "time:hours requires numeric argument");
	}
	Object *r = Runtime_make(runtime, Number);
	ImpNumber_setRaw(r, MINUTE_PER_HOUR * SECONDS_PER_MINUTE * MICROSECONDS_PER_SECOND * ImpNumber_getRaw(argv[0]));
	return r;
}
