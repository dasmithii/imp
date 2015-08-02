#ifndef iPOSITION
#define iPOSITION




typedef struct {
	size_t line;
	size_t column;
} iPosition;


void iPosition_print(iPosition *self);
void iPosition_shift(iPosition *self, int n);
void iPosition_newLine(iPosition *self);




#endif