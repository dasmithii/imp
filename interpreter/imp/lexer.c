// The lexer API consists of the 'lex' function, which
// returns a Tokenization, and Tokenization_clean,
// which frees Tokenization internals.
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




typedef struct {
	char *code;
	char *position;
	size_t line;
	size_t column;

	size_t tokenCount;
	size_t tokenCapacity;
	Token *tokenBuffer;

	char *error;
} Lexer;


static inline Lexer Lexer_of(char *code){
	assert(code);

	Lexer r = {
		.code = code,
		.position = code,
		.line = 0,
		.column = 0,
		.tokenCount = 0,
		.tokenCapacity = 5
	};

	r.tokenBuffer = malloc(r.tokenCapacity * sizeof(Token));
	if(r.tokenBuffer){
		r.error = NULL;
	} else{
		r.error = "failed malloc on lexer initialization";
	}
	return r;
}


static inline void Lexer_register(Lexer *self, Token token){
	// expand token buffer if necessary
	if(self->tokenCount == self->tokenCapacity){
		self->tokenCapacity *= 2;
		self->tokenBuffer = realloc(self->tokenBuffer
			                       , self->tokenCapacity * sizeof(Token));
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
	self->position += n;
	self->column += n;
}


static inline bool Lexer_tryWhitespace(Lexer *self){
	if(*self->position == ' ' || *self->position == '\t'){
		Lexer_shiftRightBy(self, 1);
		return true;
	}
	return false;
}


static inline bool Lexer_tryNewLine(Lexer *self){
	if(*self->position == '\n'){
		self->column = 0;
		self->line++;
		self->position++;
		return true;
	}
	return false;
}


static inline bool Lexer_tryComment(Lexer *self){
	if(self->position[0] == '/' && 
	   self->position[1] == '/'){
		Lexer_shiftRightBy(self, 2);
		while(*self->position && self->position[0] != '\n'){
			Lexer_shiftRightBy(self, 1);
		}
		return true;
	}
	return false;
}


static inline bool Lexer_tryGrouping(Lexer *self){
	#define PAIRING(c, t)                       \
		case c: {                               \
			Token token = {.type = TOKEN_##t};  \
			Lexer_register(self, token);        \
			Lexer_shiftRightBy(self, 1);               \
			return true;                        \
		}

	switch(*self->position){
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
	char *c = self->position;
	if(*c == '-') {
		++c;
	}
	if(isdigit(c[0]) || (c[0] == '.' && isdigit(c[1]))){
		Token t = {.type = TOKEN_NUMBER};
		char *begin = self->position;
		t.data.number = strtod(begin, &self->position);
		Lexer_register(self, t);
		return true;
	}
	return false;
}


bool isValidRouteChar(char c){
	return isValidRouteBegin(c) || isdigit(c);
}

bool isValidRouteBegin(char c){
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


bool isValidRouteText(char *text){
	if(!text || *text == 0){
		return false;
	}
	int len = strlen(text);
	if(len == 0 || !isValidRouteBegin(text[0])){
		return false;
	}
	for(int i = 1; i < len; i++){
		if(!isValidRouteChar(text[i])){
			return false;
		}
	}
	return true;
}


static inline bool Lexer_tryRoute(Lexer *self){
	if(isValidRouteBegin(self->position[0])){
		Token token = {.type = TOKEN_ROUTE};
		char *begin = self->position;
		while(isValidRouteChar(self->position[0])){
			Lexer_shiftRightBy(self, 1);
		}
		token.data.text = malloc(1 + self->position - begin);
		if(!token.data.text){
			self->error = "failed malloc";
			return true;
		}
		token.data.text[self->position - begin] = 0;
		memcpy(token.data.text, begin, (self->position - begin));
		Lexer_register(self, token);
		return true;
	}
	return false;
}


static inline bool Lexer_tryString(Lexer *self){
	char d = self->position[0]; // delimitter
	if(d == '"' || d == '\'' || d == '`'){
		Lexer_shiftRightBy(self, 1);

		Token token = {.type = TOKEN_STRING};
		char *begin = self->position;

		while(self->position[0] != d){
			if(self->position[0] == 0){
				self->error = "unterminated string";
				return true;
			}
			Lexer_shiftRightBy(self, 1);
		}

		token.data.text = malloc(1 + self->position - begin);
		if(!token.data.text){
			self->error = "failed malloc";
			return true;
		}
		token.data.text[self->position - begin] = 0;
		memcpy(token.data.text, begin, (self->position - begin));
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
	Token t = {.type = TOKEN_SOFT_OPEN};
	Lexer_register(self, t);

	while(*self->position){
		Lexer_step(self);
		if(self->error){
			return;
		}
	}

	t.type = TOKEN_SOFT_CLOSE;
	Lexer_register(self, t);
}


Tokenization lex(char *code){
	assert(code);
	Lexer lexer = Lexer_of(code);
	if(!lexer.error){
		Lexer_run(&lexer);
	}

	Tokenization r = {
		.buffer = lexer.tokenBuffer,
		.size = lexer.tokenCount,
		.error = NULL
	};
	if(lexer.error){
		r.error = malloc(strlen(r.error) + 32);
		sprintf(r.error, "line %zu, column %zu: %s", lexer.line, lexer.column, lexer.error);
	}

	return r;
}


void Tokenization_clean(Tokenization *self){
	assert(self);
	if(self->error){
		free(self->error);
	}
	for(int i = 0; i < self->size; i++){
		Token_clean(self->buffer + i);
	}
	free(self->buffer);
}


void Tokenization_print(Tokenization *self){
	assert(self);
	assert(!self->error);
	for(int i = 0; i < self->size; i++){
		Token_printVerbose(self->buffer + i);
		printf(" ");
	}
	printf("\n");
}

