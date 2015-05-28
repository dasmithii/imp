#ifndef IMP_BUILTIN_VECTOR_H_
#define IMP_BUILTIN_VECTOR_H_

#include "../object.h"
#include "../toolbox/vector.h"


void ImpVector_init(Object *self);
Vector *ImpVector_getRaw(Object *self);


#endif