#ifndef IMP_LEXER_H_
#define IMP_LEXER_H_

#include <imp/token.h>




typedef struct {
	Token *buffer;
	size_t size;
	char *error;
} Tokenization;


// interface
Tokenization lex(char *code);
void Tokenization_clean(Tokenization *self);


// miscellaneous
void Tokenization_print(Tokenization *tokenization);
bool isValidRouteChar(char c);
bool isValidRouteBegin(char c);
bool isValidRouteText(char *text);




#endif