#ifndef iTOKEN
#define iTOKEN

#include <stdbool.h>

#include "position.h"




typedef enum {
	iTOKEN_ROUTE,
	iTOKEN_NUMBER,
	iTOKEN_STRING,

	iTOKEN_HARD_OPEN,
	iTOKEN_HARD_CLOSE,
	iTOKEN_SOFT_OPEN,
	iTOKEN_SOFT_CLOSE,
	iTOKEN_CURLY_OPEN,
	iTOKEN_CURLY_CLOSE,
	iTOKEN_COMMENT
} iTokenType;


typedef struct {
	iPosition position;
	iTokenType type;
	union {
		char *text;
		double number;
	} data;
} iToken;


//// management
void iToken_clean(iToken *token);
void iToken_free(iToken *token);
iToken *iToken_copy(iToken *self);


//// miscellaneous
void iToken_print(iToken *token);
void iToken_printType(iToken *token);
void iToken_printVerbose(iToken *token);


//// helpers
bool iToken_isOpen(iToken *self);
bool iToken_isClosed(iToken *self);
bool iToken_isGrouping(iToken *self);
bool iToken_isTextual(iToken *self);
bool iToken_isLiteral(iToken *self);

bool iToken_isContextualRoute(iToken *self);




#endif