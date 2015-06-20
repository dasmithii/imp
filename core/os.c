#include <string.h>
#include <dirent.h> 
#include <stdio.h> 

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
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
	Object_putShallow(module, "@", obj);
	Object_unreference(obj);
	return NULL;
}

// lists only files (i.e. not symlinks or directories)
Object *os_eachItemIn(Runtime *runtime
	                , Object *context
	                , Object *module
	                , int argc
                    , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "os:eachFileNameIn requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "os:eachFileNameIn requires String as first argument");
	}
	if(!Object_canBeActivated(argv[1])){
		Runtime_throwString(runtime, "os:eachFileNameIn requires activate-able object as second argument");
	}

	DIR           *d;
	struct dirent *dir;
	d = opendir(ImpString_getRaw(argv[0]));
	if(d){
		while((dir = readdir(d)) != NULL){
			Object *item = Runtime_make(runtime, String);
			ImpString_setRaw(item, dir->d_name);
			Runtime_activate(runtime
				           , context
				           , argv[1]
				           , 1
				           , &item);
		}
		closedir(d);
	}

	return NULL;
}


// lists only files (i.e. not symlinks or directories)
Object *os_filesIn(Runtime *runtime
	             , Object *context
	             , Object *module
	             , int argc
	             , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "os:filesIn requires exactly one argument");
	}
	if(BuiltIn_id(argv[0]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "os:filesIn requires stringified argument");
	}

	Object *r = Runtime_make(runtime, Vector);
	Object_reference(r);

	DIR           *d;
	struct dirent *dir;
	d = opendir(ImpString_getRaw(argv[0]));
	if(d){
		while((dir = readdir(d)) != NULL){
			if(dir->d_type == DT_REG){
				Object *item = Runtime_make(runtime, String);
				ImpString_setRaw(item, dir->d_name);
				Runtime_callMethod(runtime
					             , context
					             , r
					             , "append"
					             , 1
					             , &item);
			}
		}
		closedir(d);
	}

	Object_unreference(r);
	return r;
}


