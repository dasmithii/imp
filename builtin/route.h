#ifndef IMP_BUILTIN_ROUTE_H_
#define IMP_BUILTIN_ROUTE_H_

#include "../c.h"
#include "../object.h"


bool ImpRoute_isValid(Object *self);

void ImpRoute_init(Object *self);
void ImpRoute_print(Object *self);
char *ImpRoute_getRaw(Object *self);
void ImpRoute_setRaw(Object *self, char *text);
void ImpRoute_set(Object *self, Object *other);

Object *ImpRoute_submapping(Object *self, Object *context);
Object *ImpRoute_submapping_(char *self, Object *context);
Object *ImpRoute_mapping(Object *self, Object *context);
Object *ImpRoute_mapping_(char *self, Object *context);

int ImpRoute_argc(Object *self);
void ImpRoute_argv(Object *self, int i, char *dest);

Object *unrouteInContext(Object *obj, Object *ctx);


#endif