#ifndef IMP_BUILTIN_NUMBER_H_
#define IMP_BUILTIN_NUMBER_H_

#include "../object.h"




void ImpNumber_init(Object *self, Runtime *runtime);
bool ImpNumber_isValid(Object *self);
void ImpNumber_print(Object *self);

void ImpNumber_add(Object *self, Object *other);
void ImpNumber_sub(Object *self, Object *other);
void ImpNumber_mult(Object *self, Object *other);
void ImpNumber_div(Object *self, Object *other);

void ImpNumber_set(Object *self, Object *value);
void ImpNumber_setRaw(Object *self, double value);
double ImpNumber_getRaw(Object *self);
int ImpNumber_getRawRounded(Object *self);




#endif