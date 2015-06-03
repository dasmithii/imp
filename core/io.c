#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/string.h>
#include <imp/builtin/general.h>
#include <imp/builtin/route.h>
#include <stdio.h>



static FILE *File_getRaw(Object *self){
	assert(Object_isValid(self));
	FILE **fpp = Object_getDataDeep(self, "__data");
	if(!fpp){
		return NULL;
	}
	return *fpp;
}

static void File_setRaw(Object *self, FILE *fp){
	assert(Object_isValid(self));
	FILE **fpp = Object_getDataDeep(self, "__data");
	if(fpp){
		*fpp = fp;
	} else {
		fpp = malloc(sizeof(FILE*));
		if(!fpp){
			abort();
		}
		*fpp = fp;
		Object_putDataShallow(self, "__data", fpp);
	}
}


Object *io_File_open(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(Object_isValid(caller));
	
	if(argc != 2){
		Runtime_throwString(runtime, "File:open requires exactly two arguments (path and mode)");
	}

	if(BuiltIn_id(argv[0]) != BUILTIN_STRING || BuiltIn_id(argv[1]) != BUILTIN_STRING){
		Runtime_throwString(runtime, "File:open requires strings as its arguments");
	}

	char *path = ImpString_getRaw(argv[0]);
	char *mode = ImpString_getRaw(argv[1]);

	FILE *fp = fopen(path, mode);
	if(!fp){
		Runtime_throwFormatted(runtime, "file '%s' could not be opened in mode '%s'", path, mode);
	}

	Object *r = Runtime_clone(runtime, caller);
	File_setRaw(r, fp);

	return r;
}



Object *io_File_close(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));
	assert(Object_isValid(context));
	if(argc != 0){
		Runtime_throwString(runtime, "File:close does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		Runtime_throwString(runtime, "attempted to close non-open file");
	}
	fclose(fp);
	File_setRaw(caller, NULL);
	return NULL;
}

Object *io_File_readCharacter(Runtime *runtime
	                        , Object *context
	                        , Object *caller
	                        , int argc
	                        , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));
	assert(Object_isValid(context));

	if(argc != 0){
		Runtime_throwString(runtime, "File:readCharacter does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		Runtime_throwString(runtime, "attempted to read from unopened file");
	}

	char buf[2];
	buf[0] = getc(fp);
	buf[1] = 0;

	Object *r = Runtime_cloneField(runtime, "String");
	ImpString_setRaw(r, buf);
	return r;
}


Object *io_File_readLine(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));
	assert(Object_isValid(context));

	if(argc != 0){
		Runtime_throwString(runtime, "File:readLine does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		Runtime_throwString(runtime, "attempted to read from unopened file");
	}

	Object *r = Runtime_cloneField(runtime, "String");
	char *line = NULL;
		size_t len = 0;
		ssize_t read;
		read = getline(&line, &len, fp);
	if(read == -1){
		Runtime_throwString(runtime, "failed to getline");
	}
	ImpString_setRaw(r, line);
	free(line);
	return r;
}


Object *io_File_read(Runtime *runtime
	               , Object *context
	               , Object *caller
	               , int argc
	               , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));
	assert(Object_isValid(context));

	if(argc != 0){
		Runtime_throwString(runtime, "File:read does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		Runtime_throwString(runtime, "attempted to read from unopened file");
	}

	char *contents;
	size_t fileSize = 0;
	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	contents = malloc(fileSize+1);
	if(!contents){
		abort();
	}
	size_t size=fread(contents,1,fileSize,fp);
	contents[size]=0; 

	Object *r = Runtime_cloneField(runtime, "String");
	ImpString_setRaw(r, contents);
	free(contents);

	return r;
}



Object *io_File_write(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	assert(runtime);
	assert(Object_isValid(caller));
	assert(Object_isValid(context));

	if(argc == 0){
		Runtime_throwString(runtime, "File:write requires at least one argument");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		Runtime_throwString(runtime, "tried writing to nonexistent file");
	}

	for(int i = 0; i < argc; i++){
		if(!argv[i]){
			fprintf(fp, "NULL");
		} else if(BuiltIn_id(argv[i]) == BUILTIN_STRING){
			fprintf(fp, "%s", ImpString_getRaw(argv[i]));
		} else if(Object_hasMethod(argv[i], "asString")){
			Object *s = Runtime_callMethod(runtime, context, argv[i], "asString", 0, NULL);
			if(BuiltIn_id(s) != BUILTIN_STRING){
				Runtime_throwString(runtime, "asString method did not return string");
			}
			fprintf(fp, "%s", ImpString_getRaw(s));
		} else {
			Runtime_throwString(runtime, "File:write given unwritable parameter(s)");
		}

		if(i != argc - 1){
			fprintf(fp, " ");
		}
	}
	return NULL;
}


Object *io_File_writeLine(Runtime *runtime
	                , Object *context
	                , Object *caller
	                , int argc
	                , Object **argv){
	if(argc > 0){
		io_File_write(runtime, context, caller, argc, argv);
	}
	FILE *fp = File_getRaw(caller);
	fprintf(fp, "\n");
	return NULL;
}


Object *io_onImport(Runtime *runtime
	              , Object *context
	              , Object *io
	              , int argc
                  , Object **argv){
	Object *baseFile = Object_getShallow(io, "File");

	Object *i = Runtime_clone(runtime, baseFile); // clone base file type
	File_setRaw(i, stdin);
	Object_putShallow(io, "stdin", i);
	Object_reference(i);

	Object *o = Runtime_clone(runtime, baseFile);
	File_setRaw(o, stdout);
	Object_putShallow(io, "stdout", o);
	Object_reference(o); 

	return NULL;
}
