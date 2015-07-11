#ifndef IMP_BUILTIN_IMPORTER_H_
#define IMP_BUILTIN_IMPORTER_H_

#include "../c.h"
#include "../object.h"




void ImpImporter_init(Object *self, Runtime *runtime);
Object *Imp_import(Runtime *runtime, char *module);




#endif