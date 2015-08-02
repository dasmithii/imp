#ifndef IMP_BUILTIN
#define IMP_BUILTIN

#include "../object.h"
#include "../c.h"




typedef enum {
	iBUILTIN_ROUTE,
	iBUILTIN_STRING,
	iBUILTIN_NUMBER,
	iBUILTIN_CLOSURE,
	iBUILTIN_IMPORTER,
	iBUILTIN_OBJECT,

	iBUILTIN_NULL,

	iBUILTIN_BREAK,
	iBUILTIN_CONTINUE,

	iBUILTIN_NOT, // not built in (for errors)
} iBuiltinId;


void iBuiltin_setId(iObject *self, iBuiltinId id);
iBuiltinId iBuiltin_id(iObject *self);
iBuiltinId iBuiltin_protoId(iObject *self);
bool iBuiltin_protoHadId(iObject *self, iBuiltinId biid);




#endif