#ifndef IMP_TOKEN_H_
#define IMP_TOKEN_H_

#include <stdbool.h>




typedef enum {
	TOKEN_ROUTE,
	TOKEN_NUMBER,
	TOKEN_STRING,

	TOKEN_HARD_OPEN,
	TOKEN_HARD_CLOSE,
	TOKEN_SOFT_OPEN,
	TOKEN_SOFT_CLOSE,
	TOKEN_CURLY_OPEN,
	TOKEN_CURLY_CLOSE,
	TOKEN_COMMENT
} TokenType;


typedef struct {
	TokenType type;
	union {
		char *text;
		double number;
	} data;
} Token;


//// management
void Token_clean(Token *token);
void Token_free(Token *token);
Token *Token_copy(Token *self);


//// miscellaneous
void Token_print(Token *token);
void Token_printType(Token *token);
void Token_printVerbose(Token *token);


//// helpers
bool Token_isOpen(Token *self);
bool Token_isClosed(Token *self);
bool Token_isGrouping(Token *self);
bool Token_isTextual(Token *self);
bool Token_isLiteral(Token *self);


#endif