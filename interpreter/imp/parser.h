#ifndef IMP_PARSER_H_
#define IMP_PARSER_H_

#include <stdbool.h>

#include <imp/lexer.h>
#include <imp/token.h>




typedef enum {
	iNODE_BLOCK,
	iNODE_CALL,
	iNODE_OBJECT,
	iNODE_CLOSURE,
	iNODE_LEAF
} iParseNodeType;


typedef struct iParseNode {
	iParseNodeType type;
	iToken *token;            // could use union here... fuck it though
	size_t argc;             //   it's simpler this way
	struct iParseNode *argv;
} iParseNode;


typedef struct {
	iTokenization tokenization;
	iParseNode root;
	char *error;
} iParseTree;


//// management
int iParseTree_init(iParseTree *tree, char *code);
void iParseTree_clean(iParseTree *tree);


//// miscellaneous
void iParseTree_print(iParseTree *self);
void iParseNode_print(iParseNode *self);

// create node without parent tree
iParseNode iParseNode_deepCopy(iParseNode *self);

// for parse nodes without a parent tree
void iParseNode_deepClean(iParseNode *self);

bool iParseNode_isContextualRoute(iParseNode *self);


#endif