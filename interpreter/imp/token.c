#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <imp/token.h>




void iToken_clean(iToken *token) {
	assert(token);
	if(token->type == iTOKEN_ROUTE   ||
	   token->type == iTOKEN_COMMENT || 
	   token->type == iTOKEN_STRING){
		free(token->data.text);
		token->data.text = NULL;
	}
}


void iToken_print(iToken *token){
	assert(token);
	switch(token->type){
	case iTOKEN_ROUTE:
		printf("%s", token->data.text);
		break;
	case iTOKEN_STRING:
		printf("\"%s\"", token->data.text);
		break;
	case iTOKEN_NUMBER:
		printf("%f", token->data.number);
		break;
	case iTOKEN_HARD_OPEN:
		printf("[");
		break;
	case iTOKEN_HARD_CLOSE:
		printf("]");
		break;
	case iTOKEN_SOFT_OPEN:
		printf("(");
		break;
	case iTOKEN_SOFT_CLOSE:
		printf(")");
		break;
	case iTOKEN_CURLY_OPEN:
		printf("{");
		break;
	case iTOKEN_CURLY_CLOSE:
		printf("}");
		break;
	case iTOKEN_COMMENT:
		// TODO: support comments
		break;
	default:
		printf("FUCK FUCK FUCK !!!\n");
		break;
	}
}


void iToken_printType(iToken *token){
	assert(token);
	switch(token->type){
	case iTOKEN_ROUTE:
		printf("route");
		break;
	case iTOKEN_NUMBER:
		printf("number");
		break;
	case iTOKEN_STRING:
		printf("string");
		break;
	case iTOKEN_COMMENT:
		printf("comment");
		break;
	default:
		iToken_print(token);
		break;
	}
}


void iToken_printVerbose(iToken *token){
	assert(token);
	iToken_printType(token);
	if(token->type == iTOKEN_ROUTE    ||
	   token->type == iTOKEN_NUMBER   ||
	   token->type == iTOKEN_STRING   ||
	   token->type == iTOKEN_COMMENT){
	   printf(";");
	   iToken_print(token);
	}
}


bool iToken_isOpen(iToken *self){
	assert(self);
	if(!self){
		return false;
	}
	return self->type == iTOKEN_SOFT_OPEN  || 
	       self->type == iTOKEN_HARD_OPEN  ||
	       self->type == iTOKEN_CURLY_OPEN;
}


bool iToken_isClosed(iToken *self){
	assert(self);
	if(!self){
		return false;
	}
	return self->type == iTOKEN_SOFT_CLOSE  || 
	       self->type == iTOKEN_HARD_CLOSE  ||
	       self->type == iTOKEN_CURLY_CLOSE;
}


bool iToken_isGrouping(iToken *self){
	assert(self);
	return iToken_isOpen(self) || iToken_isClosed(self);
}


bool iToken_isTextual(iToken *self){
	assert(self);
	return self->type == iTOKEN_ROUTE    ||
	       self->type == iTOKEN_COMMENT  ||
	       self->type == iTOKEN_STRING;
}


iToken *iToken_copy(iToken *self){
	assert(self);
	iToken *r = malloc(sizeof(iToken));
	if(!r){
		abort();
	}
	*r = *self;
	if(iToken_isTextual(self) && self->data.text){
		r->data.text = strdup(self->data.text);
		if(!r->data.text){
			abort();
		}
	}
	return r;
}


void iToken_free(iToken *self){
	assert(self);
	iToken_clean(self);
	free(self);
}


bool iToken_isLiteral(iToken *self){
	assert(self);
	return self->type == iTOKEN_STRING   ||
	       self->type == iTOKEN_ROUTE    ||
	       self->type == iTOKEN_NUMBER;
}


bool iToken_isContextualRoute(iToken *self){
	return self->type == iTOKEN_ROUTE
	    && self->data.text[0] == ':';
}