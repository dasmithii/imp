// The lexer API consists of the 'lex' function, which
// returns a iTokenization, and iTokenization_clean,
// which frees iTokenization internals.
//
// Implementation wise, we use the Lexer struct to 
// simplify things. It maintains state while decomposed
// lexing functions (prefixed with 'Lexer_try') check
// for tokens of each type. Each of these decomposed
// functions return a boolean based on whether or not
// they recognized a token. They also return true in 
// the case of an error.
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <imp/lexer.h>
#include <imp/token.h>

#include "position.h"




typedef struct {
	char *code;
	char *ptr;
	iPosition position;

	size_t tokenCount;
	size_t tokenCapacity;
	iToken *tokenBuffer;

	char *error;
} Lexer;


static inline Lexer Lexer_of(char *code){
	assert(code);

	Lexer r = {
		.code = code,
		.ptr = code,
		.position = {.line = 0, .column = 0},
		.tokenCount = 0,
		.tokenCapacity = 5
	};

	r.tokenBuffer = malloc(r.tokenCapacity * sizeof(iToken));
	if(r.tokenBuffer){
		r.error = NULL;
	} else{
		r.error = "failed malloc on lexer initialization";
	}
	return r;
}


static inline void Lexer_register(Lexer *self, iToken token){
	token.position = self->position;
	// expand token buffer if necessary
	if(self->tokenCount == self->tokenCapacity){
		self->tokenCapacity *= 2;
		self->tokenBuffer = realloc(self->tokenBuffer
			                       , self->tokenCapacity * sizeof(iToken));
		if(!self->tokenBuffer){
			self->error = "failed to register token; failed malloc";
			return;
		}
	}
	// add token to buffer
	self->tokenBuffer[self->tokenCount] = token;
	self->tokenCount++;
}


static inline void Lexer_shiftRightBy(Lexer *self, int n){
	self->ptr += n;
	iPosition_shift(&self->position, n);
}


static inline bool Lexer_tryWhitespace(Lexer *self){
	if(*self->ptr == ' ' || *self->ptr == '\t'){
		Lexer_shiftRightBy(self, 1);
		return true;
	}
	return false;
}


static inline bool Lexer_tryNewLine(Lexer *self){
	if(*self->ptr == '\n'){
		iPosition_newLine(&self->position);
		self->ptr++;
		return true;
	}
	return false;
}


static inline bool Lexer_tryComment(Lexer *self){
	if(self->ptr[0] == '/' && 
	   self->ptr[1] == '/'){
		Lexer_shiftRightBy(self, 2);
		while(*self->ptr && self->ptr[0] != '\n'){
			Lexer_shiftRightBy(self, 1);
		}
		return true;
	}
	return false;
}


