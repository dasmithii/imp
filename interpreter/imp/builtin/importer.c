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




bool iImporter_isValid(iObject *self){
	return iObject_isValid(self) &&
	       iBuiltin_id(self) == iBUILTIN_IMPORTER;
}


static bool pathExists(const char *path) {
    struct stat st;
    int result = stat(path, &st);
    return result == 0;
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


static void *fileToDL(iRuntime *runtime, char *path, iObject *context){

	// TODO: if cache is loaded, dump it
	assert(runtime);
	assert(path);

	char *code = readFile(path);
	if(!code){
		iRuntime_throwFormatted(runtime, context, "failed to read file: '%s'", path);
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
			iRuntime_throwString(runtime, context, "failed to make .so cache dir");
		}

		// compile .so file
		sprintf(buf, "gcc -std=c99 -Wall -Wextra -g -I /usr/local/imp/headers -shared -o %s -fPIC %s /usr/local/imp/imp.so", dest, path);
		if(system(buf)){
			iRuntime_throwFormatted(runtime, context, "failed to build '%s'", dest);
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
static void importInternalModuleTo(iRuntime *runtime
	                             , char *path
	                             , iObject *context){
	iObject_reference(context);

	// read source file
	char *code = readFile(path);
	if(!code){
		iRuntime_throwFormatted(runtime, context, "failed to read file: %s", path);
	}

	// load dynamic library
	void *so = fileToDL(runtime, path, context);
	if(!so){
		iRuntime_throwString(runtime, context, "failed to dlopen");
	}

	// where module will be imported to within <context>
	char importName[128];
	getNameOfImport(importName, path);

	// prefix denoting exported functions
	char prefix[128]; *prefix = 0;
	sprintf(prefix, "iObject *%s_", importName);
	const int prefixLen = strlen(prefix);

	// scan through code, loading each symbol prefixed with
	// the above
	char *ptr = code;
	while(ptr && *ptr){
		ptr = strstr(ptr, prefix);
		if(!ptr){
			break;
		}
		ptr += prefixLen;

		char wopre[48];
		char *it = wopre;

		while(ptr && (isalnum(*ptr) || *ptr == '_')){
			*it = *ptr;
			++it;
			++ptr;
		}
		*it = 0;

 
		char full_[64]; *full_ = 0;
		char *full = full_;  // full symbol
		strcat(full, prefix);
		full += strlen("iObject *");
		strcat(full, wopre);

		// load module-level function
		void *sym = dlsym(so, full);
		if(!sym){
			iRuntime_throwFormatted(runtime, context, "failed to find symbol '%s'", full);
		}

		if((*wopre >= 'a' && *wopre <= 'z') || *wopre == '_'){
			if(strcmp(wopre, "activate") == 0){
				iObject_registerCActivator(context, sym);
			} else {
				// special cases
				if(strcmp(wopre, "clone") == 0){
					iRuntime_registerCMethod(runtime, context, "~", sym);
				} else if(strcmp(wopre, "copy") == 0){
					iRuntime_registerCMethod(runtime, context, "$", sym);
				} else {
					iRuntime_registerCMethod(runtime, context, wopre, sym);
				}
			}
			if(endswith(wopre, "onImport")){
				iRuntime_callMethod(runtime, context, context, "onImport", 0, NULL);
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

			iObject *baseObj;

			if(iObject_hasKeyShallow(context, baseKey)){
				baseObj = iObject_getShallow(context, baseKey);
			} else{
				baseObj = iRuntime_MAKE(runtime, Object);
				iObject_putShallow(context, baseKey, baseObj);
			} 

			assert(baseObj);

			char *afterBase = wopre + strlen(baseKey) + 1;
			if(strcmp(afterBase, "activate") == 0){
				iObject_registerCActivator(baseObj, sym);
			} else {
				iRuntime_registerCMethod(runtime, baseObj, afterBase, sym);
			}

			if(endswith(wopre, "onImport")){
				iRuntime_callMethod(runtime, context, baseObj, "onImport", 0, NULL);
			}
		} else {
			iRuntime_throwString(runtime, context, "importer: BAD SYMBOL");
		}
	}


	// dlclose(so); TODO:? make it so module_ctx destructor calls dlclose
	iObject_unreference(context);
	free(code);
} 
      

static void importRegularModuleTo(iRuntime *runtime
	                            , char *path
	                            , iObject *context){
	assert(runtime);
	assert(iObject_isValid(context));
	assert(path);
	iRuntime_executeFileInContext(runtime, path, context);
}


static void importPackageTo(iRuntime *runtime
	                      , char *path
	                      , iObject *context){
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
				iObject_putShallow(context, submoduleName, i_import(runtime, submodulePath, context));
			}
		}
		closedir(d);
	}
}


static iObject *importWithoutUsingCache(iRuntime *runtime, char *modulePath, iObject *context){ // module should not have file extention
	assert(runtime);
	assert(modulePath);

	if(*modulePath == 0){
		iRuntime_throwString(runtime, context, "cannot import empty string.");
	}

	// Create import context.
	iObject *r = iRuntime_MAKE(runtime, Object);
	iObject_reference(r); // permanent reference (modules aren't ever collected)

	char buf[256]; // buffer for trying different extentions

	// Try importing local package.
	if(isDirectory(modulePath)){
		importPackageTo(runtime, modulePath, r);
		return r;
	}

	// Try importing global package.
	sprintf(buf, "%s/index/%s", runtime->root, modulePath);
	if(isDirectory(buf)){
		importPackageTo(runtime, buf, r);
		return r;
	}

	// Try importing local .imp file.
	sprintf(buf, "%s.imp", modulePath);
	if(pathExists(buf)){
		importRegularModuleTo(runtime, buf, r);
		return r;
	}

	// Try importing local .c file.
	sprintf(buf, "%s.c", modulePath);
	if(pathExists(buf)){
		importInternalModuleTo(runtime, buf, r);
		return r;
	}

	// Try importing global .imp file.
	sprintf(buf, "%s/index/%s.imp", runtime->root, modulePath);
	if(pathExists(buf)){
		importRegularModuleTo(runtime, buf, r);
		return r;
	}

	// Try importing global .c file.
	sprintf(buf, "%s/index/%s.c", runtime->root, modulePath);
	if(pathExists(buf)){
		importInternalModuleTo(runtime, buf, r);
		return r;
	}

	iRuntime_throwFormatted(runtime, context, "failed to import '%s' (path does not exist)", modulePath);
	return NULL;
}


iObject *i_import(iRuntime *runtime, char *modulePath, iObject *context){
	if(runtime->imports){
		iObject *cached = iObject_getShallow(runtime->imports, modulePath);
		if(cached){
			return cached;
		}
	} else {
		runtime->imports = iRuntime_rawObject(runtime);
		iObject_reference(runtime->imports);
	}
	iObject *r = importWithoutUsingCache(runtime, modulePath, context);
	iObject_putShallow(runtime->imports, modulePath, r);
	return r;
}


static iObject *activate_(iRuntime *runtime
	                   , iObject *context
	                   , iObject *caller
	                   , int argc
	                   , iObject **argv){
	assert(runtime);
	assert(iObject_isValid(context));

	if(argc != 1 && argc != 2){
		iRuntime_throwString(runtime, context, "import requires one or two arguments.");
	} else if(!iString_isValid(argv[0])){
		iRuntime_throwString(runtime, context, "import requires a string as its first argument.");
	}

	char *modulePath = iString_getRaw(argv[0]);
	iObject *module = i_import(runtime, modulePath, context);

	if(argc == 1){ //
		char importName[32];
		getNameOfImport(importName, modulePath);
		iObject_putShallow(context, importName, module);
		return module;
	} else { // copy module contents to specified
		iObject *dest = argv[1];

		// check that to-context import is possible
		for(int i = 0; i < module->slotCount; i++){
			iSlot *slot = module->slots + i;
			if(iSlot_isPrimitive(slot)){
				if(strcmp(slot->key, "__onImport") != 0  &&     
			       strcmp(slot->key, "__referenceCount") != 0){
			       	iRuntime_throwFormatted(runtime, context, "failed to import '%s' into context (has internal method)", modulePath);
				}
			} else if(iObject_hasKeyShallow(dest, slot->key) &&
				      slot->data != iObject_getShallow(module, slot->key)){
				iRuntime_throwFormatted(runtime, context, "failed to import '%s' into context because of conflict '%s'", modulePath, slot->key);
			}
		}

		// it is; transfer
		for(int i = 0; i < module->slotCount; i++){
			iSlot *slot = module->slots + i;
			if(iSlot_isPrimitive(slot)){
				continue;
			}
			if(strcmp(slot->key, "#") != 0){
				iObject_putShallow(dest, slot->key, iSlot_object(slot));
			}
		}

		return dest;
	}
}


void iImporter_init(iObject *self, iRuntime *runtime){
	assert(self);
	iBuiltin_setId(self, iBUILTIN_IMPORTER);
	runtime->imports = iRuntime_MAKE(runtime, Object);
	iObject_putShallow(runtime->root_scope, "imports", runtime->imports);
	iObject_registerCActivator(self, activate_);
}
