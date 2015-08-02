#ifndef IMP_OBJECT_POOL_H_
#define IMP_OBJECT_POOL_H_
#include "object.h"


typedef struct iObjectPool_* iObjectPool;

iObjectPool iObjectPool_forRuntime(void *runtime);
iObject *iObjectPool_allocate(iObjectPool self);
void iObjectPool_lockGC(iObjectPool self);
void iObjectPool_unlockGC(iObjectPool self);
void iObjectPool_free(iObjectPool self);



#endif