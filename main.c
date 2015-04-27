#include "stdio.h"
#include "lexer.h"

int main(int argc, char **argv){
	Tokenization t = lex("(add 5 -5 'adam')");
	Tokenization_print(&t);
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
