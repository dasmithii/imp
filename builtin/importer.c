#include "importer.h"
#include "general.h"

#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <dlfcn.h>

#include "../commands.h"
#include "string.h"
#include "builtin/general.h"


bool ImpImporter_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_IMPORTER;
}


static bool fileExists(const char *path) {
    struct stat st;
    int result = stat(path, &st);
    return result == 0;
}

static char *readFile(char *path){
	assert(path);
	FILE *stream;
	char *contents;
	size_t fileSize = 0;
	stream = fopen(path, "rb");
	if(!stream){
		return NULL;
	}
	fseek(stream, 0L, SEEK_END);
	fileSize = ftell(stream);
	fseek(stream, 0L, SEEK_SET);
	contents = malloc(fileSize+1);
	size_t size=fread(contents,1,fileSize,stream);
	contents[size]=0; 
	fclose(stream);
	return contents;
}




// Internal modules provide an imp interface to C code. They
// are useful when high performance is required and/or for
// wrapping existing C libraries.
//
// An internal module consists primarily of one .c file and 
// the headers it includes. Soon, multi-file modules will
// be supported. But for now, this is the way.
static void importInternal(Runtime *runtime
	                , Object *context
	                , char *path){
	char *code = readFile(path);
	if(!code){
		Runtime_throwFormatted(runtime, "failed to read file: %s", path);
	}

	char name[32]; *name = 0;
	strcat(name, path);
	strchr(name, '.')[0] = 0;

	printf("%s\n", name);


	char prefix[64]; *prefix = 0;
	strcat(prefix, "Object *");
	strcat(prefix, path);
	char *end = strchr(prefix, '.');
	end[0] = '_';
	end[1] = 0;

	printf("%s\n", prefix);


	const int prefixLen = strlen(prefix);

	Vector symbols;
	Vector_init(&symbols, 48);	

	char *ptr = code;
	while(ptr && *ptr){
		ptr = strstr(ptr, prefix);
		if(!ptr){
			break;
		}
		ptr += prefixLen;

		char symbol[48];
		char *it = symbol;


		while(ptr && (isalnum(*ptr) || *ptr == '_')){
			*it = *ptr;
			++it;
			++ptr;
		}
		*it = 0;
		Vector_append(&symbols, symbol);
		printf(" - detected symbol: %s\n", symbol);
	}


	// compile .so file
	char command[128];
	sprintf(command, "gcc -shared -o %s.so -fPIC %s", name, path);
	if(system(command)){
		Runtime_throwFormatted(runtime, "failed to build %s.so", name);
	}


	// load .so and...
	char sopath[32]; *sopath = 0;
	strcat(sopath, name);
	strcat(sopath, ".so");
	void *so = dlopen(sopath, RTLD_LAZY);
	if(!so){
		Runtime_throwFormatted(runtime, "failed to dlopen %s", sopath);
	}


	Object_reference(context);
	Object *module_ctx = Runtime_clone(runtime, context);
	Object_putShallow(context, name, module_ctx);


	for(int i = 0; i < symbols.size; i++){
		char *wopre = Vector_hook(&symbols, i);

		char full_[64]; *full_ = 0;
		char *full = full_;
		strcat(full, prefix);
		full += strlen("Object *");
		strcat(full, wopre);
		if(*wopre >= 'a' && *wopre <= 'z'){
			// load module-level function
			void *sym = dlsym(so, full);
			if(!sym){
				Runtime_throwFormatted(runtime, "failed to find symbol '%s'", full);
			}
			char key[64]; *key = 0;
			strcat(key, "__");
			strcat(key, wopre);

			Object_registerCMethod(module_ctx, key, sym);
		} else if(*wopre >= 'A' && *wopre <= 'Z'){
			// load method (and its base object, if not yet loaded)
			// TODO:
			Runtime_throwString(runtime, "NYI");
		} else {
			Runtime_throwString(runtime, "BAD SYMBOL");
		}
	}



	Object_unreference(context);


	// dlclose(so);


	Vector_clean(&symbols);
	// gather types




	free(code);
} 
       

static void importRegular(Runtime *runtime
	               , Object *context
	               , char *path){
	assert(runtime);
	assert(Object_isValid(context));
	assert(path);

	const int plen = strlen(path);

	char *code = readFile(path);
	if(!code){
		Runtime_throwString(runtime, "failed to read file.");
	}

	char name[64];
	char *ptr = name;
	for(int i = 0; i < plen; i++){
		if(path[i] == '/'){
			*name = 0;
			ptr = name;
		} else if(path[i] == '.'){
			*ptr = 0;
			break;
		} else {
			*ptr = path[i];
			++ptr;
		}
	}
	*ptr = 0;

	if(Object_hasKeyShallow(context, name)){
		Runtime_throwString(runtime, "module name would overide existing variable.");
	}

	Object_reference(context);
	Object *module_ctx = Runtime_clone(runtime, context);
	Object_putShallow(context, name, module_ctx);
	Runtime_executeSourceInContext(runtime, code, module_ctx);
	Object_unreference(context);
	free(code);
}


void Imp_import(Runtime *runtime
	          , Object *context
	          , char *module){
	assert(runtime);
	assert(Object_isValid(context));
	assert(module);

	if(*module == 0){
		Runtime_throwString(runtime, "cannot import empty string.");
	}

	char buf[64];

	// check <name>.imp
	*buf = 0;
	strcat(buf, module);
	strcat(buf, ".imp");
	if(fileExists(buf)){
		importRegular(runtime, context, buf);
	}

	// check <name>.c
	*buf = 0;
	strcat(buf, module);
	strcat(buf, ".c");
	if(fileExists(buf)){
		importInternal(runtime, context, buf);
	}

	// check <root>/<name>.imp
	*buf = 0;
	strcat(buf, Imp_root());
	strcat(buf, module);
	strcat(buf, ".imp");
	if(fileExists(buf)){
		importRegular(runtime, context, buf);
	}

	// check <root>/<name>.imp
	*buf = 0;
	strcat(buf, Imp_root());
	strcat(buf, module);
	strcat(buf, ".c");
	if(fileExists(buf)){
		importInternal(runtime, context, buf);
	}
}

static Object *ImpImporter_activate_internal(Runtime *runtime
	                                       , Object *context
	                                       , Object *caller
	                                       , int argc
	                                       , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));
	assert(ImpImporter_isValid(caller));

	if(argc != 1){
		Runtime_throwString(runtime, "import requires exactly one argument.");
	} else if(!ImpString_isValid(argv[0])){
		Runtime_throwString(runtime, "import requires a string as its first argument.");
	} else {
		char *module = ImpString_getRaw(argv[0]);
		Imp_import(runtime, context, module); // TODO: handle import errors
	}

	return NULL;
}


void ImpImporter_init(Object *self){
	assert(self);
	BuiltIn_setId(self, BUILTIN_IMPORTER);
	Object_registerCMethod(self, "__activate", ImpImporter_activate_internal);
}
