#include <stdio.h>

#include "position.h"




void iPosition_print(iPosition *self){
	printf("[%zu:%zu]", self->line, self->column);
}


void iPosition_shift(iPosition *self, int n){
	self->column += n;
}


void iPosition_newLine(iPosition *self){
	self->column = 0;
	self->line++;
}
