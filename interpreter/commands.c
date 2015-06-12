#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <imp/runtime.h>

#include "commands.h"
#include "version.h"

static char *root = "/usr/local/imp";




void Imp_useRoot(char *path){
	root = path;
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
	Runtime_init(&runtime, root, 0, NULL);

	for(;;){
		char input[128];
		printf(" > ");
		fgets(input, 128, stdin);
		Runtime_executeSource(&runtime, input);
		// TODO: check for exceptions
	}
}


void Imp_executeFile(char *path, int argc, char **argv){
	char *contents = readFile(path);
	if(!contents){
		printf("Failed to read file %s.\n", path);
		exit(1);
	}
	Runtime runtime;
	Runtime_init(&runtime, root, argc, argv);
	Runtime_executeSource(&runtime, contents);
	// TODO: check for exceptions
}


static bool fileExists(const char *path) {
    struct stat st;
    int result = stat(path, &st);
    return result == 0;
}


void Imp_index(char *dest, char *src){
	char full_dest[64];
	sprintf(full_dest, "%s/%s/%s", root, "index", dest);


	if(fileExists(full_dest)){
		fprintf(stderr, "Code is already indexed at that destination. Please choose another destination, or delete the existing code first.");
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
	    	sprintf(command, "cp -r %s/* %s", src, full_dest);
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
	sprintf(command, "rm -rf %s/%s/%s", root, "index", id);
	if(system(command)){
		printf("Failed to remove indexed code.\n");
		exit(1);
	}
}


void Imp_printEnvironment(){
	printf("\n\tImp environment:"
	     "\n\t\troot:     %s"
	     "\n\t\tversion:  v%s"
	     "\n\n", root, IMP_VERSION);
}


void Imp_printUsage(){
	const char *const intro = 
	"\tImp - the programming language.";

	const char *const usage = 
	"\tusage:"
	"\n\t	imp ---------------------- Launch REPL."
	"\n\t	imp <path> --------------- Interpret file."
	"\n\t	imp index <path> <as> ---- Include local module in registry."
	"\n\t	imp remove <id> ---------- Remove package from registry."
	"\n\t	imp install <id> --------- Install project globally."
	"\n\t	imp help ----------------- Display this text."
	"\n\t	imp environment ---------- Check version and stuff.";

	const char *const options = 
		"\toptions:"
	"\n\t	-r <path> ---------------- Specify package registry location.";

	printf("\n%s\n\n%s\n\n%s\n\n", intro, usage, options);
}

