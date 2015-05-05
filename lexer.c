#include "lexer.h"
#include "token.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int Tokenization_init(Tokenization *tokenization, char *code){
	assert(tokenization);
	assert(code);
	tokenization->error = NULL;
	Vector_init(&tokenization->tokens, sizeof(Token));

	// prefix with '('
	const Token beginToken = {.type = TOKEN_SOFT_OPEN};
	Vector_append(&tokenization->tokens, &beginToken);

	Token token;
	bool afterSpace;
	while(*code){
		if(isspace(*code)){
			++code;
			afterSpace = true;
			continue;
		}


		switch(*code){
		case '(':
			token.type = TOKEN_SOFT_OPEN;
			break;
		case ')':
			token.type = TOKEN_SOFT_CLOSE;
			break;
		case '[':
			token.type = TOKEN_HARD_OPEN;
			break;
		case ']':
			token.type = TOKEN_HARD_CLOSE;
			break;
		case '{':
			token.type = TOKEN_CURLY_OPEN;
			break;
		case '}':
			token.type = TOKEN_CURLY_CLOSE;
			break;
		case '!':
			token.type = TOKEN_NOT;
			break;
		case '@':
			token.type = TOKEN_AT;
			break;
		case '#':
			token.type = TOKEN_HASH;
			break;
		case '$':
			token.type = TOKEN_DOLLAR;
			break;
		case '%':
			token.type = TOKEN_PERCENT;
			break;
		case '^':
			token.type = TOKEN_CARROT;
			break;
		case '&':
			token.type = TOKEN_AMP;
			break;
		case '*':
			token.type = TOKEN_STAR;
			break;
		case '-':
			token.type = TOKEN_DASH;
			break;
		case '+':
			token.type = TOKEN_PLUS;
			break;
		case '?':
			token.type = TOKEN_QUESTION;
			break;
		case ':':
			token.type = TOKEN_COLON;
			break;
		case ';':
			token.type = TOKEN_SEMI;
			break;
		default:
			{
				char *end = code + 1;
				if(*code == '\''){
					token.type = TOKEN_STRING;
					++code;
					++end;
					while(*end != '\''){
						++end;
					}
					token.data.text = malloc((end - code) + 1);
					token.data.text[end - code] = 0;
					memcpy(token.data.text, code, (end-code));
					++end;
				} else if(isalpha(*code)){
					token.type = TOKEN_ATOM;
					end = code + 1;
					while(isalnum(*end)){
						++end;
					}
					token.data.text = malloc((end - code) + 1);
					token.data.text[end - code] = 0;
					memcpy(token.data.text, code, (end-code));
				} else if(isdigit(*code) || *code == '.'){
					token.type = TOKEN_NUMBER;
					end = code + 1;
					while(isdigit(*end) || *end == '.'){
						++end;
					}
					token.data.number = strtod(code, &end); // ISSUE
				}
				code = end - 1; // to counteract ** below
				break;
			}
		}
		++code; // **                        **  ^^^
		afterSpace = false;
		Vector_append(&tokenization->tokens, &token);
	}

	const Token endToken = {.type = TOKEN_SOFT_CLOSE};
	Vector_append(&tokenization->tokens, &endToken);
	return 0; // TODO: handle errors
}


static void cleanToken(void *token){
	assert(token);
	Token_clean((Token*) token);
}

	
void Tokenization_clean(Tokenization *tokenization){
	assert(tokenization);
	if(tokenization->error){
		free(tokenization->error);
	}
	Vector_each(&tokenization->tokens, cleanToken);
	Vector_clean(&tokenization->tokens);
}


Tokenization lex(char *code){
	assert(code);
	Tokenization r;
	Tokenization_init(&r, code);
	return r;
}


static void printToken(void *token){
	assert(token);
	Token_printVerbose((Token*) token);
	printf(" ");
}

void Tokenization_print(Tokenization *tokenization){
	assert(tokenization);
	assert(!tokenization->error);
	Vector_each(&tokenization->tokens, printToken);
}

Token *Tokenization_begin(Tokenization *self){
	assert(self);
	return (Token*) self->tokens.buffer.data;
}

Token *Tokenization_end(Tokenization *self){
	assert(self);
	return ((Token*) self->tokens.buffer.data) + self->tokens.size;
}