static inline bool Lexer_tryGrouping(Lexer *self){
	#define PAIRING(c, t)                       \
		case c: {                               \
			iToken token = {.type = iTOKEN_##t};  \
			Lexer_register(self, token);        \
			Lexer_shiftRightBy(self, 1);               \
			return true;                        \
		}

	switch(*self->ptr){
	PAIRING('(', SOFT_OPEN)
	PAIRING(')', SOFT_CLOSE)
	PAIRING('[', HARD_OPEN)
	PAIRING(']', HARD_CLOSE)
	PAIRING('{', CURLY_OPEN)
	PAIRING('}', CURLY_CLOSE)
	default:
		return false;
	}
}


static inline bool Lexer_tryNumber(Lexer *self){
	char *c = self->ptr;
	if(*c == '-') {
		++c;
	}
	if(isdigit(c[0]) || (c[0] == '.' && isdigit(c[1]))){
		iToken t = {.type = iTOKEN_NUMBER};
		char *begin = self->ptr;
		t.data.number = strtod(begin, &self->ptr);
		Lexer_register(self, t);
		return true;
	}
	return false;
}


bool iIsValidRouteChar(char c){
	return iIsValidRouteBegin(c) || isdigit(c);
}

bool iIsValidRouteBegin(char c){
	return isalpha(c) ||
	       c == '~'   ||
	       c == '!'   ||
	       c == '@'   ||
	       c == '#'   ||
	       c == '$'   ||
	       c == '%'   ||
	       c == '^'   ||
	       c == '&'   ||
	       c == '*'   ||
	       c == '_'   ||
	       c == '-'   ||
	       c == '+'   ||
	       c == '='   ||
	       c == '|'   ||
	       c == '/'   ||
	       c == '?'   ||
	       c == '>'   ||
	       c == '<'   ||
	       c == ';'   ||
	       c == ':';
}


bool iIsValidRouteText(char *text){
	if(!text || *text == 0){
		return false;
	}
	int len = strlen(text);
	if(len == 0 || !iIsValidRouteBegin(text[0])){
		return false;
	}
	for(int i = 1; i < len; i++){
		if(!iIsValidRouteChar(text[i])){
			return false;
		}
	}
	return true;
}


static inline bool Lexer_tryRoute(Lexer *self){
	if(iIsValidRouteBegin(self->ptr[0])){
		iToken token = {.type = iTOKEN_ROUTE};
		char *begin = self->ptr;
		while(iIsValidRouteChar(self->ptr[0])){
			Lexer_shiftRightBy(self, 1);
		}
		token.data.text = malloc(1 + self->ptr - begin);
		if(!token.data.text){
			self->error = "failed malloc";
			return true;
		}
		token.data.text[self->ptr - begin] = 0;
		memcpy(token.data.text, begin, (self->ptr - begin));
		Lexer_register(self, token);
		return true;
	}
	return false;
}


static inline bool Lexer_tryString(Lexer *self){
	char d = self->ptr[0]; // delimitter
	if(d == '"' || d == '\'' || d == '`'){
		Lexer_shiftRightBy(self, 1);

		iToken token = {.type = iTOKEN_STRING};
		char *begin = self->ptr;

		while(self->ptr[0] != d){
			if(self->ptr[0] == 0){
				self->error = "unterminated string";
				return true;
			}
			Lexer_shiftRightBy(self, 1);
		}

		token.data.text = malloc(1 + self->ptr - begin);
		if(!token.data.text){
			self->error = "failed malloc";
			return true;
		}
		token.data.text[self->ptr - begin] = 0;
		memcpy(token.data.text, begin, (self->ptr - begin));
		Lexer_register(self, token);

		Lexer_shiftRightBy(self, 1);
		return true;
	}
	return false;
}


static inline void Lexer_step(Lexer *self){

	#define IMP_CHECK(type)  if(Lexer_try##type(self)) return
	IMP_CHECK(Whitespace);
	IMP_CHECK(NewLine);
	IMP_CHECK(Comment);
	IMP_CHECK(Grouping);
	IMP_CHECK(Number);
	IMP_CHECK(Route);
	IMP_CHECK(String);
	#undef IMP_CHECK

	self->error = "invalid token";
}


static inline void Lexer_run(Lexer *self){
	iToken t = {.type = iTOKEN_SOFT_OPEN};
	Lexer_register(self, t);

	while(*self->ptr){
		Lexer_step(self);
		if(self->error){
			return;
		}
	}

	t.type = iTOKEN_SOFT_CLOSE;
	Lexer_register(self, t);
}


iTokenization lex(char *code){
	assert(code);
	Lexer lexer = Lexer_of(code);
	if(!lexer.error){
		Lexer_run(&lexer);
	}

	iTokenization r = {
		.buffer = lexer.tokenBuffer,
		.size = lexer.tokenCount,
		.error = NULL
	};
	if(lexer.error){
		r.error = malloc(strlen(r.error) + 32);
		sprintf(r.error, "line %zu, column %zu: %s", lexer.position.line, lexer.position.column, lexer.error);
	}

	return r;
}


void iTokenization_clean(iTokenization *self){
	assert(self);
	if(self->error){
		free(self->error);
	}
	for(size_t i = 0; i < self->size; i++){
		iToken_clean(self->buffer + i);
	}
	free(self->buffer);
}


void iTokenization_print(iTokenization *self){
	assert(self);
	assert(!self->error);
	for(size_t i = 0; i < self->size; i++){
		iToken_printVerbose(self->buffer + i);
		printf(" ");
	}
	printf("\n");
}

