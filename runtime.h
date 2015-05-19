#ifndef IMP_RUNTIME_H_
#define IMP_RUNTIME_H_
#include "toolbox/vector.h"
#include "object.h"
#include "parser.h"


typedef struct {
	char *error;
	Object *root_scope;

	Vector collectables;  // TODO: use queue or something lockless instead of vector

	Object *lastReturnValue; // TODO: track return values for each coroutine

	int gc_locks;
} Runtime;


void Runtime_init(Runtime *self);
void Runtime_clean(Runtime *self);
Object *Runtime_execute(Runtime *self, char *code);
Object *Runtime_rawObject(Runtime *self);
Object *Runtime_clone(Runtime *self, Object *base);
Object *Runtime_cloneField(Runtime *self, char *name);

Object *Runtime_activateOn(Runtime *runtime
	                     , Object *context
	                     , Object *caller
	                     , int argc
	                     , Object **argv
	                     , Object *origin);

Object *Runtime_activate(Runtime *runtime
	                   , Object *context
	                   , Object *caller
	                   , int argc
	                   , Object **argv);

Object *Runtime_executeInContext(Runtime *runtime
	                              , Object *context
	                              , ParseNode node);

void Runtime_throw(Runtime *runtime, Object *exception);
void Runtime_throwString(Runtime *runtime, char *exception);
void Runtime_markRecursive(Runtime *runtime, Object *object);
Object *Runtime_shallowCopy(Runtime *runtime, Object *object);

int Runtime_objectCount(Runtime *self);

void Runtime_setReturnValue(Runtime *self, Object *value);
void Runtime_clearReturnValue(Runtime *self);
Object *Runtime_returnValue(Runtime *self);

void Runtime_print(Runtime *self, Object *context, Object *object);
void Runtime_interrupt(Runtime *self);

void Runtime_lockGC(Runtime *self);
void Runtime_unlockGC(Runtime *self);

#endif