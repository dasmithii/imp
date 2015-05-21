#ifndef IMP_BUILTIN_BOOLEAN_H_
#define IMP_BUILTIN_BOOLEAN_H_
#include "../object.h"


//// management
void ImpBoolean_init(Object *self);
bool ImpBoolean_getRaw(Object *self);
void ImpBoolean_setRaw(Object *self, bool value);


//// helpers
void ImpBoolean_setTrue(Object *self);
void ImpBoolean_setFalse(Object *self);
void ImpBoolean_negate(Object *self);


#endif