#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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
	if(!pathExists(path)){
		return false;
	}
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
		Runtime_throwFormatted(runtime, "failed to read file: '%s'", path);
	}
	unsigned long checksum = hash((unsigned char*) code);
	free(code);
	code = NULL;

	char dest[128];
	sprintf(dest, "%s/cache/%lu.so", runtime->root, checksum);
	if(!pathExists(dest)){
		char buf[512];

		// ensure that cache directory is set up
		sprintf(buf, "mkdir -p %s/cache", runtime->root);
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
	Object *r = Runtime_simpleClone(runtime, context);
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


static bool endswith(char *whole, char *part){
	int wholeLen = strlen(whole);
	int partLen = strlen(part);
	if(wholeLen < partLen){
		return false;
	}
	return strcmp(whole + wholeLen - partLen, part) == 0;
}

// Internal modules provide an imp interface to C code. They
// are useful when high performance is required and/or for
// wrapping existing C libraries.
//
// An internal module consists primarily of one .c file and 
// the headers it includes. Soon, multi-file modules will
// be supported. But for now, this is the way.
static void importInternalModuleTo(Runtime *runtime
	                , char *path
	                , Object *context){
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

		if((*wopre >= 'a' && *wopre <= 'z') || *wopre == '_'){
			if(strcmp(wopre, "activate") == 0){
				Object_registerCActivator(context, sym);
			} else {
				Runtime_registerCMethod(runtime, context, wopre, sym);
			}
			if(endswith(wopre, "onImport")){
				Runtime_callMethod(runtime, context, context, "onImport", 0, NULL);
			}
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

			Object *baseObj;

			if(Object_hasKeyShallow(context, baseKey)){
				baseObj = Object_getShallow(context, baseKey);
			} else{
				baseObj = Runtime_make(runtime, Object);
				Object_putShallow(context, baseKey, baseObj);
			} 

			assert(baseObj);

			char *afterBase = wopre + strlen(baseKey) + 1;
			if(strcmp(afterBase, "activate") == 0){
				Object_registerCActivator(baseObj, sym);
			} else {
				Runtime_registerCMethod(runtime, baseObj, afterBase, sym);
			}

			if(endswith(wopre, "onImport")){
				Runtime_callMethod(runtime, context, baseObj, "onImport", 0, NULL);
			}
		} else {
			Runtime_throwString(runtime, "BAD SYMBOL");
		}
	}

	Object_unreference(context);

	// dlclose(so); TODO:? make it so module_ctx destructor calls dlclose

	Vector_clean(&symbols);
	free(code);
} 
      

static void importRegularModuleTo(Runtime *runtime
	                            , char *path
	                            , Object *context){
	assert(runtime);
	assert(Object_isValid(context));
	assert(path);

	char *code = readFile(path);
	if(!code){
		Runtime_throwFormatted(runtime, "failed to read file '%s'", path);
	}

	Runtime_executeSourceInContext(runtime, code, context);

	free(code);
}


static void importPackageTo(Runtime *runtime
	                      , char *path
	                      , Object *context){
	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	if(d){
		char submoduleName[128];
		char submodulePath[256];
		while ((dir = readdir(d)) != NULL){
			if(dir->d_name[0] != '.'){
				getNameOfImport(submoduleName, dir->d_name);
				sprintf(submodulePath, "%s/%s", path, dir->d_name);
				removeModuleFileExtention(submodulePath);
				Object_putShallow(context, submoduleName, Imp_import(runtime, submodulePath));
			}
		}
		closedir(d);
	}
}


static Object *importWithoutUsingCache(Runtime *runtime, char *modulePath){ // module should not have file extention
	assert(runtime);
	assert(modulePath);

	if(*modulePath == 0){
		Runtime_throwString(runtime, "cannot import empty string.");
	}

	Object *r = Runtime_make(runtime, Object);
	Object_reference(r); // permanent reference (modules aren't ever collected)

	if(isDirectory(modulePath)){
		importPackageTo(runtime, modulePath, r);
		return r;
	}

	char buf[128];

	// check <local>.imp
	sprintf(buf, "%s.imp", modulePath);
	if(pathExists(buf)){
		importRegularModuleTo(runtime, buf, r);
		return r;
	}

	// check <local>.c 
	sprintf(buf, "%s.c", modulePath);
	if(pathExists(buf)){
		importInternalModuleTo(runtime, buf, r);
		return r;
	}

	// check <global>.imp
	sprintf(buf, "%s/index/%s.imp", runtime->root, modulePath);
	if(pathExists(buf)){
		importRegularModuleTo(runtime, buf, r);
		return r;
	}

	// check <global>.c
	sprintf(buf, "%s/index/%s.c", runtime->root, modulePath);
	if(pathExists(buf)){
		importInternalModuleTo(runtime, buf, r);
		return r;
	}


	Runtime_throwFormatted(runtime, "failed to import '%s' (path does not exist)", modulePath);
	return NULL;
}


Object *Imp_import(Runtime *runtime, char *modulePath){
	if(runtime->imports){
		Object *cached = Object_getShallow(runtime->imports, modulePath);
		if(cached){
			return cached;
		}
	} else {
		runtime->imports = Runtime_rawObject(runtime);
		Object_reference(runtime->imports);
	}
	Object *r = importWithoutUsingCache(runtime, modulePath);
	Object_putShallow(runtime->imports, modulePath, r);
	return r;
}


static Object *activate_(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv){
	assert(runtime);
	assert(Object_isValid(context));

	if(argc != 1 && argc != 2){
		Runtime_throwString(runtime, "import requires one or two arguments.");
	} else if(!ImpString_isValid(argv[0])){
		Runtime_throwString(runtime, "import requires a string as its first argument.");
	}

	char *modulePath = ImpString_getRaw(argv[0]);
	Object *module = Imp_import(runtime, modulePath);

	if(argc == 1){ //
		char importName[32];
		getNameOfImport(importName, modulePath);
		Object_putShallow(context, importName, module);
		return module;
	} else { // copy module contents to specified
		Object *dest = argv[1];

		// check that to-context import is possible
		for(int i = 0; i < module->slotCount; i++){
			Slot *slot = module->slots + i;
			if(Slot_isPrimitive(slot)){
				if(strcmp(slot->key, "__onImport") != 0  &&     
			       strcmp(slot->key, "__referenceCount") != 0){
			       	Runtime_throwFormatted(runtime, "failed to import '%s' into context (has internal method)", modulePath);
				}
			} else if(Object_hasKeyShallow(dest, slot->key) &&
				      slot->data != Object_getShallow(module, slot->key)){
				Runtime_throwFormatted(runtime, "failed to import '%s' into context because of conflict '%s'", modulePath, slot->key);
			}
		}

		// it is; transfer
		for(int i = 0; i < module->slotCount; i++){
			Slot *slot = module->slots + i;
			if(Slot_isPrimitive(slot)){
				continue;
			}
			if(strcmp(slot->key, "#") != 0){
				Object_putShallow(dest, slot->key, Slot_object(slot));
			}
		}

		return dest;
	}
}


void ImpImporter_init(Object *self, Runtime *runtime){
	assert(self);
	BuiltIn_setId(self, BUILTIN_IMPORTER);
	runtime->imports = Runtime_make(runtime, Object);
	Object_putShallow(runtime->root_scope, "imports", runtime->imports);
	Object_registerCActivator(self, activate_);
}
