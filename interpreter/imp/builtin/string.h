#ifndef IMP_BUILTIN_STRING_H_
#define IMP_BUILTIN_STRING_H_

#include "../c.h"
#include "../object.h"


bool ImpString_isValid(Object *self);
void ImpString_init(Object *self, Runtime *runtime);
void ImpString_print(Object *self);
char *ImpString_getRaw(Object *self);
void ImpString_setRaw(Object *self, char *text);
void ImpString_set(Object *self, Object *other);
void ImpString_concatenateRaw(Object *self, char *text);

#endif