#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "commands.h"
#include "runtime.h"

static char *root = "/usr/local/imp";




void Imp_debugMode(bool status){
	// TODO
}


void Imp_useRoot(char *path){
	root = path;
}


char *Imp_root(){
	return root;
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


void Imp_launchREPL(){
	Runtime runtime;
	Runtime_init(&runtime);

	for(;;){
		char code[164];
		printf(" > ");
		fgets(code, 164, stdin);
		Runtime_executeSource(&runtime, code);
	}
}


void Imp_executeString(char *code){
	Runtime runtime;
	Runtime_init(&runtime);
	Runtime_executeSource(&runtime, code);
}


void Imp_executeFile(char *path){
	char *contents = readFile(path);
	if(!contents){
		printf("Failed to read file %s.\n", path);
		exit(1);
	}
	Imp_executeString(contents);
}


void Imp_fetch(char *source){
	printf("TODO: not yet implemented.");
	// TODO: either fetch tarbal or do git pull
}


static bool fileExists(const char *path) {
    struct stat st;
    int result = stat(path, &st);
    return result == 0;
}


void Imp_index(char *dest, char *src){
	char full_dest[64];
	sprintf(full_dest, "%s/%s/%s", Imp_root(), "index", dest);

	if(fileExists(full_dest)){
		printf("Code is already indexed at that destination. Please choose another destination, or delete the existing code first.");
		exit(1); 
	}


	struct stat s;
	if(stat(src, &s) == 0){
		char command[128];
	    if(s.st_mode & S_IFDIR){
	    	// make necessary directories
	    	sprintf(command, "mkdir -p %s", full_dest);
	    	if(system(command)){
	    		printf("Failed to create directories.\n");
	    		exit(1);
	    	}
	    	// copy package contents
	    	sprintf(command, "cp -r %s %s", src, full_dest);
	        if(system(command)){
	        	printf("Failed to copy package contents.");
	        	exit(1);
	        }
	        return;
	    } else if(s.st_mode & S_IFREG){
	    	// make necessary directories
	    	char root[128];
	    	strcpy(root, full_dest);
	    	char *end = strrchr(root, '/');
	    	if(end){
	    		*end = 0;
	    		sprintf(command, "mkdir -p %s", root);
		    	if(system(command)){
		    		printf("Failed to create directories.\n");
		    		exit(1);
		    	}
	    	}

	    	// copy module contents
	    	sprintf(command, "cp %s %s", src, full_dest);
	        if(system(command)){
	        	printf("Failed to copy module contents.");
	        	exit(1);
	        }
	    	return;
	    }
	}
	abort();
}


void Imp_remove(char *id){
	char command[128];
	sprintf(command, "rm -rf %s/%s/%s", Imp_root(), "index", id);
	if(system(command)){
		printf("Failed to remove indexed code.\n");
		exit(1);
	}
}


void Imp_update(char *id){
	printf("TODO: not yet implemented.");
	// TODO: if git repo, pull
	//       if std, pull
}


void Imp_installProject(char *root){
	printf("TODO: not yet implemented.");
}


void Imp_printEnvironment(){
	printf("\n\tImp environment:"
	     "\n\t\troot:     /usr/local/imp"
	     "\n\t\tversion:  0.0.1"
	     "\n\n");
}

