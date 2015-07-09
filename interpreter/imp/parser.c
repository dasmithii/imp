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

	node->argc = 0;

	// check if leaf node
	if(end == begin + 1){
		if(!Token_isUnary(begin) && !Token_isLiteral(begin)){
			parent->error = strdup("mismatched grouping operators (invalid leaf).");
			return 1;
		}
		node->type = LEAF_NODE;
		node->token = begin;
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
		node->argc = 2;
		node->argv = malloc(2 * sizeof(ParseNode));
		return ParseNode_init(parent, node->argv, begin, begin + 1)     ||
		       ParseNode_init(parent, node->argv + 1, begin + 1, end);
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
	#define IMP_REGISTER_PARSED(node)                  \
		if(ParseNode_isContextualRoute(&node)){        \
			if(subCount == 0){                         \
				printf("ERROR ERROR: invalid :route"); \
				exit(1);                               \
			} else {                                   \
				node.argc = 1;                         \
				node.argv = malloc(sizeof(ParseNode)); \
				if(!node.argv){                             \
					abort();                           \
				}                                      \
				node.argv[0] = subArray[subCount - 1]; \
				subArray[subCount - 1] = node;         \
			}                                          \
		} else {                                       \
			if(subCount == subCapacity){               \
				subCapacity *= 2;                      \
				subArray = realloc(subArray, subCapacity * sizeof(ParseNode)); \
				if(!subArray){                         \
					abort();                           \
				}                                      \
			}                                          \
			subArray[subCount] = node;                 \
			subCount++;                                \
		}



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
				IMP_REGISTER_PARSED(node)
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
	IMP_REGISTER_PARSED(final)


	if(beginType == TOKEN_CURLY_OPEN){
		for(int i = 0; i < subCount; i++){
			ParseNode *sub = subArray + i;
			if(sub->type != CALL_NODE){
				// translate from {<code>} to {(return (<code>))}


				node->argc = 1;
				node->argv = malloc(sizeof(ParseNode));
				// TODO: check return.
		
				ParseNode *rn = node->argv;
				rn->argc = 2;
				rn->argv = malloc(2 * sizeof(ParseNode));
				// TODO: check return.
				rn->type = CALL_NODE;

				ParseNode *rnav = rn->argv;


				rnav[0].type = LEAF_NODE;
				rnav[0].token = &RETURN_TOKEN;
				rnav[0].argc = 0;

				rnav[1].type = CALL_NODE;
				rnav[1].argc = subCount;
				rnav[1].argv = subArray;
	
				return 0;
			}
		}
	}

	// TODO: realloc subArray to minimize memory used
	node->argc = subCount;
	node->argv = subArray;
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


static void ParseNode_clean(ParseNode *self){
	assert(self);

	if(self->argc){
		for(int i = 0; i < self->argc; ++i){
			ParseNode_clean(self->argv + i);
		}
		free(self->argv);
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
		Token_print(self->token);
	} else {
		if(self->argc > 0){
			ParseNode_print(self->argv);
			for(int i = 1; i < self->argc; i++){
				printf(" ");
				ParseNode_print(self->argv + i);
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
		r.token = Token_copy(self->token);
	}

	r.argc = self->argc;
	if(r.argc){
		r.argv = malloc(r.argc * sizeof(ParseNode));
		if(!r.argv){
			abort();
		}
		for(int i = 0; i < r.argc; i++){
			r.argv[i] = ParseNode_deepCopy(self->argv + i);
		}
	} else {
		r.argv = NULL;
	}
	return r;
}


// like ParseNode_clean but accounting for tokens, which are
// stored in ParseTree->tokenization regularly, but not in
// this case (for use in closures). Bad code.
void ParseNode_deepClean(ParseNode *self){
	assert(self);

	if(self->type == LEAF_NODE){
		Token_free(self->token);
	}

	if(self->argc > 0){
		for(int i = 0; i < self->argc; i++){
			ParseNode_deepClean(self->argv + i);
		}
		free(self->argv);
	}
}


bool ParseNode_isContextualRoute(ParseNode *self){
	return self->type == LEAF_NODE 
	    && Token_isContextualRoute(self->token);
}

