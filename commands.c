#include "commands.h"
#include "runtime.h"
#include <stdlib.h>
#include <stdio.h>

#include "toolbox/queue.h"

static char *root = "/usr/local/imp";

void Imp_debugMode(bool status){

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


void Imp_fetchPackage(char *source){
	printf("TODO: not yet implemented.");
}


void Imp_indexPackage(char *dest, char *src){	
	// figure out if src is 
	printf("TODO: not yet implemented.");
}


void Imp_removePackage(char *id){
	char command[64];
	sprintf(command, "rm -rf %s/%s", Imp_root(), id);
	int rc = system(command);
	exit(rc);
}


void Imp_updatePackage(char *id){
	printf("TODO: not yet implemented.");
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





