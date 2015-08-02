#ifndef IMP_LEXER_H_
#define IMP_LEXER_H_

#include <imp/token.h>




typedef struct {
	char file[256];
	iToken *buffer;
	size_t size;
	char *error;
} iTokenization;


// interface
iTokenization lex(char *code);
void iTokenization_clean(iTokenization *self);


// miscellaneous
void iTokenization_print(iTokenization *tokenization);
bool iIsValidRouteChar(char c);
bool iIsValidRouteBegin(char c);
bool iIsValidRouteText(char *text);




#endif