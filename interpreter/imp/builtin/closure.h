#ifndef IMP_BUILTIN_CLOSURE_H_
#define IMP_BUILTIN_CLOSURE_H_

#include "../c.h"
#include "../object.h"
#include "../parser.h"




//// management
void ImpClosure_init(Object *self, Runtime *runtime);
void ImpClosure_compile(Runtime *runtime, Object *self, ParseNode *code, Object *context);


//// miscellaneous 
void ImpClosure_print(Object *self);




#endif