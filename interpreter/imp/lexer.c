#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <imp/lexer.h>
#include <imp/token.h>


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


int Tokenization_init(Tokenization *tokenization, char *code){
	assert(tokenization);
	assert(code);
	tokenization->error = NULL;

	// skip shebang
	if(code[0] == '#' && code[1] == '!'){
		while(*code && *code != '\n'){
			++code;
		}
		++code;
	}

	// 
	size_t tokenCount = 0;
	size_t tokenCapacity = 128;
	Token *tokenArray = malloc(tokenCapacity * sizeof(Token));
	if(!tokenArray){
		abort();
	}
	#define IMP_REGISTER_TOKEN(t)          \
		if(tokenCount == tokenCapacity){   \
			tokenCapacity *= 2;            \
			tokenArray = realloc(tokenArray, tokenCapacity * sizeof(Token)); \
			if(!tokenArray){               \
				abort();                   \
			}                              \
		}                                  \
		tokenArray[tokenCount] = t;        \
		tokenCount++

	// prefix with '('
	Token tmp = {.type = TOKEN_SOFT_OPEN};
	IMP_REGISTER_TOKEN(tmp);

	Token token;
	bool afterSpace;
	int line = 0;
	char *end = code + strlen(code);
	while(*code){
		if(isspace(*code)){
			++code;
			afterSpace = true;
			if(*code == '\n'){
				line++;
			}
			continue;
		}

		// extract comment
		if(code[0] == '/' && end - code >= 0 && code[1] == '/'){
			while(*code && *code != '\n'){
				++code;
			}
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
		default:
			{
				char *end = code + 1;
				if(*code == '\''){
					token.type = TOKEN_STRING;
					++code;
					++end;
					while(*end != '\''){
						if(*end == 0){
							tokenization->error = strdup("unterminated string.");
							return 1;
						}
						++end;
					}
					token.data.text = malloc((end - code) + 1);
					if(!token.data.text){
						abort();
					}
					token.data.text[end - code] = 0;
					memcpy(token.data.text, code, (end-code));
					++end;
				} else if(isValidRouteBegin(*code)){
					token.type = TOKEN_ROUTE;
					end = code + 1;
					while(isValidRouteChar(*end)){
						++end;
					}
					token.data.text = malloc((end - code) + 1);
					if(!token.data.text){
						abort();
					}
					token.data.text[end - code] = 0;
					memcpy(token.data.text, code, (end-code));
				} else if(isdigit(*code) || *code == '.'){
					token.type = TOKEN_NUMBER;
					end = code + 1;
					while(isdigit(*end) || *end == '.'){
						++end;
					}
					token.data.number = strtod(code, &end); // ISSUE
				} else {
					tokenization->error = malloc(164);
					sprintf(tokenization->error, "Unrecognized token '%c' on line %d.", *code, line);
					return 1;
				}
				code = end - 1; // to counteract ** below
				break;
			}
		}
		++code; // **                        **  ^^^
		afterSpace = false;
		IMP_REGISTER_TOKEN(token);
	}

	tmp.type = TOKEN_SOFT_CLOSE;
	IMP_REGISTER_TOKEN(tmp);
	#undef IMP_REGISTER_TOKEN
	tokenization->size = tokenCount;
	tokenization->buffer = tokenArray;
	return 0; // TODO: handle errors
}

	
void Tokenization_clean(Tokenization *tokenization){
	assert(tokenization);
	if(tokenization->error){
		free(tokenization->error);
		tokenization->error = NULL;
	}
	for(int i = 0; i < tokenization->size; i++){
		Token_clean(tokenization->buffer + i);
	}
	free(tokenization->buffer);
	tokenization->buffer = NULL;
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


void Tokenization_print(Tokenization *self){
	assert(self);
	assert(!self->error);
	for(int i = 0; i < self->size; i++){
		Token_printVerbose(self->buffer + i);
		printf(" ");
	}
}

