#ifndef IMP_BUILTIN_ATOM_H_
#define IMP_BUILTIN_ATOM_H_
#include "../c.h"
#include "../object.h"


void ImpAtom_init(Object *self);
void ImpAtom_print(Object *self);
char *ImpAtom_getRaw(Object *self);
void ImpAtom_setRaw(Object *self, char *text);
void ImpAtom_set(Object *self, Object *other);

Object *ImpAtom_mapping(Object *self, Object *context);


#endif