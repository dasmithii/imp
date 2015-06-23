#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>




static Object **getBuffer(Object *array){
	return (Object**) Object_getDataDeep(array, "__data");
}

static int getSize(Object *array){
	return ImpNumber_getRawRounded(Object_getDataDeep(array, "size"));
}



Object *Array_clone(Runtime *runtime
	              , Object *context
	              , Object *base
	              , int argc
	              , Object **argv){
	Object *r = Runtime_make(runtime, Object);
	Object_reference(r);

	Object *size = Runtime_make(runtime, Number);
	ImpNumber_setRaw(size, (double) argc);
	Object_putShallow(r, "size", size);

	Object **data = NULL;
	if(argc){
		data = calloc(argc, sizeof(Object*));
		if(!data){
			Runtime_throwFormatted(runtime, "failed to allocate array of size %d", argc);
		}
		for(int i = 0; i < argc; i++){
			data[i] = argv[i];
		}
	}
	Object_putDataShallow(r, "__data", data);

	Object_unreference(r);
	return r;
}


Object *Array_get(Runtime *runtime
	            , Object *context
	            , Object *self
	            , int argc
	            , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Array:get requires exactly 1 argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Array:get requires numeric index");
	}

	int size = getSize(self);
	int i = ImpNumber_getAsInt(argv[0]);

	if(i < 0 || i >= size){
		Runtime_throwFormatted(runtime, "Array:get index out of bounds %d/%d", i, size);
	}

	return getBuffer(self)[i];
}


Object *Array_set(Runtime *runtime
	            , Object *context
	            , Object *self
	            , int argc
	            , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "Array:set requires exactly 2 arguments");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Array:set requires numeric index");
	}

	int size = getSize(self);
	int i = ImpNumber_getAsInt(argv[0]);

	if(i < 0 || i >= size){
		Runtime_throwFormatted(runtime, "Array:set index out of bounds %d/%d", i, size);
	}

	getBuffer(self)[i] = argv[1];
	return NULL;
}


Object *Array_resize(Runtime *runtime
	               , Object *context
	               , Object *self
	               , int argc
	               , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Array:resize requires exactly 1 arguments");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Array:resize requires a numeric index");
	}

	int oldSize = getSize(self);
	int newSize = ImpNumber_getAsInt(argv[0]);

	if(newSize < 0){
		Runtime_throwString(runtime, "Array:resize requires non-negative argument");
	}

	if(newSize == 0){
		Object_putDataDeep(self, "__data", NULL);
	} else {
		Object **oldBuffer = getBuffer(self);
		Object **newBuffer = realloc(oldBuffer, newSize * sizeof(Object*));
		if(!newBuffer){
			Runtime_throwFormatted(runtime, "Array:resize failed to allocate buffer of length %s", newSize);
		}
		if(newSize > oldSize){
			for(int i = oldSize; i < newSize; i++){
				newBuffer[i] = NULL;
			}
		}
	}

	ImpNumber_setRaw(Object_getDeep(self, "size"), (double) newSize);

	return NULL;
}


Object *Array_mark(Runtime *runtime
	             , Object *context
	             , Object *self
	             , int argc
	             , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Array:mark does not accept arguments");
	}

	Object **buffer = getBuffer(self);
	if(buffer){
		const size_t size = getSize(self);
		for(int i = 0; i < size; i++){
			Runtime_markRecursive(runtime, buffer[i]);
		}
	}
}





