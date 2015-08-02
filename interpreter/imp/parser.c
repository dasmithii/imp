#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <imp/parser.h>


// for injected returns in short-hand closures
static const iToken iRETURN_TOKEN = {.type = iTOKEN_ROUTE, .data.text = "return"};


// TODO replace this.
static bool iToken_isUnary(iToken *t){
	return false;
}


static int iParseNode_init(iParseTree *parent
	                     , iParseNode *node
	                     , iToken *begin
	                     , iToken *end){
	assert(node);
	assert(begin);
	assert(end);

	node->argc = 0;

	// check if leaf node
	if(end == begin + 1){
		if(!iToken_isUnary(begin) && !iToken_isLiteral(begin)){
			parent->error = strdup("mismatched grouping operators (invalid leaf).");
			return 1;
		}
		node->type = iNODE_LEAF;
		node->token = begin;
		return 0;
	}


	iTokenType beginType = begin[0].type;


	switch(begin[0].type){
	case iTOKEN_HARD_OPEN:
		node->type = iNODE_OBJECT;
		if((end - 1)->type != iTOKEN_HARD_CLOSE){
			parent->error = strdup("unmatched hard bracket.");
			return -1;
		}
		break;
	case iTOKEN_SOFT_OPEN:
		node->type = iNODE_CALL;
		if((end - 1)->type != iTOKEN_SOFT_CLOSE){
			parent->error = strdup("unmatched soft bracket.");
			return -1;
		}
		break;
	case iTOKEN_CURLY_OPEN:
		node->type = iNODE_CLOSURE;
		if((end - 1)->type != iTOKEN_CURLY_CLOSE){
			parent->error = strdup("unmatched curly brace.");
			return -1;
		}
		break;
	default:
		if(iToken_isClosed(begin)){
			parent->error = strdup("misplaced closing.");
			return 1;
		}
		node->type = iNODE_CALL;
		node->argc = 2;
		node->argv = malloc(2 * sizeof(iParseNode));
		return iParseNode_init(parent, node->argv, begin, begin + 1)     ||
		       iParseNode_init(parent, node->argv + 1, begin + 1, end);
	}

	// remove beginning and trailing grouping operators
	++begin;
	--end;


	if(end == begin){
		parent->error = strdup("empty group.");
		return 1;
	}


	// The following subCount, subCapacity, and subArray combine
	// to form a dynamic array of iParseNodes. IMP_REGISTER_PARSED
	// appends to this dynamic array.
	size_t subCount = 0;
	size_t subCapacity = 4;
	iParseNode *subArray = malloc(subCapacity * sizeof(iParseNode));
	if(!subArray){
		abort();
	}
	#define IMP_REGISTER_PARSED(node)                  \
		if(iParseNode_isContextualRoute(&node)){        \
			if(subCount == 0){                         \
				printf("ERROR ERROR: invalid :route"); \
				exit(1);                               \
			} else {                                   \
				node.argc = 1;                         \
				node.argv = malloc(sizeof(iParseNode)); \
				if(!node.argv){                             \
					abort();                           \
				}                                      \
				node.argv[0] = subArray[subCount - 1]; \
				subArray[subCount - 1] = node;         \
			}                                          \
		} else {                                       \
			if(subCount == subCapacity){               \
				subCapacity *= 2;                      \
				subArray = realloc(subArray, subCapacity * sizeof(iParseNode)); \
				if(!subArray){                         \
					abort();                           \
				}                                      \
			}                                          \
			subArray[subCount] = node;                 \
			subCount++;                                \
		}



	int depth = 0;
	iToken *it = begin;
	iToken *prev = NULL;

	// Iterate through tokens, tracking grouping level depth. The 
	// goal here is to separate independent groupings and 
	// recursively parse sub-groupings.
	while(it < end){

		if(depth == 0){
			if(prev){
				// We have found an independent sub-grouping. 
				// iParse and register it recursively.
				iParseNode node;
				if(iParseNode_init(parent, &node, prev, it)){
					return 1;
				}
				IMP_REGISTER_PARSED(node)
			}

			// Mark beginning of next sub grouping.
			prev = it;
			if(iToken_isUnary(prev)){
				while(iToken_isUnary(it)){
					++it;
				}
				if(iToken_isOpen(it)){
					int d = 1;
					while(d){
						++it;
						if(iToken_isOpen(it)){
							++d;
						} else if(iToken_isClosed(it)){
							--d;
						}
					}
				}
			}
		}

		// check groupings and iterate
		if(iToken_isOpen(it)){
			++depth;
		} else if(iToken_isClosed(it)){
			--depth;
		}
		++it;
	}
	iParseNode final;
	if(iParseNode_init(parent, &final, prev, it)){
		return 1;
	}
	IMP_REGISTER_PARSED(final)


	if(beginType == iTOKEN_CURLY_OPEN){
		for(size_t i = 0; i < subCount; i++){
			iParseNode *sub = subArray + i;
			if(sub->type != iNODE_CALL){
				// translate from {<code>} to {(return (<code>))}


				node->argc = 1;
				node->argv = malloc(sizeof(iParseNode));
				// TODO: check return.
		
				iParseNode *rn = node->argv;
				rn->argc = 2;
				rn->argv = malloc(2 * sizeof(iParseNode));
				// TODO: check return.
				rn->type = iNODE_CALL;

				iParseNode *rnav = rn->argv;


				rnav[0].type = iNODE_LEAF;
				rnav[0].token = &iRETURN_TOKEN;
				rnav[0].argc = 0;

				rnav[1].type = iNODE_CALL;
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


int iParseTree_init(iParseTree *tree, char *code){
	assert(tree);
	assert(code);
	tree->error = NULL;

	tree->tokenization = lex(code);
	if(tree->tokenization.error){
		tree->error = strdup(tree->tokenization.error);
		return 1;
	}

	if(iParseNode_init(tree
     , &tree->root
     , tree->tokenization.buffer
     , tree->tokenization.buffer + tree->tokenization.size)){
		return 1;
	}
	tree->root.type = iNODE_BLOCK;
	return 0;
}


static void iParseNode_clean(iParseNode *self){
	assert(self);

	if(self->argc){
		for(size_t i = 0; i < self->argc; ++i){
			iParseNode_clean(self->argv + i);
		}
		free(self->argv);
	}
}


void iParseTree_clean(iParseTree *tree){
	assert(tree);

	if(tree->error){
		free(tree->error);
		tree->error = NULL;
	}
	iParseNode_clean(&tree->root);
	iTokenization_clean(&tree->tokenization);
}


void iParseNode_print(iParseNode *self){
	assert(self);

	switch(self->type){
	case iNODE_CALL:
		printf("(");
		break;
	case iNODE_OBJECT:
		printf("[");
		break;
	case iNODE_CLOSURE:
		printf("{");
		break;
	}

	if(self->type == iNODE_LEAF){
		iToken_print(self->token);
	} else {
		if(self->argc > 0){
			iParseNode_print(self->argv);
			for(size_t i = 1; i < self->argc; i++){
				printf(" ");
				iParseNode_print(self->argv + i);
			}
		}
	}


	switch(self->type){
	case iNODE_CALL:
		printf(")");
		break;
	case iNODE_OBJECT:
		printf("]");
		break;
	case iNODE_CLOSURE:
		printf("}");
		break;
	}
}


void iParseTree_print(iParseTree *self){
	assert(self);

	iParseNode_print(&self->root);
}


iParseNode iParseNode_deepCopy(iParseNode *self){
	assert(self);

	iParseNode r;
	r.type = self->type;
	if(r.type == iNODE_LEAF){
		r.token = iToken_copy(self->token);
	}

	r.argc = self->argc;
	if(r.argc){
		r.argv = malloc(r.argc * sizeof(iParseNode));
		if(!r.argv){
			abort();
		}
		for(size_t i = 0; i < r.argc; i++){
			r.argv[i] = iParseNode_deepCopy(self->argv + i);
		}
	} else {
		r.argv = NULL;
	}
	return r;
}


// like iParseNode_clean but accounting for tokens, which are
// stored in iParseTree->tokenization regularly, but not in
// this case (for use in closures). Bad code.
void iParseNode_deepClean(iParseNode *self){
	assert(self);

	if(self->type == iNODE_LEAF){
		iToken_free(self->token);
	}

	if(self->argc > 0){
		for(size_t i = 0; i < self->argc; i++){
			iParseNode_deepClean(self->argv + i);
		}
		free(self->argv);
	}
}


bool iParseNode_isContextualRoute(iParseNode *self){
	return self->type == iNODE_LEAF 
	    && iToken_isContextualRoute(self->token);
}

