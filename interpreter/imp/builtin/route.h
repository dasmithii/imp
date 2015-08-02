#ifndef IMP_BUILTIN_ROUTE_H_
#define IMP_BUILTIN_ROUTE_H_

#include "../object.h"




bool iRoute_isValid(iObject *self);
void iRoute_init(iObject *self, iRuntime *runtime);
void iRoute_print(iObject *self);

char *iRoute_getRaw(iObject *self);
void iRoute_setRaw(iObject *self, char *text);
void iRoute_set(iObject *self, iObject *other);

void iRoute_setContext(iObject *self, iObject *context);

iObject *iRoute_submapping(iObject *self);
iObject *iRoute_submapping_(char *self, iObject *context);
iObject *iRoute_mapping(iObject *self);
iObject *iRoute_mapping_(char *self, iObject *context);

int iRoute_argc(iObject *self);
void iRoute_argv(iObject *self, int i, char *dest);

iObject *unroute(iObject *obj);




#endif