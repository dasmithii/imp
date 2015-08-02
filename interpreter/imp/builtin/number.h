#ifndef IMP_BUILTIN_NUMBER_H_
#define IMP_BUILTIN_NUMBER_H_

#include "../object.h"




void iNumber_init(iObject *self, iRuntime *runtime);
bool iNumber_isValid(iObject *self);
void iNumber_print(iObject *self);

void iNumber_add(iObject *self, iObject *other);
void iNumber_sub(iObject *self, iObject *other);
void iNumber_mult(iObject *self, iObject *other);
void iNumber_div(iObject *self, iObject *other);

void iNumber_set(iObject *self, iObject *value);
void iNumber_setRaw(iObject *self, double value);
double iNumber_getRaw(iObject *self);
int iNumber_getRawRounded(iObject *self);




#endif