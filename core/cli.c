#include <string.h>

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/string.h>


extern int imp_raw_argc;
extern char **imp_raw_argv;



Object *cli_onImport(Runtime *runtime
	           , Object *context
	           , Object *module
	           , int argc
	           , Object **argv){
	Object *options = Runtime_newObject(runtime);
	Object_reference(options);
	Object *args = Runtime_cloneField(runtime, "Vector");
	Object_reference(args);
	for(int i = 0; i < imp_raw_argc; i++){
		char *a = imp_raw_argv[i];
		if(a[0] == '-'){
			char *mid = strchr(a, '=');
			if(mid){
				char name[48];
				strcpy(name, a);
				*strchr(name, '=') = 0;
				Object *val = Runtime_cloneField(runtime, "String");
				ImpString_setRaw(val, mid + 1);
				Object_putShallow(options, name, val);
			} else {
				Object *val = Runtime_cloneField(runtime, "String");
				ImpString_setRaw(val, "1");
				Object_putShallow(options, a, val);
			}
		} else {
			Object *arg = Runtime_cloneField(runtime, "String");
			ImpString_setRaw(arg, a);
			Runtime_callMethod(runtime
				             , context
				             , args
				             , "append"
				             , 1
				             , &arg);
		}
	}
	Object_putShallow(module, "options", options);
	Object_putShallow(module, "args", args);
	Object_unreference(options);
	Object_unreference(args);

}