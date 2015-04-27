#ifndef BASELINE_BIT_UTILITIES
#define BASELINE_BIT_UTILITIES
#include <stdbool.h>


#define GET_BIT(n, i) (((unsigned) n) & (1 << (i)) >> (i))? true:false


#define SET_BIT(n, i, v) \
	if(v) FLAG_BIT(n,i); \
	else  CLEAR_BIT(n,i)


#define FLAG_BIT(n,i)\
	n |= 1 << (unsigned) (i)


#define CLEAR_BIT(n,i)\
	n &= ~(1 << (unsigned) i)


#define TOGGLE_BIT(n, i)\
	SET_BIT(n, i, !(GET_BIT(n, i)))


#endif