#ifndef __ESCAPE_H__
#define __ESCAPE_H__

#ifndef EAPI
#define EAPI
#endif /* EAPI */
#define __UNUSED__ __attribute__((unused))

#include <unistd.h>

#ifdef CLOCK_REALTIME
#undef CLOCK_REALTIME
#endif
#ifdef CLOCK_PROF
#undef CLOCK_PROF
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
#undef CLOCK_PROCESS_CPUTIME_ID
#endif

/**
 * @mainpage Escape
 * @image html  e_big.png
 * @author Youness Alaoui
 * @date 2011
 *
 * @section intro_sec Introduction
 *
 * The Escape library is a library that acts implements some of the missing
 * function from the PS3 GameOS system that are needed for the proper
 * functioning of the EFL.
 *
 * @section escape_sec Escape API Documentation
 *
 * Take a look at the documentation of @ref mmap/munmap.
 *
 */

#include "escape_libgen.h"
#include "escape_unistd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Nothing to see here */

#ifdef __cplusplus
}
#endif

#endif /* __ESCAPE_H__ */
