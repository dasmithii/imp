#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>

#include <stdbool.h>


static Object *root = NULL; // module root
static Object *ts = NULL;   // true singleton
static Object *fs = NULL;   // false singleton


Object *Boolean_not(Runtime *runtime
	              , Object *context
	              , Object *caller
	              , int argc
	              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Boolean:not accepts no arguments");
	}
	if(caller == ts){
		return fs;
	} else if(caller == fs){
		return ts;
	} else {
		Runtime_throwString(runtime, "invalid boolean");
		return NULL;
	}
}


Object *Boolean_asNumber(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Boolean:asNumber accepts no arguments");
	}
	Object *r = Runtime_cloneField(runtime, "Number");
	if(caller == ts){
		ImpNumber_setRaw(r, 1);
	} else if(caller == fs){
		ImpNumber_setRaw(r, 0);
	} else {
		Runtime_throwString(runtime, "invalid boolean");
	}
	return r;
}


Object *Boolean_asString(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Boolean:asString accepts no arguments");
	}
	Object *r = Runtime_cloneField(runtime, "String");
	if(caller == ts){
		ImpString_setRaw(r, "true");
	} else if(caller == fs){
		ImpString_setRaw(r, "false");
	} else {
		Runtime_throwString(runtime, "invalid boolean");
	}
	return r;
}


Object *Boolean_onImport(Runtime *runtime
	                   , Object *context
	                   , Object *module
	                   , int argc
	                   , Object **argv){
	root = module;

	ts = Runtime_clone(runtime, root);
	Object_putShallow(module, "true", ts);

	fs = Runtime_clone(runtime, root);
	Object_putShallow(module, "false", fs);
	return NULL;
}


Object *Boolean_clone(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	Runtime_throwString(runtime, "Boolean not clonable");
	return NULL;
}
