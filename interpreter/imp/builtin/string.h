#ifndef IMP_BUILTIN_STRING_H_
#define IMP_BUILTIN_STRING_H_

#include "../c.h"
#include "../object.h"


bool iString_isValid(iObject *self);
void iString_init(iObject *self, iRuntime *runtime);
void iString_print(iObject *self);
char *iString_getRaw(iObject *self);
void iString_setRaw(iObject *self, char *text);
void iString_setRawPointer(iObject *self, char *text);
void iString_set(iObject *self, iObject *other);
void iString_concatenateRaw(iObject *self, char *text);

#endif