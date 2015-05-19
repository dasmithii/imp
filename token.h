#ifndef IMP_TOKEN_H_
#define IMP_TOKEN_H_
#include <assert.h>
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
	TOKEN_COMMENT,

	TOKEN_NOT,
	TOKEN_AT,
	TOKEN_HASH,
	TOKEN_DOLLAR,
	TOKEN_PERCENT,
	TOKEN_CARROT,
	TOKEN_AMP,
	TOKEN_STAR,
	TOKEN_PLUS,
	TOKEN_QUESTION,
	TOKEN_SEMI,
	TOKEN_DASH
} TokenType;


typedef struct {
	TokenType type;
	union {
		char *text;
		double number;
	} data;
} Token;


void Token_clean(Token *token);
void Token_print(Token *token);
void Token_printType(Token *token);
void Token_printVerbose(Token *token);

bool Token_isOpen(Token *self);
bool Token_isClosed(Token *self);
bool Token_isGrouping(Token *self);
bool Token_isUnary(Token *self);

#endif