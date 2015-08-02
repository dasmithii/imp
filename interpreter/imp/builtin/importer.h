#ifndef IMP_BUILTIN_IMPORTER_H_
#define IMP_BUILTIN_IMPORTER_H_

#include "../c.h"
#include "../object.h"




void iImporter_init(iObject *self, iRuntime *runtime);
iObject *i_import(iRuntime *runtime, char *module, iObject *context);




#endif