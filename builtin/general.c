#include "general.h"



void BuiltIn_setId(Object *self, BuiltinId id){
	BuiltinId *data = malloc(sizeof(BuiltinId));
	*data = id; 
	Object_putDataShallow(self, "_id", data);
}

BuiltinId BuiltIn_id(Object *self){
	BuiltinId *id = Object_getDataShallow(self, "_id");
	if(!id){
		return NOT_BUILTIN;
	}
	return *id;
}

bool BuiltIn_hasId(Object *self, BuiltinId id){
	return BuiltIn_id(self) == id;
}

BuiltinId BuiltIn_protoId(Object *self){
	Object *prototype = Object_getShallow(self, "_prototype");
	if(!prototype){
		return NOT_BUILTIN;
	}
	return BuiltIn_id(prototype);
}

bool BuiltIn_protoHadId(Object *self, BuiltinId id){
	return BuiltIn_protoId(self) == id;
}

