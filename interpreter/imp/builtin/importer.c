#include <ctype.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "../commands.h"

#include "general.h"
#include "importer.h"
#include "string.h"


bool ImpImporter_isValid(Object *self){
	return Object_isValid(self) &&
	       BuiltIn_id(self) == BUILTIN_IMPORTER;
}


static bool pathExists(const char *path) {
    struct stat st;
    int result = stat(path, &st);
    return result == 0;
}

static bool isFile(const char *path){
	struct stat s;
	if(stat(path,&s) == 0){
	    if(s.st_mode & S_IFREG){
	        return true;
	    }
	}
	return false;
}

static bool isDirectory(const char *path){
	struct stat s;
	if(stat(path,&s) == 0){
	    if(s.st_mode & S_IFDIR){
	        return true;
	    }
	}
	return false;
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

	char dest[128];
	sprintf(dest, "%s/cache/%lu.so", Imp_root(), checksum);
	if(!pathExists(dest)){
		char buf[512];

		// ensure that cache directory is set up
		sprintf(buf, "mkdir -p %s/cache", Imp_root());
		if(system(buf)){
			Runtime_throwString(runtime, "failed to make .so cache dir");
		}

		// compile .so file
		sprintf(buf, "gcc -std=c99 -Wall -Wextra -g -I /usr/local/imp/headers -shared -o %s -fPIC %s /usr/local/imp/imp.so", dest, path);
		if(system(buf)){
			Runtime_throwFormatted(runtime, "failed to build '%s'", dest);
		}
	}
	return dlopen(dest, RTLD_LAZY);
}


static void removeModuleFileExtention(char *dest){
	int len = strlen(dest);
	if(len > 2 && dest[len-2] == '.' && dest[len-1] == 'c'){
		dest[len-2] = 0;
	} else if(len > 4              &&
	          dest[len-4] == '.'   &&
	          dest[len-3] == 'i'   &&
	          dest[len-2] == 'm'   &&
	          dest[len-1] == 'p'){
		dest[len-4] = 0;
	}
}


static void getNameOfImport(char *dest, char *path){
	// skip all but last path element
	char *lastSlash = strrchr(path, '/');
	if(lastSlash){
		strcpy(dest, lastSlash + 1);
	} else {
		strcpy(dest, path);
	}

	// if extension is .c or .imp, truncate it off
	removeModuleFileExtention(dest);
}

static Object *contextForImportName(Runtime *runtime
	                                          , Object *context
	                                          , char *importName){
	if(Object_hasKeyDeep(context, importName)){
		Runtime_throwFormatted(runtime, "import collision at '%s'", importName);
	}

	Object_reference(context);
	Object *r = Runtime_clone(runtime, context);
	Object_putShallow(context, importName, r);
	Object_unreference(context);
	return r;
}


static Object *contextForImportPath(Runtime *runtime
	                                          , Object *context
	                                          , char *importPath){
	char importName[128];
	getNameOfImport(importName, importPath);
	return contextForImportName(runtime, context, importName);
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

	char importName[128];
	getNameOfImport(importName, path);

	char prefix[128]; *prefix = 0;
	strcat(prefix, "Object *");
	strcat(prefix, importName);
	strcat(prefix, "_");

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

	Object *module_ctx = contextForImportName(runtime, context, importName);

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

	Object *module_ctx = contextForImportPath(runtime, context, path);
	Runtime_executeSourceInContext(runtime, code, module_ctx);

	free(code);
}


static void importPackage(Runtime *runtime
	                    , Object *context
	                    , char *path){
	Object *subcontext = contextForImportPath(runtime, context, path);

	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	if(d){
		char submodule[128];
		while ((dir = readdir(d)) != NULL){
			if(dir->d_name[0] != '.'){
				sprintf(submodule, "%s/%s", path, dir->d_name);
				removeModuleFileExtention(submodule);
				Imp_import(runtime, subcontext, submodule);
			}
		}
		closedir(d);
	}
}


void Imp_import(Runtime *runtime
	          , Object *context
	          , char *module){ // module should not have file extention
	assert(runtime);
	assert(Object_isValid(context));
	assert(module);

	if(*module == 0){
		Runtime_throwString(runtime, "cannot import empty string.");
	}

	if(isDirectory(module)){
		importPackage(runtime, context, module);
		return;
	}

	char buf[128];

	// check <local>.imp
	sprintf(buf, "%s.imp", module);
	if(pathExists(buf)){
		importRegular(runtime, context, buf);
		return;
	}

	// check <local>.c 
	sprintf(buf, "%s.c", module);
	if(pathExists(buf)){
		importInternal(runtime, context, buf);
		return;
	}

	// check <global>.imp
	sprintf(buf, "%s/index/%s.imp", Imp_root(), module);
	if(pathExists(buf)){
		importRegular(runtime, context, buf);
		return;
	}

	// check <global>.c
	sprintf(buf, "%s/index/%s.c", Imp_root(), module);
	if(pathExists(buf)){
		importInternal(runtime, context, buf);
		return;
	}


	Runtime_throwFormatted(runtime, "failed to import '%s' (path does not exist)g", module);
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
