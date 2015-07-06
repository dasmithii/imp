#ifndef IMP_TASK_H_
#define IMP_TASK_H_

#include "object.h"



typedef struct {
	Object *lastReturnValue;
	bool    returnWasCalled; 
} Task;



#endif