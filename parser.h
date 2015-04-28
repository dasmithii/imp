#ifndef IMP_PARSER_H_
#define IMP_PARSER_H_
#include "token.h"
#include "lexer.h"


typedef enum {
	CALL_NODE,
	MACRO_NODE,
	CLOSURE_NODE,
	LEAF_NODE
} ParseNodeType;


typedef struct ParseNode {
	ParseNodeType type;
	union {
		Token *token;
		struct {
			size_t argc;
			struct ParseNode *argv;
		} non_leaf;
	} contents;
} ParseNode;

typedef struct {
	Tokenization tokenization;
	ParseNode root;
	char *error;
} ParseTree;


int ParseTree_init(ParseTree *tree, char *code);
void ParseTree_clean(ParseTree *tree);
void ParseTree_print(ParseTree *self);

#endif