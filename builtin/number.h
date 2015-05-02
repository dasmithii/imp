#ifndef IMP_BUILTIN_NUMBER_H_
#define IMP_BUILTIN_NUMBER_H_
#include "../object.h"

// management functions
void ImpNumber_init(Object *self);

// in place arithmatic
void ImpNumber_add(Object *self, Object *other);
void ImpNumber_sub(Object *self, Object *other);
void ImpNumber_mult(Object *self, Object *other);
void ImpNumber_div(Object *self, Object *other);

// utilities
void ImpNumber_print(Object *self);
void ImpNumber_set(Object *self, Object *value);

void ImpNumber_setRaw(Object *self, double value);
double ImpNumber_getRaw(Object *self);

#endif