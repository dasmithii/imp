#include "parser.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "toolbox/vector.h"


static int ParseNode_init(ParseNode *node, Token *begin, Token *end){
	assert(node);
	assert(begin);
	assert(end);

	if(!node || !begin){
		printf("fuck");
		exit(1);
	}


	// check if leaf node
	if(end == begin + 1){
		node->type = LEAF_NODE;
		node->contents.token = begin;
		return 0;
	}

	switch(begin[0].type){
	case TOKEN_HARD_OPEN:
		node->type = MACRO_NODE;
		if((end - 1)->type != TOKEN_HARD_CLOSE){
			//TODO
			return -1;
		}
		break;
	case TOKEN_SOFT_OPEN:
		node->type = CALL_NODE;
		if((end - 1)->type != TOKEN_SOFT_CLOSE){
			//TODO
			return -1;
		}
		break;
	case TOKEN_CURLY_OPEN:
		node->type = CLOSURE_NODE;
		if((end - 1)->type != TOKEN_CURLY_CLOSE){
			//TODO
			return -1;
		}
		break;
	default:
		node->type = CALL_NODE;
		node->contents.non_leaf.argc = 2;
		node->contents.non_leaf.argv = malloc(2 * sizeof(ParseNode));
		ParseNode_init(node->contents.non_leaf.argv, begin, begin + 1); // TODO: check error
		ParseNode_init(node->contents.non_leaf.argv + 1, begin + 1, end);   // TODO: check error
		return 0;
		break;
	}

	// remove beginning and trailing grouping operators
	++begin;
	--end;

	// find and parse sub-nodes
	Vector subs;
	Vector_init(&subs, sizeof(ParseNode));
	int depth = 0;
	Token *it = begin;
	Token *prev = NULL;
	while(it < end){

		// record beginning. parse prev if necessary
		if(depth == 0){
			if(prev){
				ParseNode node;
				ParseNode_init(&node, prev, it);
				Vector_append(&subs, &node);
			}
			prev = it;
			if(Token_isUnary(prev)){
				while(Token_isUnary(it)){
					++it;
				}
				if(Token_isOpen(it)){
					int d = 1;
					while(d){
						++it;
						if(Token_isOpen(it)){
							++d;
						} else if(Token_isClosed(it)){
							--d;
						}
					}
				}
			}
		}

		// check groupings and iterate
		if(Token_isOpen(it)){
			++depth;
		} else if(Token_isClosed(it)){
			--depth;
		}
		++it;
	}
	ParseNode final;
	ParseNode_init(&final, prev, it);
	Vector_append(&subs, &final);
	node->contents.non_leaf.argc = subs.size;
	node->contents.non_leaf.argv = (void*)subs.buffer.data;
	return 0;
}

int ParseTree_init(ParseTree *tree, char *code){
	assert(tree);
	assert(code);

	int ec = Tokenization_init(&tree->tokenization, code);
	if(ec){
		tree->error = strdup(tree->tokenization.error);
		return ec;
	}
	int rc = ParseNode_init(&tree->root
		                , Tokenization_begin(&tree->tokenization)
		                , Tokenization_end(&tree->tokenization));
	tree->root.type = BLOCK_NODE;
	return rc;
}


static void ParseNode_clean(ParseNode *node){
	assert(node);

	if(node->type != LEAF_NODE){
		const size_t argc = node->contents.non_leaf.argc;
		if(argc > 0){
			for(int i = 0; i < argc; ++i){
				ParseNode_clean(node->contents.non_leaf.argv + i);
			}
			free(node->contents.non_leaf.argv);
		}
	}
}


void ParseTree_clean(ParseTree *tree){
	assert(tree);

	if(tree->error){
		free(tree->error);
		tree->error = NULL;
	}
	ParseNode_clean(&tree->root);
	Tokenization_clean(&tree->tokenization);
}


void ParseNode_print(ParseNode *self){
	assert(self);

	switch(self->type){
	case CALL_NODE:
		printf("(");
		break;
	case MACRO_NODE:
		printf("[");
		break;
	case CLOSURE_NODE:
		printf("{");
		break;
	}

	if(self->type == LEAF_NODE){
		Token_print(self->contents.token);
	} else {
		if(self->contents.non_leaf.argc > 0){
			ParseNode_print(self->contents.non_leaf.argv);
			for(int i = 1; i < self->contents.non_leaf.argc; i++){
				printf(" ");
				ParseNode_print(self->contents.non_leaf.argv + i);
			}
		}
	}


	switch(self->type){
	case CALL_NODE:
		printf(")");
		break;
	case MACRO_NODE:
		printf("]");
		break;
	case CLOSURE_NODE:
		printf("}");
		break;
	}
}

void ParseTree_print(ParseTree *self){
	assert(self);

	ParseNode_print(&self->root);
}

// typedef enum {
// 	CALL_NODE,
// 	MACRO_NODE,
// 	CLOSURE_NODE,
// 	LEAF_NODE
// } ParseNodeType;


// typedef struct ParseNode {
// 	ParseNodeType type;
// 	union {
// 		Token *token;
// 		struct {
// 			size_t argc;
// 			struct ParseNode *argv;
// 		} non_leaf;
// 	} contents;
// } ParseNode;

// typedef struct {
// 	Tokenization tokenization;
// 	ParseNode root;
// 	char *error;
// } ParseTree;


ParseNode ParseNode_deepCopy(ParseNode *self){
	assert(self);

	ParseNode r;
	r.type = self->type;
	if(r.type == LEAF_NODE){
		r.contents.token = malloc(sizeof(Token));
		*r.contents.token = *self->contents.token;
	} else {
		size_t argc = self->contents.non_leaf.argc;
		r.contents.non_leaf.argc = self->contents.non_leaf.argc;
		r.contents.non_leaf.argv = malloc(argc * sizeof(ParseNode));
		for(int i = 0; i < argc; i++){
			r.contents.non_leaf.argv[i] = ParseNode_deepCopy(self->contents.non_leaf.argv + i);
		}
	}
	return r;
}


void ParseNode_deepClean(ParseNode *self){
	assert(self);

	if(self->type == LEAF_NODE){
		free(self->contents.token);
		self->contents.token = NULL;
	} else {
		size_t argc = self->contents.non_leaf.argc;
		for(int i = 0; i < argc; i++){
			ParseNode_deepClean(self->contents.non_leaf.argv + i);
		}
		free(self->contents.non_leaf.argv);
		self->contents.non_leaf.argc = 0;
	}
}



