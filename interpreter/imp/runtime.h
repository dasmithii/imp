#ifndef IMP_RUNTIME_H_
#define IMP_RUNTIME_H_

#include <imp/object.h>
#include <imp/parser.h>
#include "ObjectPool.h"




typedef struct {
	char *error;
	Object *root_scope;

	ImpObjectPool objectPool;

	Object *lastReturnValue; // TODO: track return values for each coroutine
	bool    returnWasCalled; 

	// configuration
	char *root; // importer search path (like goroot)
	int argc;
	char **argv;

	// built-ins
	Object *Object;
	Object *Number;
	Object *String;
	Object *Array;
	Object *Closure;
	Object *Route;
	Object *nil;

	Object *Importer; // activate-able import object
	Object *imports; // cache of dlls
} Runtime;


//// management
void Runtime_init(Runtime *self, char *root, int argc, char **argv);
void Runtime_clean(Runtime *self);


//// object creation
Object *Runtime_rawObject(Runtime *self); // TODO: replace with make(Empty)
Object *Runtime_clone(Runtime *self, Object *base);
Object *Runtime_simpleClone(Runtime *self, Object *base);
Object *Runtime_cloneField(Runtime *self, char *name);

// T should be Object/Number/String/Vector/Closure/Route
#define Runtime_make(R, T) Runtime_clone(R, R->T)


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


//// exception handling
void Runtime_throw(Runtime *runtime, Object *exception);
void Runtime_throwString(Runtime *runtime, char *exception);
void Runtime_throwFormatted(Runtime *runtime, const char *format, ...);


//// garbage collection
bool Runtime_isManaged(Runtime *self, Object *object);
int Runtime_objectCount(Runtime *self);


//// miscellaneous
void Runtime_print(Runtime *self, Object *context, Object *object);
Object *Runtime_callMethod(Runtime *self
	                     , Object *context
	                     , Object *object
	                     , char *methodName
	                     , int argc
	                     , Object **argv);
Object *Runtime_callSpecialMethod(Runtime *self
	                     , Object *context
	                     , Object *object
	                     , char *methodName
	                     , int argc
	                     , Object **argv);


#endif