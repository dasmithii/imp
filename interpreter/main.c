#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"




static void iargs(char *message){
	fprintf(stderr, "Invalid arguments: %s\n", message);
	fprintf(stderr, "   - use 'imp help' for more info.");
	exit(1);
}


int main(int argc, char **argv){
	char *root = "/usr/local/imp";
	int c;
	opterr = 0;

	while((c = getopt(argc, argv, "r::")) != -1){
		switch (c){
		case 'r':
			root = optarg;
			break;
		case '?':
			if(isprint(optopt)){
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			} else{
				fprintf(stderr
					  , "Unknown option character `\\x%x'.\n"
					  , optopt);
			}
			return 1;
		default:
			abort();
		}
	}

	argc -= optind;
	argv += optind;

	if(argc == 0){ 
		Imp_launchREPL();
	} else if(strcmp("index", argv[0]) == 0){
		if(argc != 3){
			iargs("index requires source and destination paths");
		}
		Imp_index(argv[1], argv[2]);
	} else if(strcmp("remove", argv[0]) == 0){
		if(argc != 2){
			iargs("remove requires module specifier");
		}
		Imp_remove(argv[1]);
	} else if(strcmp("help", argv[0]) == 0){
		Imp_printUsage();
	} else if(strcmp("environment", argv[0]) == 0){
		Imp_printEnvironment();
	} else {
		Imp_executeFile(argv[0], argc-1, argv+1);
	}
}