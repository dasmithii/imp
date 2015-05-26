#include <stdio.h>
#include "object.h"
#include "runtime.h"

Object *tester_sayHi(Runtime *runtime
	        , Object *context
	        , Object *caller
	        , int argc
	        , Object **argv){
	printf("hi!\n");
	return NULL;
}
