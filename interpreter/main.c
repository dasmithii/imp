#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "commands.h"




static void iargs(char *message){
	fprintf(stderr, "Invalid arguments: %s.\n", message);
	fprintf(stderr, "   - use 'imp help' for more info.");
	exit(1);
}


int main(int argc, char **argv){


	static struct option lopts[] = {
		{"index", 1, 0, 'i'},
		{"unindex", 1, 0, 'u'},
		{"help", 0, 0, 'h'},
	    {"version", 0, 0, 'v'},

	    {"destination", 1, 0, 'd'},

	    {"root", 1, 0, 'r'},
	    {"from-root", 0, 0, 'f'},
	    {NULL, 0, NULL, 0}
	};


	char *root = "/usr/local/imp";
	int c;
	opterr = 0;

	int opti = 0;

	char *opt_i = NULL;
	char *opt_u = NULL;
	bool  opt_h = false;
	bool  opt_v = false;
	char *opt_d = NULL;
	bool  opt_f = false;

	while((c = getopt_long(argc, argv, "i:u:hvd:r:f", lopts, &opti)) != -1){
		switch (c){
		case 'i': opt_i = strdup(optarg);
			break;
		case 'u': opt_u = strdup(optarg);
			break;
		case 'h': opt_h = true;
			break;
		case 'v': opt_v = true;
			break;
		case 'd': opt_d = strdup(optarg);
			break;
		case 'r': root = strdup(optarg);
			break;
		case 'f': opt_f = true;
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

	if(((opt_i? 1:0) + (opt_u? 1:0) + opt_h + opt_v > 1)  || (opt_d && !opt_i)){
		iargs("invalid option combination");
	} else if(opt_i){
		char *d = opt_d? opt_d:opt_i;
		Imp_index(d, opt_i);
	} else if(opt_u){
		Imp_remove(opt_u);
	} else if(opt_h){
		Imp_printUsage();
	} else if(opt_v){
		Imp_printEnvironment();
	} else if(argc == 0){
		Imp_launchREPL();
	} else {
		if(opt_f){
			char fp[256];
			sprintf(fp, "%s/index/%s", root, argv[0]);
			Imp_executeFile(fp, argc-1, argv+1);
		} else {
			Imp_executeFile(argv[0], argc-1, argv+1);
		}
	}
}