#ifndef IMP_RUNTIME_H_
#define IMP_RUNTIME_H_
#include "toolbox/vector.h"
#include "object.h"
#include "parser.h"


typedef struct {
	char *error;
	Object *root_scope;

	Vector collectables;  // TODO: use queue or something lockless instead of vector
} Runtime;


void Runtime_init(Runtime *self);
void Runtime_clean(Runtime *self);
Object *Runtime_execute(Runtime *self, char *code);
Object *Runtime_rawObject();

Object *Runtime_activate(Runtime *runtime
	                   , Object *object
	                   , Object *context
	                   , int argc
	                   , Object **argv);

Object *Runtime_executeInContext(Runtime *runtime
	                              , Object *context
	                              , ParseNode node);


#endif