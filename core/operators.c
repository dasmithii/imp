#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/boolean.h>


Object *eq(Runtime *runtime
	            , Object *context
	            , Object *left
	            , int argc
                , Object **argv){
	if(argc == 0){
		Runtime_throwString(runtime, "== requires arguments");
	}
	if(!Object_hasMethod(left, "equals")){
		Runtime_throwString(runtime, "equality method not defined");
	}

	for(int i = 0; i < argc; i++){
		Object *b = Runtime_callMethod(runtime
			                         , context
			                         , left
			                         , "equals"
			                         , 1
			                         , &argv[i]);
		if(ImpBoolean_getRaw(b) == false){
			Object *r = Runtime_cloneField(runtime, "false");
			ImpBoolean_setRaw(r, false);
			return r;
		}
	}

	Object *r = Runtime_cloneField(runtime, "true");
	ImpBoolean_setRaw(r, true);
	return r;
}


Object *operators_onImport(Runtime *runtime
	                     , Object *context
	                     , Object *module
	                     , int argc
                         , Object **argv){
	Object_putDataShallow(context, "__==", eq);
	return NULL;
}
