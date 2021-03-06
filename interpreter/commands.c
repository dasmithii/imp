#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <signal.h>
#include <setjmp.h>

#include <imp/runtime.h>
#include <imp/c.h>

#include "commands.h"
#include "version.h"

static char *root = "/usr/local/imp";




void Imp_useRoot(char *path){
	root = path;
}


static bool readCommand(char **toVar){
	size_t used = 0;
	size_t cap = 256;
	*toVar = realloc(*toVar, cap);
	**toVar = 0;
	if(*toVar == NULL){
		perror("Failed to malloc command buffer.");
		return false;
	}

	// Read lines until we are at a line break and the
	// grouping depth is 0.
	size_t depth = 0;
	size_t nopen = 0;
	bool first = true;
	do {
		if(first){
			first = false;
		} else if(isatty(fileno(stdin))){
			printf("   ");
		}
		#define MAX_LINE_LENGTH 1024
		char line[MAX_LINE_LENGTH];
		if(!fgets(line, MAX_LINE_LENGTH, stdin)){
			// reached EOF
			return false;
		}

		// Skip shebang lines.
		if(*line == '#'){
			continue;
		}

		// Update depth. Skip comments.
		char *cp = line;
		while(*cp){
			const char c = *cp;
			if(c == '(' || c == '{' || c == '['){
				depth++; nopen++;
			} else if(c == ')' || c == '}' || c == ']'){
				depth--;
			} else if(c == '/' && cp[1] == '/'){
				break;
			}
			cp++;
		}

		// Lengthen command buffer if necessary. Concatenate line 
		// onto it.
		used += strlen(line);
		if(used >= cap){
			cap = used * 2;
			*toVar = realloc(*toVar, cap);
			if(*toVar == NULL){
				perror("failed to realloc command buffer");
				abort();
			}
		}

		strcat(*toVar, line);
	} while(depth > 0 || nopen == 0);

	return true;
}


static sigjmp_buf intbuf;


static void onsigint(int sig){
	signal(sig, SIG_IGN);
	printf("\n");
	signal(SIGINT, onsigint);
	longjmp(intbuf, 1);
}


static bool esc = false;

static iObject *activator_(iRuntime *runtime
	                     , iObject *context
	                     , iObject *self
	                     , int argc
	                     , iObject **argv){
	char *command = NULL;

	for(;;){
		if(isatty(fileno(stdin))){
			printf(" > ");
		}

		if(!readCommand(&command)){
			esc = true;
			break;
		}
		iRuntime_executeSource(runtime, command);
	}

	if(command){
		free(command);
	}
	return NULL;
}


void Imp_launchREPL(void){
	iRuntime runtime;
	iRuntime_init((iRuntime*) &runtime, root, 0, NULL);

	iObject *block = iRuntime_MAKE(&runtime, Closure);
	iRuntime_registerCMethod(&runtime
	                       , block
                           , "_activate"
                           , activator_);


	iRuntime_executeSource((iRuntime*) &runtime, "(import 'core/exceptions')");
	iRuntime_executeSource((iRuntime*) &runtime, "(import 'core/io')");
	iObject_putShallow((&runtime)->root_scope, "_repl", block);


	if(isatty(fileno(stdin))){
		printf("\n"
			   "   Imp %s  - Welcome!\n"
			   "   Use CTRL-D to exit and CTRL-C to abandon a line.\n\n", IMP_VERSION);

		if(signal(SIGINT, onsigint) == SIG_ERR){
			perror("failed to set signal handler");
			abort();
		}
	}

	for(;;){
		if(!setjmp(intbuf)){

			iRuntime_executeSource((iRuntime*)&runtime, ""
				"(exceptions:try {"
				"	(_repl)"
	    		"} {io:writeLine 'Uncaught exception:' (@:at 0) '!!!'})");

			if(esc){
				break;
			}
		}
 
		// TODO: check for exceptions
		// TODO: print output if not null
	}
}


void Imp_executeFile(char *path, int argc, char **argv){
	iRuntime runtime;
	iRuntime_init(&runtime, root, argc, argv);
	iRuntime_executeFile(&runtime, path);
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


void Imp_printEnvironment(void){
	printf("\n\tImp environment:"
	     "\n\t\troot:     %s"
	     "\n\t\tversion:  %s"
	     "\n\n", root, IMP_VERSION);
}


void Imp_printUsage(void){
	const char *const intro = 
	"\tImp - the programming language.";

	const char *const usage = 
	"\tcommands:"
	"\n\t	--------------------------- Launch REPL."
	"\n\t	<path> -------------------- Interpret file."
	"\n\t	-i, --index <module> ------ Include local module in registry."
	"\n\t	-u, --unindex <module> ---- Remove package from registry."
	"\n\t	-h, --help ---------------- Display this text."
	"\n\t	-v, --version ------------- Check version and stuff.";

	const char *const options = 
	"\toptions:"
	"\n\t	-r, --root <path> --------- Specify package registry location."
	"\n\t	-f, --from-root ----------- Prefix given filepath with ${root}/index."
	"\n\t	-d, --destination <x> ----- Specify index/<x> destination.";


	printf("\n%s\n\n%s\n\n%s\n\n", intro, usage, options);
}

