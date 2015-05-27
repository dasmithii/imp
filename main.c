#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"




static const char *const intro = 
"\tImp - the programming language.";

static const char *const usage = 
  "\tusage:"
"\n\t  imp [repl] --------------- Launch REPL."
"\n\t  imp <path> --------------- Interpret file."
"\n\t  imp execute <code> ------- Interpret string."
"\n\t  imp fetch <url> ---------- Download remote package to local registry."
"\n\t  imp index <path> <name> -- Include package in local registry."
"\n\t  imp remove <package> ----- Remove package from local registry."
"\n\t  imp update <package> ----- Git pull package in local registry."
"\n\t  imp install <project> ---- Install project globally."
"\n\t  imp environment ---------- Print configuration."
"\n\t  imp help ----------------- Display more information.";

static const char *const options = 
  "\toptions:"
"\n\t  -root <path> ------------- Specify local package registry."
"\n\t  -debug ------------------- Run in debug mode.";




void print_usage(){
	printf("\n%s\n\n%s\n\n%s\n\n", intro, usage, options);
}


void invalid_args(char *message){
	printf("Invalid arguments: %s\n", message);
	printf("\n");
	print_usage();
	exit(1);
}


int main(int argc, char **argv){
	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			if(strcmp(argv[i], "-root") == 0){
				if(i == argc - 1){
					invalid_args("root not specified.");
				}
				Imp_useRoot(argv[i + 1]);
				for(int j = i; j < argc - 1; j++){
					argv[j] = argv[j+1];
				}
				for(int j = i; j < argc - 1; j++){
					argv[j] = argv[j+1];
				}
				argc -= 2;
			} else if(strcmp(argv[i], "-debug") == 0){
				Imp_debugMode(true);
				for(int j = i; j < argc - 1; j++){
					argv[j] = argv[j+1];
				}
				argc--;
			} else {
				invalid_args("invalid option.");
			}
		}
	}

	if(argc == 1){
		Imp_launchREPL();
	} else if(strcmp(argv[1], "execute") == 0){
		if(argc == 2){
			Imp_executeString(argv[2]);
		} else {
			invalid_args("'execute' accepts exactly one parameter.");
		}
	} else if(strcmp(argv[1], "repl") == 0){
		if(argc == 2){
			Imp_launchREPL();
		} else {
			invalid_args("'repl' doesn't accept parameters.");
		}
	} else if(strcmp(argv[1], "fetch") == 0){
		if(argc == 3){
			Imp_fetch(argv[2]);
		} else {
			invalid_args("'fetch' accepts exactly one parameter.");
		}
	} else if(strcmp(argv[1], "index") == 0){
		if(argc == 4){
			Imp_index(argv[2], argv[3]);
		} else {
			invalid_args("'index' accepts exactly two parameters.");
		}
	} else if(strcmp(argv[1], "remove") == 0){
		if(argc == 3){
			Imp_remove(argv[2]);
		} else {
			invalid_args("'remove' accepts exactly one parameter.");
		}
	} else if(strcmp(argv[1], "update") == 0){
		if(argc == 3){
			Imp_fetch(argv[2]);
		} else {
			invalid_args("'update' accepts exactly one parameter.");
		}
	} else if(strcmp(argv[1], "install") == 0){
		if(argc == 3){
			Imp_installProject(argv[2]);
		} else {
			invalid_args("'install' accepts exactly one parameter.");
		}
	} else if(strcmp(argv[1], "environment") == 0){
		if(argc != 2){
			invalid_args("'environment' does not accept parameters.");
		} else{
			Imp_printEnvironment();
		}
	} else if(strcmp(argv[1], "help") == 0){
		if(argc == 2){
			print_usage();
		} else if (argc == 3){
			invalid_args("'help' accepts no arguments.");
		}
	} else {
		if(argc == 2){
			Imp_executeFile(argv[1]);
		} else {
			invalid_args("!!!");
		}
	}
}
