#ifndef IMP_RUNTIME_H_
#define IMP_RUNTIME_H_

#include "toolbox/vector.h"
#include "toolbox/stack.h"
#include "object.h"
#include "parser.h"




typedef struct {
	char *error;
	Object *root_scope;
	Vector collectables;  // TODO: use queue or something lockless instead of vector

	Object *lastReturnValue; // TODO: track return values for each coroutine

	int gc_locks;
	bool gc_on;

	Stack tryStack; // stack of try calls
} Runtime;


//// management
void Runtime_init(Runtime *self);
void Runtime_clean(Runtime *self);


//// object creation
Object *Runtime_rawObject(Runtime *self);
Object *Runtime_clone(Runtime *self, Object *base);
Object *Runtime_cloneField(Runtime *self, char *name);


//// code execution
Object *Runtime_executeSource(Runtime *self, char *code);
Object *Runtime_executeSourceInContext(Runtime *self, char *code, Object *context);
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


//// return value 'register'
void Runtime_setReturnValue(Runtime *self, Object *value);
void Runtime_clearReturnValue(Runtime *self);
Object *Runtime_returnValue(Runtime *self);


//// exception handling
void Runtime_throw(Runtime *runtime, Object *exception);
void Runtime_throwString(Runtime *runtime, char *exception);
void Runtime_throwFormatted(Runtime *runtime, const char *format, ...);


//// garbage collection
void Runtime_lockGC(Runtime *self);
void Runtime_unlockGC(Runtime *self);
bool Runtime_isManaged(Runtime *self, Object *object);
void Runtime_markRecursive(Runtime *runtime, Object *object);
int Runtime_objectCount(Runtime *self);


//// miscellaneous
void Runtime_print(Runtime *self, Object *context, Object *object);


#endif