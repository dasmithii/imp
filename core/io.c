#include <imp/runtime.h>
#include <imp/object.h>
#include <imp/builtin/string.h>
#include <stdio.h>

// Reads character from reader given as first 
// argument. Stdin is used when no argument is
// provided.
Object *io_readCharacter(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc > 1){
		Runtime_throwString(runtime, "readCharacter accepts at most one argument");
	}

	if(argc == 0){
		Object *r = Runtime_cloneField(runtime, "string");
		char buf[2];
		buf[0] = getchar();
		buf[1] = 0;
		ImpString_setRaw(r, buf);
		return r;
	} else {
		return Runtime_callMethod(runtime, context, argv[0], "readCharacter", 0, NULL);
	}

	return NULL;
}




Object *io_readLine(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc > 1){
		Runtime_throwString(runtime, "readCharacter accepts at most one argument");
	}

	if(argc == 0){
		Object *r = Runtime_cloneField(runtime, "string");
		char *line = NULL;
  		size_t len = 0;
  		ssize_t read;
  		read = getline(&line, &len, stdin);
		if(read == -1){
			Runtime_throwString(runtime, "failed to getline");
		}
		ImpString_setRaw(r, line);
		free(line);
		return r;
	} else {
		return Runtime_callMethod(runtime, context, argv[0], "readLine", 0, NULL);
	}

	return NULL;
}


