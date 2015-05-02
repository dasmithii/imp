#include <stdio.h>
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include "runtime.h"


static const char *const usage = 
  "\tusage:"
"\n\t  imp <file> ---------------- Interpret file."
"\n\t  imp help ------------------ Display this message.\0";


char *readFile(char *path){
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



int main(int argc, char **argv){
	if(argc == 2){
		if(strcmp(argv[1], "help") == 0){
			printf("%s", usage);
		} else{
			char *code = readFile(argv[1]);
			if(!code){
				printf("Failed to read file.\n");
				return -1;
			}
			Runtime runtime;
			Runtime_init(&runtime);
			Runtime_execute(&runtime, code);
		}
	} else{
		printf("%s", usage);
	}
}
