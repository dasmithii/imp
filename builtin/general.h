#ifndef IMP_BUILTIN_H_
#define IMP_BUILTIN_H_
#include "../object.h"
#include "../c.h"


typedef enum {
	BUILTIN_ATOM,
	BUILTIN_STRING,
	BUILTIN_NUMBER,
	BUILTIN_CLOSURE,
	BUILTIN_PRINTER,
	BUILTIN_DEFINER,
	BUILTIN_SETTER,
	NOT_BUILTIN
} BuiltinId;


void BuiltIn_setId(Object *self, BuiltinId id);
BuiltinId BuiltIn_id(Object *self);
BuiltinId BuiltIn_protoId(Object *self);
bool BuiltIn_protoHadId(Object *self, BuiltinId biid);



#endif