#include "general.h"




void BuiltIn_setId(Object *self, BuiltinId id){
	assert(self);

	BuiltinId *data = malloc(sizeof(BuiltinId));
	if(!data){
		abort();
	}
	*data = id; 
	Object_putDataShallow(self, "__id", data);
}


BuiltinId BuiltIn_id(Object *self){
	assert(self);
	BuiltinId *id = Object_getDataDeep(self, "__id");
	if(!id){
		return NOT_BUILTIN;
	}
	return *id;
}


bool BuiltIn_hasId(Object *self, BuiltinId id){
	assert(self);
	return BuiltIn_id(self) == id;
}

BuiltinId BuiltIn_protoId(Object *self){
	assert(self);
	Object *prototype = Object_getShallow(self, "_prototype");
	if(!prototype){
		return NOT_BUILTIN;
	}
	return BuiltIn_id(prototype);
}

bool BuiltIn_protoHadId(Object *self, BuiltinId id){
	assert(self);
	return BuiltIn_protoId(self) == id;
}

