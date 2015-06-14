#include <string.h>

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/string.h>
#include <imp/builtin/vector.h>



Object *cli_onImport(Runtime *runtime
	               , Object *context
	               , Object *module
	               , int argc
	               , Object **argv){
	printf("argc: %d\n", runtime->argc);
	Object *obj = Runtime_cloneField(runtime, "Vector");
	Object_reference(obj);

	Vector *vec = ImpVector_getRaw(obj);
	for(int i = 0; i < runtime->argc; i++){
		printf("%s\n", runtime->argv[i]);
		Object *arg = Runtime_cloneField(runtime, "String");
		ImpString_setRaw(arg, runtime->argv[i]);
		Vector_append(vec, arg);
	}
	printf("what\n");
	Object_putShallow(module, "arguments", obj);
	Object_unreference(obj);
	return NULL;
}