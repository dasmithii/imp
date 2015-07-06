#include <stdio.h>

#include "position.h"




void Position_print(Position *self){
	printf("[%zu:%zu]", self->line, self->column);
}


void Position_shift(Position *self, int n){
	self->column += n;
}


void Position_newLine(Position *self){
	self->column = 0;
	self->line++;
}
