#ifndef IMP_LEXER_H_
#define IMP_LEXER_H_

#include <imp/toolbox/vector.h>
#include <imp/token.h>




typedef struct {
	Vector tokens;
	char *error;
} Tokenization;


// interface
Tokenization lex(char *code);


// management
int Tokenization_init(Tokenization *tokenization, char *code);
void Tokenization_clean(Tokenization *tokenization);


// miscellaneous
void Tokenization_print(Tokenization *tokenization);
Token *Tokenization_begin(Tokenization *self);
Token *Tokenization_end(Tokenization *self);


#endif