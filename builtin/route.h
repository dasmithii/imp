#ifndef IMP_BUILTIN_ROUTE_H_
#define IMP_BUILTIN_ROUTE_H_
#include "../c.h"
#include "../object.h"


void ImpRoute_init(Object *self);
void ImpRoute_print(Object *self);
char *ImpRoute_getRaw(Object *self);
void ImpRoute_setRaw(Object *self, char *text);
void ImpRoute_set(Object *self, Object *other);

Object *ImpRoute_mapping(Object *self, Object *context);


#endif