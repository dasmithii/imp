#include "importer.h"
#include "general.h"

#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <dlfcn.h>
#include <time.h>

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

unsigned long hash(unsigned char *str) {
    unsigned long hash = 5381;
    int c;
    while(*str){
    	c = *str;
    	str++;
    	hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}


static void *fileToDL(Runtime *runtime, char *path){

	// TODO: if cache is loaded, dump it
	assert(runtime);
	assert(path);

	char *code = readFile(path);
	if(!code){
		Runtime_throwFormatted(runtime, "failed to read file '%s'", path);
	}
	unsigned long checksum = hash((unsigned char*) code);
	free(code);
	code = NULL;

	char dest[64];
	sprintf(dest, "%s/cache/%ld.so", Imp_root(), checksum);
	if(!fileExists(dest)){
		char buf[128];

		// ensure that cache directory is set up
		sprintf(buf, "mkdir -p %s/cache", Imp_root());
		if(system(buf)){
			Runtime_throwString(runtime, "failed to make .so cache dir");
		}

		// compile .so file
		sprintf(buf, "gcc -shared -o %s -fPIC %s", dest, path);
		if(system(buf)){
			Runtime_throwFormatted(runtime, "failed to build '%s'", dest);
		}
	}
	return dlopen(dest, RTLD_LAZY);
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
	}

	// load dynamic library
	void *so = fileToDL(runtime, path);
	if(!so){
		Runtime_throwString(runtime, "failed to dlopen");
	}


	Object_reference(context);
	Object *module_ctx = Runtime_clone(runtime, context);
	Object_putShallow(context, name, module_ctx);


	for(int i = 0; i < symbols.size; i++){
		char *wopre = Vector_hook(&symbols, i); // without prefix
 
		char full_[64]; *full_ = 0;
		char *full = full_;  // full symbol
		strcat(full, prefix);
		full += strlen("Object *");
		strcat(full, wopre);


		// load module-level function
		void *sym = dlsym(so, full);
		if(!sym){
			Runtime_throwFormatted(runtime, "failed to find symbol '%s'", full);
		}


		if(*wopre >= 'a' && *wopre <= 'z'){
			char key[64]; *key = 0;
			strcat(key, "__");
			strcat(key, wopre);

			Object_registerCMethod(module_ctx, key, sym);
		} else if(*wopre >= 'A' && *wopre <= 'Z'){
			// load method (and its base object, if not yet loaded)
			// TODO:

			char baseKey[32];
			for(int i = 0;; i++){
				if(wopre[i] == 0 || wopre[i] == '_'){
					baseKey[i] = 0;
					break;
				}
				baseKey[i] = wopre[i];
			}

			char key[64]; *key = 0;
			strcat(key, "__");
			strcat(key, wopre + strlen(baseKey) + 1);
			

			if(!Object_hasKeyShallow(module_ctx, baseKey)){
				Object_putShallow(module_ctx, baseKey, Runtime_rawObject(runtime));
			}

			Object *baseObj = Object_getShallow(module_ctx, baseKey);
			assert(baseObj);



			Object_registerCMethod(baseObj, key, sym);
		} else {
			Runtime_throwString(runtime, "BAD SYMBOL");
		}
	}



	Object_unreference(context);


	// dlclose(so); TODO: make it so module_ctx destructor calls dlclose


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
		Runtime_throwFormatted(runtime, "failed to read file '%s'", path);
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
		Runtime_throwFormatted(runtime, "module name '%s' would overide existing variable", name);
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

	char buf[128];

	// check local .imp
	sprintf(buf, "%s.imp", module);
	if(fileExists(buf)){
		importRegular(runtime, context, buf);
		return;
	}

	// check local .c 
	sprintf(buf, "%s.c", module);
	if(fileExists(buf)){
		importInternal(runtime, context, buf);
		return;
	}

	// check global .imp
	sprintf(buf, "%s/index/%s.imp", Imp_root(), module);
	if(fileExists(buf)){
		importRegular(runtime, context, buf);
		return;
	}

	// check global.c
	sprintf(buf, "%s/index/%s.c", Imp_root(), module);
	if(fileExists(buf)){
		importInternal(runtime, context, buf);
		return;
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
