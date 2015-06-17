#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>
#include <stdbool.h>
#include <string.h>


// note: this function, isZero, is duplicated for all
// control structures (in each file) because I'm inept.
// Make sure to replicate any changes to this function
// in the other files as well.
static bool isZero(Runtime *runtime
	             , Object *ctx
	             , Object *obj){
	if(!obj){
		return true;
	}

	if(BuiltIn_id(obj) == BUILTIN_NUMBER){
		return ImpNumber_getRaw(obj) == 0;
	}

	if(Object_hasMethod(obj, "?")){
		Object_reference(ctx);
		Object_reference(obj);
		Object *asBoolean = Runtime_callMethod(runtime
			                                 , ctx
			                                 , obj
			                                 , "?", 0, NULL);

		Object_unreference(ctx);
		Object_unreference(obj);
		return ImpNumber_getRaw(asBoolean) == 0;
	}

	Runtime_throwString(runtime, "object not boolean");
	return false;
}
