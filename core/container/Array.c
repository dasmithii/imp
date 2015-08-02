#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/general.h>
#include <imp/builtin/number.h>
#include <imp/builtin/importer.h>
#include <imp/builtin/string.h>
#include <string.h>




static iObject **getBuffer(iObject *array){
	return (iObject**) iObject_getDataDeep(array, "__data");
}

static int getSize(iObject *array){
	return iNumber_getRawRounded(iObject_getDeep(array, "size"));
}


iObject *Array_at(iRuntime *runtime
	            , iObject *context
	            , iObject *self
	            , int argc
	            , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Array:at requires exactly 1 argument");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "Array:at requires numeric index");
	}

	int size = getSize(self);
	int i = iNumber_getRawRounded(argv[0]);

	if(i < 0 || i >= size){
		iRuntime_throwFormatted(runtime, context, "Array:at index out of bounds %d/%d", i, size);
	}

	return getBuffer(self)[i];
}


static iObject *atEq_(iRuntime *runtime
	                , iObject *context
	                , iObject *self
	                , int argc
	                , iObject **argv){
	if(argc != 2){
		iRuntime_throwString(runtime, context, "Array:at= requires exactly 2 arguments");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "Array:at= requires numeric index");
	}

	int size = getSize(self);
	int i = iNumber_getRawRounded(argv[0]);

	if(i < 0 || i >= size){
		iRuntime_throwFormatted(runtime, context, "Array:at= index out of bounds %d/%d", i, size);
	}

	getBuffer(self)[i] = argv[1];
	return NULL;
}


iObject *Array_resizeTo(iRuntime *runtime
	                                        , iObject *context
	                                        , iObject *self
	                                        , int argc
	                                        , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Array:resize requires exactly 1 arguments");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "Array:resize requires a numeric index");
	}

	int oldSize = getSize(self);
	int newSize = iNumber_getRawRounded(argv[0]);

	if(newSize < 0){
		iRuntime_throwString(runtime, context, "Array:resize requires non-negative argument");
	}

	if(newSize == 0){
		iObject_putDataDeep(self, "__data", NULL);
	} else {
		iObject **oldBuffer = getBuffer(self);
		iObject **newBuffer = malloc(newSize * sizeof(iObject*));  // TODO: use realloc here
		if(!newBuffer){
			iRuntime_throwFormatted(runtime, context, "Array:resize failed to allocate buffer of length %s", newSize);
		}

		memcpy(newBuffer, oldBuffer, oldSize * sizeof(iObject*));
		for(int i = oldSize; i < newSize; i++){
			newBuffer[i] = runtime->nil;
		}
		iObject_putDataDeep(self, "__data", newBuffer); // implicitly frees oldBuffer
	}

	iNumber_setRaw(iObject_getDeep(self, "size"), (double) newSize);

	return NULL;
}


iObject *Array__markInternalsRecursively(iRuntime *runtime
	                                   , iObject *context
	                                   , iObject *self
	                                   , int argc
	                                   , iObject **argv){
	if(argc != 0){
		iRuntime_throwString(runtime, context, "Array:mark does not accept arguments");
	}

	iObject **buffer = getBuffer(self);
	if(buffer){
		const size_t size = getSize(self);
		for(size_t i = 0; i < size; i++){
			if(buffer[i]){
				iRuntime_callMethod(runtime, NULL, buffer[i], "_markRecursively", 0, NULL);
			}
		}
	}
	return NULL;
}



iObject *Array_withSize(iRuntime *runtime
	                   , iObject *context
	                   , iObject *Array
	                   , int argc
	                   , iObject **argv){
	if(argc != 1){
		iRuntime_throwString(runtime, context, "Array:withSize requires exactly one argument");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_NUMBER){
		iRuntime_throwString(runtime, context, "Array:withSize requires numeric argument");
	}
	const int size = iNumber_getRawRounded(argv[0]);

	if(size < 0){
		iRuntime_throwString(runtime, context, "Array:withSize requires non-negative argument");
	}

	iObject *r = iRuntime_simpleClone(runtime, Array);
	iObject_reference(r);

	{
		iObject *sizeiObject = iRuntime_MAKE(runtime, Number);
		iNumber_setRaw(sizeiObject, (double) size);
		iObject_putShallow(r, "size", sizeiObject);
	}

	iObject **data = NULL;
	if(size){
		data = malloc(size * sizeof(iObject*));
		if(!data){
			iRuntime_throwString(runtime, context, "Array:withSize ... malloc failed");
		}
		for(int i = 0; i < size; i++){
			data[i] = runtime->nil;
		}
	}
	iObject_putDataShallow(r, "__data", data);

	iObject_unreference(r);
	return r;
}


iObject *Array_withContents(iRuntime *runtime
	                      , iObject *context
	                      , iObject *Array
	                      , int argc
	                      , iObject **argv){
	iObject *r = iRuntime_simpleClone(runtime, Array);
	iObject_reference(r);

	iObject *size = iRuntime_MAKE(runtime, Number);
	iNumber_setRaw(size, (double) argc);
	iObject_putShallow(r, "size", size);

	iObject **data = calloc(argc, sizeof(iObject*));
	if(!data){
		iRuntime_throwString(runtime, context, "Array:withSize ... malloc failed");
	}
	memcpy(data, argv, argc * sizeof(iObject*));
	iObject_putDataShallow(r, "__data", data);

	iObject_unreference(r);
	return r;
}


iObject *Array_onImport(iRuntime *runtime
	                  , iObject *context
	                  , iObject *Array
	                  , int argc
	                  , iObject **argv){
	iObject_putShallow(Array, "#", i_import(runtime, "core/container/abstract/Sequence", context));
	iRuntime_registerCMethod(runtime, Array, "at=", atEq_);
	return NULL;
}
