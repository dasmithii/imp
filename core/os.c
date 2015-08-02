#include <string.h>
#include <dirent.h> 
#include <stdio.h> 

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/string.h>
#include <imp/builtin/vector.h>



iObject *os_onImport(iRuntime *runtime
	               , iObject *context
	               , iObject *module
	               , int argc
	               , iObject **argv){
	iObject *obj = iRuntime_MAKE(runtime, Vector);
	iObject_reference(obj);

	Vector *vec = iVector_getRaw(obj);
	for(int i = 0; i < runtime->argc; i++){
		iObject *arg = iRuntime_MAKE(runtime, String);
		iString_setRaw(arg, runtime->argv[i]);
		Vector_append(vec, &arg);
	}
	iObject_putShallow(module, "@", obj);
	iObject_unreference(obj);
	return NULL;
}

// lists only files (i.e. not symlinks or directories)
iObject *os_eachItemIn(iRuntime *runtime
	                 , iObject *context
	                 , iObject *module
	                 , int argc
                     , iObject **argv){
	if(argc != 2){
		iRuntime_throwString(runtime, context, "os:eachFileNameIn requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "os:eachFileNameIn requires String as first argument");
	}
	if(!iObject_canBeActivated(argv[1])){
		iRuntime_throwString(runtime, context, "os:eachFileNameIn requires activate-able object as second argument");
	}

	DIR           *d;
	struct dirent *dir;
	d = opendir(iString_getRaw(argv[0]));
	if(d){
		while((dir = readdir(d)) != NULL){
			iObject *item = iRuntime_MAKE(runtime, String);
			iString_setRaw(item, dir->d_name);
			iRuntime_activate(runtime
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
iObject *os_filesIn(iRuntime *runtime
	              , iObject *context
	              , iObject *module
	              , int argc
	              , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "os:filesIn requires exactly one argument");
	}
	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "os:filesIn requires stringified argument");
	}

	iObject *r = iRuntime_MAKE(runtime, Vector);
	iObject_reference(r);

	DIR           *d;
	struct dirent *dir;
	d = opendir(iString_getRaw(argv[0]));
	if(d){
		while((dir = readdir(d)) != NULL){
			if(dir->d_type == DT_REG){
				iObject *item = iRuntime_MAKE(runtime, String);
				iString_setRaw(item, dir->d_name);
				iRuntime_callMethod(runtime
					             , context
					             , r
					             , "append"
					             , 1
					             , &item);
			}
		}
		closedir(d);
	}

	iObject_unreference(r);
	return r;
}


