// Errors are handled like assumptions, except their checks exist
// in production-ready programs. And rather than exiting immediately,
// they goto the "error:" label, where necessary deallocations are
// handled.
#ifndef _BASELINE_ERROR_HANDLING_
#define _BASELINE_ERROR_HANDLING_

#define error(msg)                                                       \
	fprintf(stderr, "%s (line: %d, file: %s)", msg, __LINE__, __FILE__); \
	goto error

#define check_m(cond, msg)  \
	if((cond) == 0)         \
		error(msg)

#define check(cond) \
	check_m((cond), "Unspecified Error.");

#define check_ptr(a) \
	check_m((a), "Unexpected null pointer.");

#define check_ptrs(a, b) \
	check_ptr((a));      \
 	check_ptr((b))

#define check_max(n, max) \
 	check_m((n) <= (max), "Value too large.")

#define check_ceil(n, ceil) \
 	check_m((n) < (max), "Value too large.")

#define check_min(n, min) \
 	check_m((n) >= (min), "Value too small.")

#define check_floor(n, floor) \
 	check_m((n) > (floor), "Value too small.");

#define check_bound_ii(n, min, max) \
 	check_min(n, min);              \
 	check_max(n, max)

#define check_bound_ee(n, floor, ceil) \
 	check_floor(n, min);               \
 	check_ceil(n, max)

#define check_bound_ie(n, min, ceil) \
 	check_min(n, min);               \
 	check_ceil(n, ceil)

#define check_bound_ei(n, floor, max) \
 	check_floor(n, floor);            \
 	check_max(n, max)

#define check_pos(n) \
 	check_m((n) > 0, "Non-positive number.");

#define check_neg(n) \
 	check_m((n) < 0, "Non-negative number.");

 #define check_nzero(n) \
 	check_m((n) != 0, "Expected non-zero.")


#endif