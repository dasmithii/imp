#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/string.h>
#include <string.h>




static Object **getBuffer(Object *array){
	return (Object**) Object_getDataDeep(array, "__data");
}

static int getSize(Object *array){
	return ImpNumber_getRawRounded(Object_getDeep(array, "size"));
}



Object *Array_clone(Runtime *runtime
	              , Object *context
	              , Object *base
	              , int argc
	              , Object **argv){
	if(argc != 0){
		Runtime_throwString(runtime, "Array:~ does not accept arguments");
	}

	Object *r = Runtime_simpleClone(runtime, base);
	Object_reference(r);

	Object *size = Runtime_make(runtime, Number);
	ImpNumber_setRaw(size, 0);
	Object_putShallow(r, "size", size);
	Object_putDataShallow(r, "__data", NULL);

	Object_unreference(r);
	return r;
}


Object *Array_at(Runtime *runtime
	           , Object *context
	           , Object *self
	           , int argc
	           , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Array:at requires exactly 1 argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Array:at requires numeric index");
	}

	int size = getSize(self);
	int i = ImpNumber_getRawRounded(argv[0]);

	if(i < 0 || i >= size){
		Runtime_throwFormatted(runtime, "Array:at index out of bounds %d/%d", i, size);
	}

	return getBuffer(self)[i];
}


static Object *atEq_(Runtime *runtime
	            , Object *context
	            , Object *self
	            , int argc
	            , Object **argv){
	if(argc != 2){
		Runtime_throwString(runtime, "Array:at= requires exactly 2 arguments");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Array:at= requires numeric index");
	}

	int size = getSize(self);
	int i = ImpNumber_getRawRounded(argv[0]);

	if(i < 0 || i >= size){
		Runtime_throwFormatted(runtime, "Array:at= index out of bounds %d/%d", i, size);
	}

	getBuffer(self)[i] = argv[1];
	return NULL;
}


Object *Array_resizeTo(Runtime *runtime
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
	int newSize = ImpNumber_getRawRounded(argv[0]);

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


Object *Array__mark(Runtime *runtime
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
			if(buffer[i]){
				Runtime_markRecursive(runtime, buffer[i]);
			}
		}
	}
	return NULL;
}



Object *Array_withSize(Runtime *runtime
	             , Object *context
	             , Object *Array
	             , int argc
	             , Object **argv){
	if(argc != 1){
		Runtime_throwString(runtime, "Array:withSize requires exactly one argument");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_NUMBER){
		Runtime_throwString(runtime, "Array:withSize requires numeric argument");
	}
	const int size = ImpNumber_getRawRounded(argv[0]);

	if(size < 0){
		Runtime_throwString(runtime, "Array:withSize requires non-negative argument");
	}

	Object *r = Runtime_simpleClone(runtime, Array);
	Object_reference(r);

	{
		Object *sizeObject = Runtime_make(runtime, Number);
		ImpNumber_setRaw(sizeObject, (double) size);
		Object_putShallow(r, "size", sizeObject);
	}

	Object **data = NULL;
	if(size){
		data = calloc(size, sizeof(Object*));
		if(!data){
			Runtime_throwString(runtime, "Array:withSize ... malloc failed");
		}
	}
	Object_putDataShallow(r, "__data", data);

	Object_unreference(r);
	return r;
}


Object *Array_withContents(Runtime *runtime
	                     , Object *context
	                     , Object *Array
	                     , int argc
	                     , Object **argv){
	Object *r = Runtime_simpleClone(runtime, Array);
	Object_reference(r);

	Object *size = Runtime_make(runtime, Number);
	ImpNumber_setRaw(size, (double) argc);
	Object_putShallow(r, "size", size);

	Object **data = calloc(argc, sizeof(Object*));
	if(!data){
		Runtime_throwString(runtime, "Array:withSize ... malloc failed");
	}
	memcpy(data, argv, argc * sizeof(Object*));
	Object_putDataShallow(r, "__data", data);

	Object_unreference(r);
	return r;
}


Object *Array_onImport(Runtime *runtime
	                 , Object *context
	                 , Object *Array
	                 , int argc
	                 , Object **argv){
	Object_putShallow(Array, "#", Imp_import(runtime, "core/container/Sequence"));
	Runtime_registerCMethod(runtime, Array, "at=", atEq_);
	return NULL;
}