#ifndef IMP_OBJECT_POOL_H_
#define IMP_OBJECT_POOL_H_
#include "object.h"


typedef struct ImpObjectPool_* ImpObjectPool;

ImpObjectPool ImpObjectPool_forRuntime(void *runtime);
Object *ImpObjectPool_allocate(ImpObjectPool self);
void ImpObjectPool_lockGC(ImpObjectPool self);
void ImpObjectPool_unlockGC(ImpObjectPool self);
void ImpObjectPool_free(ImpObjectPool self);



#endif