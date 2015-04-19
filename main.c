#include "stdio.h"

int main(int argc, char **argv){
	switch(argc){
	case 1:
		printf("%s\n", "repl");
		break;
	case 2:
		printf("%s\n", argv[1]);
		break;
	default:
		printf("%s\n", "invalid arguments");
	}
}
