#ifndef IMP_PARSER_H_
#define IMP_PARSER_H_

#include <stdbool.h>

#include <imp/lexer.h>
#include <imp/token.h>




typedef enum {
	BLOCK_NODE,
	CALL_NODE,
	OBJECT_NODE,
	CLOSURE_NODE,
	LEAF_NODE
} ParseNodeType;


typedef struct ParseNode {
	ParseNodeType type;
	Token *token;            // could use union here... fuck it though
	size_t argc;             //   it's simpler this way
	struct ParseNode *argv;
} ParseNode;


typedef struct {
	Tokenization tokenization;
	ParseNode root;
	char *error;
} ParseTree;


//// management
int ParseTree_init(ParseTree *tree, char *code);
void ParseTree_clean(ParseTree *tree);


//// miscellaneous
void ParseTree_print(ParseTree *self);
void ParseNode_print(ParseNode *self);

// create node without parent tree
ParseNode ParseNode_deepCopy(ParseNode *self);

// for parse nodes without a parent tree
void ParseNode_deepClean(ParseNode *self);

bool ParseNode_isContextualRoute(ParseNode *self);


#endif