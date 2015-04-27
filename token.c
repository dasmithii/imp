#include "token.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

void Token_clean(Token *token) {
	assert(token);
	if(token->type == TOKEN_ATOM || token->type == TOKEN_COMMENT){
		assert(token->data.text);
		free(token->data.text);
		token->data.text = NULL;
	}
}


void Token_print(Token *token){
	switch(token->type){
	case TOKEN_ATOM:
		printf("%s", token->data.text);
		break;
	case TOKEN_STRING:
		printf("\"%s\"", token->data.text);
		break;
	case TOKEN_NUMBER:
		printf("%f", token->data.number);
		break;
	case TOKEN_HARD_OPEN:
		printf("[");
		break;
	case TOKEN_HARD_CLOSE:
		printf("]");
		break;
	case TOKEN_SOFT_OPEN:
		printf("(");
		break;
	case TOKEN_SOFT_CLOSE:
		printf(")");
		break;
	case TOKEN_CURLY_OPEN:
		printf("{");
		break;
	case TOKEN_CURLY_CLOSE:
		printf("}");
		break;
	case TOKEN_COMMENT:
		// TODO
		break;
	case TOKEN_NOT:
		printf("!");
		break;
	case TOKEN_AT:
		printf("@");
		break;
	case TOKEN_HASH:
		printf("#");
		break;
	case TOKEN_DOLLAR:
		printf("$");
		break;
	case TOKEN_PERCENT:
		printf("%c", '%');
		break;
	case TOKEN_CARROT:
		printf("^");
		break;
	case TOKEN_AMP:
		printf("^");
		break;
	case TOKEN_STAR:
		printf("&");
		break;
	case TOKEN_PLUS:
		printf("+");
		break;
	case TOKEN_QUESTION:
		printf("?");
		break;
	case TOKEN_COLON:
		printf(":");
		break;
	case TOKEN_SEMI:
		printf(";");
		break;
	case TOKEN_DASH:
		printf("-");
		break;
	}
}


void Token_printType(Token *token){
	switch(token->type){
	case TOKEN_ATOM:
		printf("atom");
		break;
	case TOKEN_NUMBER:
		printf("number");
		break;
	case TOKEN_STRING:
		printf("string");
		break;
	case TOKEN_COMMENT:
		printf("comment");
		break;
	default:
		Token_print(token);
		break;
	}
}


void Token_printVerbose(Token *token){
	Token_printType(token);
	if(token->type == TOKEN_ATOM     ||
	   token->type == TOKEN_NUMBER   ||
	   token->type == TOKEN_STRING   ||
	   token->type == TOKEN_COMMENT){
	   printf(";");
	   Token_print(token);
	}
}