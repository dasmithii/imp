#include <assert.h>
#include <stdio.h>

#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/string.h>
#include <imp/builtin/general.h>
#include <imp/builtin/route.h>




static FILE *File_getRaw(iObject *self){
	assert(iObject_isValid(self));
	FILE **fpp = iObject_getDataDeep(self, "__data");
	if(!fpp){
		return NULL;
	}
	return *fpp;
}

static void File_setRaw(iObject *self, FILE *fp){
	assert(iObject_isValid(self));
	FILE **fpp = iObject_getDataDeep(self, "__data");
	if(fpp){
		*fpp = fp;
	} else {
		fpp = malloc(sizeof(FILE*));
		if(!fpp){
			abort();
		}
		*fpp = fp;
		iObject_putDataShallow(self, "__data", fpp);
	}
}


iObject *io_File_open(iRuntime *runtime
	                , iObject *context
	                , iObject *caller
	                , int argc
	                , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(iObject_isValid(caller));
	
	if(argc != 2){
		iRuntime_throwString(runtime, context, "File:open requires exactly two arguments (path and mode)");
	}

	if(iBuiltin_id(argv[0]) != iBUILTIN_STRING || iBuiltin_id(argv[1]) != iBUILTIN_STRING){
		iRuntime_throwString(runtime, context, "File:open requires strings as its arguments");
	}

	char *path = iString_getRaw(argv[0]);
	char *mode = iString_getRaw(argv[1]);

	FILE *fp = fopen(path, mode);
	if(!fp){
		iRuntime_throwFormatted(runtime, context, "file '%s' could not be opened in mode '%s'", path, mode);
	}

	iObject *r = iRuntime_clone(runtime, caller);
	File_setRaw(r, fp);

	return r;
}



iObject *io_File_close(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(caller));
	assert(iObject_isValid(context));
	if(argc != 0){
		iRuntime_throwString(runtime, context,  "File:close does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		iRuntime_throwString(runtime, context, "attempted to close non-open file");
	}
	fclose(fp);
	File_setRaw(caller, NULL);
	return NULL;
}

iObject *io_File_readCharacter(iRuntime *runtime
	                         , iObject *context
	                         , iObject *caller
	                         , int argc
	                         , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(caller));
	assert(iObject_isValid(context));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "File:readCharacter does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		iRuntime_throwString(runtime, context, "attempted to read from unopened file");
	}

	char buf[2];
	buf[0] = getc(fp);
	buf[1] = 0;

	iObject *r = iRuntime_MAKE(runtime, String);
	iString_setRaw(r, buf);
	return r;
}


iObject *io_File_readLine(iRuntime *runtime
	                    , iObject *context
	                    , iObject *caller
	                    , int argc
	                    , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(caller));
	assert(iObject_isValid(context));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "File:readLine does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		iRuntime_throwString(runtime, context, "attempted to read from unopened file");
	}

	iObject *r = iRuntime_MAKE(runtime, String);
	char *line = NULL;
		size_t len = 0;
		ssize_t read;
		read = getline(&line, &len, fp);
	if(read == -1){
		iRuntime_throwString(runtime, context, "failed to getline");
	}
	iString_setRaw(r, line);
	free(line);
	return r;
}


iObject *io_File_read(iRuntime *runtime
	                , iObject *context
	                , iObject *caller
	                , int argc
	                , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(caller));
	assert(iObject_isValid(context));

	if(argc != 0){
		iRuntime_throwString(runtime, context, "File:read does not accept arguments");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		iRuntime_throwString(runtime, context, "attempted to read from unopened file");
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

	iObject *r = iRuntime_MAKE(runtime, String);
	iString_setRaw(r, contents);
	free(contents);

	return r;
}



iObject *io_File_write(iRuntime *runtime
	                 , iObject *context
	                 , iObject *caller
	                 , int argc
	                 , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(caller));

	if(argc == 0){
		iRuntime_throwString(runtime, context, "File:write requires at least one argument");
	}

	FILE *fp = File_getRaw(caller);
	if(!fp){
		iRuntime_throwString(runtime, context, "tried writing to nonexistent file");
	}

	for(int i = 0; i < argc; i++){
		if(!argv[i]){
			fprintf(fp, "NULL");
		} else if(iBuiltin_id(argv[i]) == iBUILTIN_STRING){
			fprintf(fp, "%s", iString_getRaw(argv[i]));
		} else if(iObject_hasMethod(argv[i], "asString")){
			iObject *s = iRuntime_callMethod(runtime, context, argv[i], "asString", 0, NULL);
			if(iBuiltin_id(s) != iBUILTIN_STRING){
				iRuntime_throwString(runtime, context, "asString method did not return string");
			}
			fprintf(fp, "%s", iString_getRaw(s));
		} else if(iObject_hasKeyDeep(argv[i], "asString")){
			iObject *as = iObject_getDeep(argv[i], "asString");
			if(iBuiltin_id(as) != iBUILTIN_STRING){
				iRuntime_throwString(runtime, context, "asString field was not string");
			}
			fprintf(fp, "%s", iString_getRaw(as));
		} else {
			iRuntime_throwString(runtime, context, "File:write given unwritable parameter(s)");
		}

		if(i != argc - 1){
			fprintf(fp, " ");
		}
	}
	return NULL;
}


iObject *io_File_writeLine(iRuntime *runtime
	                     , iObject *context
	                     , iObject *caller
	                     , int argc
	                     , iObject **argv){
	if(argc > 0){
		io_File_write(runtime, context, caller, argc, argv);
	}
	FILE *fp = File_getRaw(caller);
	fprintf(fp, "\n");
	return NULL;
}


iObject *io_readCharacter(iRuntime *runtime
	                    , iObject *context
	                    , iObject *io
	                    , int argc
                        , iObject **argv){
	return io_File_read(runtime
		              , context
		              , iObject_getShallow(io, "stdin")
		              , argc
		              , argv);
}

iObject *io_readLine(iRuntime *runtime
	               , iObject *context
	               , iObject *io
	               , int argc
                   , iObject **argv){
	return io_File_readLine(runtime
		                  , context
		                  , iObject_getShallow(io, "stdin")
		                  , argc
		                  , argv);
}

iObject *io_write(iRuntime *runtime
	            , iObject *context
	            , iObject *io
	            , int argc
                , iObject **argv){
	return io_File_write(runtime
		               , context
		               , iObject_getShallow(io, "stdout")
		               , argc
		               , argv);
}

iObject *io_writeLine(iRuntime *runtime
	                , iObject *context
	                , iObject *io
	                , int argc
                    , iObject **argv){
	return io_File_writeLine(runtime
		                   , context
		                   , iObject_getShallow(io, "stdout")
		                   , argc
		                   , argv);
}


iObject *io_onImport(iRuntime *runtime
	               , iObject *context
	               , iObject *io
	               , int argc
                   , iObject **argv){
	iObject *baseFile = iObject_getShallow(io, "File");

	iObject *i = iRuntime_clone(runtime, baseFile); // clone base file type
	File_setRaw(i, stdin);
	iObject_putShallow(io, "stdin", i);
	iObject_reference(i);

	iObject *o = iRuntime_clone(runtime, baseFile);
	File_setRaw(o, stdout);
	iObject_putShallow(io, "stdout", o);
	iObject_reference(o); 

	return NULL;
}
