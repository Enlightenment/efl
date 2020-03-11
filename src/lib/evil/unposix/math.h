#ifndef UNPOSIX_MATH_H
#define UNPOSIX_MATH_H


#ifdef _WIN32
# define _USE_MATH_DEFINES
#endif
#include_next <math.h>

#endif

