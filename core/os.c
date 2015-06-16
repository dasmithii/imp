#include <string.h>

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/string.h>
#include <imp/builtin/vector.h>



Object *os_onImport(Runtime *runtime
	               , Object *context
	               , Object *module
	               , int argc
	               , Object **argv){
	Object *obj = Runtime_make(runtime, Vector);
	Object_reference(obj);

	Vector *vec = ImpVector_getRaw(obj);
	for(int i = 0; i < runtime->argc; i++){
		Object *arg = Runtime_make(runtime, String);
		ImpString_setRaw(arg, runtime->argv[i]);
		Vector_append(vec, &arg);
	}
	Object_putShallow(module, "arguments", obj);
	Object_unreference(obj);
	return NULL;
}