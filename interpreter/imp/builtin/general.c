#include <assert.h>

#include "general.h"




void iBuiltin_setId(iObject *self, iBuiltinId id){
	assert(self);

	iBuiltinId *data = malloc(sizeof(iBuiltinId));
	if(!data){
		abort();
	}
	*data = id; 
	iObject_putDataShallow(self, "__id", data);
}


iBuiltinId iBuiltin_id(iObject *self){
	if(!self){
		return iBUILTIN_NULL;
	}

	iBuiltinId *id = iObject_getDataDeep(self, "__id");
	if(!id){
		return iBUILTIN_NOT;
	}
	return *id;
}


bool iBuiltin_hasId(iObject *self, iBuiltinId id){
	assert(self);
	return iBuiltin_id(self) == id;
}


iBuiltinId iBuiltin_protoId(iObject *self){
	assert(self);
	iObject *prototype = iObject_getShallow(self, "#");
	if(!prototype){
		return iBUILTIN_NOT;
	}
	return iBuiltin_id(prototype);
}


bool iBuiltin_protoHadId(iObject *self, iBuiltinId id){
	assert(self);
	return iBuiltin_protoId(self) == id;
}
