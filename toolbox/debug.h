// Baseline debug utilities are built atop the standard `assert` macro,
// and are nullified accordingly when NDEBUG is defined.
//
// In place of assertions, we use assumptions, and provide various 
// helpful assumption macros, all derived from the base assume_m().
#ifndef _BASELINE_ASSUMPTIONS_
#define _BASELINE_ASSUMPTIONS_
#include <assert.h>

#define assume_m(cond, msg) \
	assert((cond) && msg)

#define assume(cond) \
	assume_m((cond), "Assumption failed.")

#define assume_ptr(a) \
	assume_m((a), "Unexpected null pointer.");

#define assume_ptrs(a, b) \
	assume_ptr((a));      \
 	assume_ptr((b))

#define assume_max(n, max) \
 	assume_m((n) <= (max), "Value too large.")

#define assume_ceil(n, ceil) \
 	assume_m((n) < (ceil), "Value too large.")

#define assume_min(n, min) \
 	assume_m((n) >= (min), "Value too small.")

#define assume_floor(n, floor) \
 	assume_m((n) > (floor), "Value too small.");

#define assume_bound_ii(n, min, max) \
 	assume_min(n, min);              \
 	assume_max(n, max)

#define assume_bound_ee(n, floor, ceil) \
 	assume_floor(n, min);               \
 	assume_ceil(n, max)

#define assume_bound_ie(n, min, ceil) \
 	assume_min(n, min);               \
 	assume_ceil(n, ceil)

#define assume_bound_ei(n, floor, max) \
 	assume_floor(n, floor);            \
 	assume_max(n, max)

#define assume_pos(n) \
 	assume_m((n) > 0, "Non-positive number.")

#define assume_neg(n) \
 	assume_m((n)) < 0, "Non-negative number.")

#define assume_nzero(n) \
 	assume_m((n) != 0, "Expected non-zero.")

#endif