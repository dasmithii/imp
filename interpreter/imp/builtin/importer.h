#ifndef IMP_BUILTIN_IMPORTER_H_
#define IMP_BUILTIN_IMPORTER_H_

#include "../c.h"
#include "../object.h"


void ImpImporter_init(Object *self);
void Imp_import(Runtime *runtime
	          , Object *context
	          , char *module);


#endif