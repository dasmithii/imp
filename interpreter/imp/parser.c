#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <imp/parser.h>


// for injected returns in short-hand closures
static const Token RETURN_TOKEN = {.type = TOKEN_ROUTE, .data.text = "return"};


// TODO replace this.
static bool Token_isUnary(Token *t){
	return false;
}


static int ParseNode_init(ParseTree *parent
	                    , ParseNode *node
	                    , Token *begin
	                    , Token *end){
	assert(node);
	assert(begin);
	assert(end);

	// check if leaf node
	if(end == begin + 1){
		if(!Token_isUnary(begin) && !Token_isLiteral(begin)){
			parent->error = strdup("mismatched grouping operators (invalid leaf).");
			return 1;
		}
		node->type = LEAF_NODE;
		node->contents.token = begin;
		return 0;
	}


	TokenType beginType = begin[0].type;


	switch(begin[0].type){
	case TOKEN_HARD_OPEN:
		node->type = OBJECT_NODE;
		if((end - 1)->type != TOKEN_HARD_CLOSE){
			parent->error = strdup("unmatched hard bracket.");
			return -1;
		}
		break;
	case TOKEN_SOFT_OPEN:
		node->type = CALL_NODE;
		if((end - 1)->type != TOKEN_SOFT_CLOSE){
			parent->error = strdup("unmatched soft bracket.");
			return -1;
		}
		break;
	case TOKEN_CURLY_OPEN:
		node->type = CLOSURE_NODE;
		if((end - 1)->type != TOKEN_CURLY_CLOSE){
			parent->error = strdup("unmatched curly brace.");
			return -1;
		}
		break;
	default:
		if(Token_isClosed(begin)){
			parent->error = strdup("misplaced closing.");
			return 1;
		}
		node->type = CALL_NODE;
		node->contents.non_leaf.argc = 2;
		node->contents.non_leaf.argv = malloc(2 * sizeof(ParseNode));
		return ParseNode_init(parent, node->contents.non_leaf.argv, begin, begin + 1)     ||
		       ParseNode_init(parent, node->contents.non_leaf.argv + 1, begin + 1, end);
	}

	// remove beginning and trailing grouping operators
	++begin;
	--end;


	if(end == begin){
		parent->error = strdup("empty group.");
		return 1;
	}


	// The following subCount, subCapacity, and subArray combine
	// to form a dynamic array of ParseNodes. IMP_REGISTER_PARSED
	// appends to this dynamic array.
	size_t subCount = 0;
	size_t subCapacity = 4;
	ParseNode *subArray = malloc(subCapacity * sizeof(ParseNode));
	if(!subArray){
		abort();
	}
	#define IMP_REGISTER_PARSED(node)    \
		if(subCount == subCapacity){     \
			subCapacity *= 2;            \
			subArray = realloc(subArray, subCapacity * sizeof(ParseNode)); \
			if(!subArray){               \
				abort();                 \
			}                            \
		}                                \
		subArray[subCount] = node;       \
		subCount++


	int depth = 0;
	Token *it = begin;
	Token *prev = NULL;

	// Iterate through tokens, tracking grouping level depth. The 
	// goal here is to separate independent groupings and 
	// recursively parse sub-groupings.
	while(it < end){

		if(depth == 0){
			if(prev){
				// We have found an independent sub-grouping. 
				// Parse and register it recursively.
				ParseNode node;
				if(ParseNode_init(parent, &node, prev, it)){
					return 1;
				}
				IMP_REGISTER_PARSED(node);
			}

			// Mark beginning of next sub grouping.
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
	if(ParseNode_init(parent, &final, prev, it)){
		return 1;
	}
	IMP_REGISTER_PARSED(final);


	if(beginType == TOKEN_CURLY_OPEN){
		for(int i = 0; i < subCount; i++){
			ParseNode *sub = subArray + i;
			if(sub->type != CALL_NODE){
				// translate from {<code>} to {(return (<code>))}


				node->contents.non_leaf.argc = 1;
				node->contents.non_leaf.argv = malloc(sizeof(ParseNode));
				// TODO: check return.
		
				ParseNode *rn = node->contents.non_leaf.argv;
				rn->contents.non_leaf.argc = 2;
				rn->contents.non_leaf.argv = malloc(2 * sizeof(ParseNode));
				// TODO: check return.
				rn->type = CALL_NODE;

				ParseNode *rnav = rn->contents.non_leaf.argv;


				rnav[0].type = LEAF_NODE;
				rnav[0].contents.token = &RETURN_TOKEN;

				rnav[1].type = CALL_NODE;
				rnav[1].contents.non_leaf.argc = subCount;
				rnav[1].contents.non_leaf.argv = subArray;
	
				return 0;
			}
		}
	}

	node->contents.non_leaf.argc = subCount;
	node->contents.non_leaf.argv = subArray;
	return 0;
}


int ParseTree_init(ParseTree *tree, char *code){
	assert(tree);
	assert(code);
	tree->error = NULL;

	tree->tokenization = lex(code);
	if(tree->tokenization.error){
		tree->error = strdup(tree->tokenization.error);
		return 1;
	}

	if(ParseNode_init(tree
     , &tree->root
     , tree->tokenization.buffer
     , tree->tokenization.buffer + tree->tokenization.size)){
		return 1;
	}
	tree->root.type = BLOCK_NODE;
	return 0;
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
	case OBJECT_NODE:
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
	case OBJECT_NODE:
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


ParseNode ParseNode_deepCopy(ParseNode *self){
	assert(self);

	ParseNode r;
	r.type = self->type;
	if(r.type == LEAF_NODE){
		r.contents.token = Token_copy(self->contents.token);
	} else {
		size_t argc = self->contents.non_leaf.argc;
		r.contents.non_leaf.argc = self->contents.non_leaf.argc;
		r.contents.non_leaf.argv = malloc(argc * sizeof(ParseNode));
		if(!r.contents.non_leaf.argv){
			abort();
		}
		for(int i = 0; i < argc; i++){
			r.contents.non_leaf.argv[i] = ParseNode_deepCopy(self->contents.non_leaf.argv + i);
		}
	}
	return r;
}


// like ParseNode_clean but accounting for tokens, which are
// stored in ParseTree->tokenization regularly, but not in
// this case (for use in closures). Bad code.
void ParseNode_deepClean(ParseNode *self){
	assert(self);

	if(self->type == LEAF_NODE){
		Token_free(self->contents.token);
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

