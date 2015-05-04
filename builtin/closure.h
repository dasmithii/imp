#ifndef IMP_BUILTIN_CLOSURE_H_
#define IMP_BUILTIN_CLOSURE_H_
#include "../c.h"
#include "../object.h"
#include "../parser.h"


void ImpClosure_init(Object *self);
void ImpClosure_print(Object *self);
void ImpClosure_compile(Object *self, ParseNode *code, Object *context);


#endif