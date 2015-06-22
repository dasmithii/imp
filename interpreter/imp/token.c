#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


#include <imp/token.h>




void Token_clean(Token *token) {
	assert(token);
	if(token->type == TOKEN_ROUTE   ||
	   token->type == TOKEN_COMMENT || 
	   token->type == TOKEN_STRING){
		free(token->data.text);
		token->data.text = NULL;
	}
}


void Token_print(Token *token){
	assert(token);
	switch(token->type){
	case TOKEN_ROUTE:
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
		// TODO: support comments
		break;
	default:
		printf("FUCK FUCK FUCK !!!\n");
		break;
	}
}


void Token_printType(Token *token){
	assert(token);
	switch(token->type){
	case TOKEN_ROUTE:
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
	assert(token);
	Token_printType(token);
	if(token->type == TOKEN_ROUTE    ||
	   token->type == TOKEN_NUMBER   ||
	   token->type == TOKEN_STRING   ||
	   token->type == TOKEN_COMMENT){
	   printf(";");
	   Token_print(token);
	}
}


bool Token_isOpen(Token *self){
	assert(self);
	if(!self){
		return false;
	}
	return self->type == TOKEN_SOFT_OPEN  || 
	       self->type == TOKEN_HARD_OPEN  ||
	       self->type == TOKEN_CURLY_OPEN;
}


bool Token_isClosed(Token *self){
	assert(self);
	if(!self){
		return false;
	}
	return self->type == TOKEN_SOFT_CLOSE  || 
	       self->type == TOKEN_HARD_CLOSE  ||
	       self->type == TOKEN_CURLY_CLOSE;
}


bool Token_isGrouping(Token *self){
	assert(self);
	return Token_isOpen(self) || Token_isClosed(self);
}


bool Token_isTextual(Token *self){
	assert(self);
	return self->type == TOKEN_ROUTE    ||
	       self->type == TOKEN_COMMENT  ||
	       self->type == TOKEN_STRING;
}


Token *Token_copy(Token *self){
	assert(self);
	Token *r = malloc(sizeof(Token));
	if(!r){
		abort();
	}
	*r = *self;
	if(Token_isTextual(self) && self->data.text){
		r->data.text = strdup(self->data.text);
		if(!r->data.text){
			abort();
		}
	}
	return r;
}


void Token_free(Token *self){
	assert(self);
	Token_clean(self);
	free(self);
}


bool Token_isLiteral(Token *self){
	assert(self);
	return self->type == TOKEN_STRING   ||
	       self->type == TOKEN_ROUTE    ||
	       self->type == TOKEN_NUMBER;
}