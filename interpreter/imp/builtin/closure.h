#ifndef IMP_BUILTIN_CLOSURE_H_
#define IMP_BUILTIN_CLOSURE_H_

#include "../c.h"
#include "../object.h"
#include "../parser.h"




void iClosure_init(iObject *self, iRuntime *runtime);
void iClosure_compile(iRuntime *runtime, iObject *self, iParseNode *code, iObject *context);


void iClosure_print(iObject *self);




#endif