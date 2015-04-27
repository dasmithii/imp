#ifndef BASELINE_STACK
#define BASELINE_STACK
#include <stdbool.h>
#include "vector.h"


typedef Vector Stack;
#define Stack_init(self, size)      Vector_init(self, size)
#define Stack_clean(self)           Vector_clean(self)
#define Stack_push(self, addr)      Vector_append(self, addr)
#define Stack_pop(self)             Vector_pop(self)
#define Stack_hookTop(self)         Vector_hookLast(self)
#define Stack_fetchTop(self, addr)  Vector_fetchLast(self, addr)
#define Stack_size(self)            ((self)->size)
#define Stack_isEmpty(self)         (Stack_size(self)? true:false)
#define Stack_clear(self)           Vector_clear(self)


#endif