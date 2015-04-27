#ifndef IMP_LEXER_H_
#define IMP_LEXER_H_
#include "toolbox/vector.h"


typedef struct {
	Vector tokens;
	char *error;
} Tokenization;

int Tokenization_init(Tokenization *tokenization, char *code);
void Tokenization_clean(Tokenization *tokenization);
void Tokenization_print(Tokenization *tokenization);
Tokenization lex(char *code);



#endif