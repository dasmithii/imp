#ifndef IMP_BUILTIN_H_
#define IMP_BUILTIN_H_

#include "../object.h"
#include "../c.h"


typedef enum {
	BUILTIN_ROUTE,
	BUILTIN_STRING,
	BUILTIN_NUMBER,
	BUILTIN_CLOSURE,
	BUILTIN_IMPORTER,
	BUILTIN_OBJECT,

	BUILTIN_NULL,
	NOT_BUILTIN,

	BUILTIN_BREAK,
	BUILTIN_CONTINUE
} BuiltinId;


void BuiltIn_setId(Object *self, BuiltinId id);
BuiltinId BuiltIn_id(Object *self);
BuiltinId BuiltIn_protoId(Object *self);
bool BuiltIn_protoHadId(Object *self, BuiltinId biid);


#endif