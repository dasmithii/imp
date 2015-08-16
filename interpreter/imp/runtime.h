#ifndef IMP_RUNTIME_H_
#define IMP_RUNTIME_H_

#include <imp/object.h>
#include <imp/parser.h>
#include "ObjectPool.h"




typedef struct {
	char *error;
	iObject *root_scope;

	iObjectPool objectPool;

	iObject *lastReturnValue; // TODO: track return values for each coroutine
	bool    returnWasCalled; 

	// configuration
	char *root; // importer search path (like goroot)
	int argc;
	char **argv;

	// built-ins
	iObject *Object;
	iObject *Number;
	iObject *String;
	iObject *Array;
	iObject *Closure;
	iObject *Route;
	iObject *nil;

	iObject *Importer; // activate-able import object
	iObject *imports; // cache of dlls
} iRuntime;


//// management
void iRuntime_init(iRuntime *self, char *root, int argc, char **argv);
void iRuntime_clean(iRuntime *self);


//// object creation
iObject *iRuntime_rawObject(iRuntime *self); // TODO: replace with make(Empty)
iObject *iRuntime_clone(iRuntime *self, iObject *base);
iObject *iRuntime_simpleClone(iRuntime *self, iObject *base);
iObject *iRuntime_cloneField(iRuntime *self, char *name);

// T should be Object/Number/String/Vector/Closure/Route
#define iRuntime_MAKE(R, T) iRuntime_clone(R, (R)->T)


//// code execution
iObject *iRuntime_executeSource(iRuntime *self, char *code);
iObject *iRuntime_executeSourceInContext(iRuntime *self, char *code, iObject *context);
iObject *iRuntime_executeFile(iRuntime *self, char *path);
iObject *iRuntime_executeFileInContext(iRuntime *self, char *path, iObject *context);
iObject *iRuntime_activateOn(iRuntime *runtime
	                       , iObject *context
	                       , iObject *caller
	                       , int argc
	                       , iObject **argv
	                       , iObject *origin);
iObject *iRuntime_activate(iRuntime *runtime
	                     , iObject *context
	                     , iObject *caller
	                     , int argc
	                     , iObject **argv);
iObject *iRuntime_executeInContext(iRuntime *runtime
	                             , iObject *context
	                             , iParseNode node);


uint64_t iRuntime_hashCodeOf(iRuntime *runtime, iObject *context, iObject *object);



//// return value 'register'
void iRuntime_setReturnValue(iRuntime *self, iObject *value);
void iRuntime_clearReturnValue(iRuntime *self);


//// exception handling
void iRuntime_throw(iRuntime *runtime, iObject *context, iObject *exception);
void iRuntime_throwString(iRuntime *runtime, iObject *context, char *exception);
void iRuntime_throwFormatted(iRuntime *runtime, iObject *context, const char *format, ...);


//// garbage collection
bool iRuntime_isManaged(iRuntime *self, iObject *object);
int iRuntime_objectCount(iRuntime *self);


//// miscellaneous
void iRuntime_print(iRuntime *self, iObject *context, iObject *object);

iObject *iRuntime_callMethod(iRuntime *self
	                       , iObject *context
	                       , iObject *object
	                       , char *methodName
	                       , int argc
	                       , iObject **argv);
iObject *iRuntime_callSpecialMethod(iRuntime *self
	                              , iObject *context
	                              , iObject *object
	                              , char *methodName
	                              , int argc
	                              , iObject **argv);


#endif
