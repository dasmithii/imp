#include "stdio.h"
#include "parser.h"

int main(int argc, char **argv){
	ParseTree tree;
	ParseTree_init(&tree, "(print -'tom') \n(add 5 -5 'adam')");
	ParseTree_print(&tree);
	// switch(argc){
	// case 1:
	// 	printf("%s\n", "repl");
	// 	break;
	// case 2:
	// 	printf("%s\n", argv[1]);
	// 	break;
	// default:
	// 	printf("%s\n", "invalid arguments");
	// }
}
