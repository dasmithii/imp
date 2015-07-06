#ifndef IMP_CODE_POSITION_H_
#define IMP_CODE_POSITION_H_




typedef struct {
	size_t line;
	size_t column;
} Position;


void Position_print(Position *self);
void Position_shift(Position *self, int n);
void Position_newLine(Position *self);




#endif