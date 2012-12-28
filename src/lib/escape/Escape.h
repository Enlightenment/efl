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
 * @page escape_main Escape
 * @date 2011 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref escape_main_intro
 * @li @ref escape_main_compiling
 * @li @ref escape_main_next_steps
 *
 * @section escape_main_intro Introduction
 *
 * The Escape library is a library that acts implements some of the missing
 * function from the PS3 GameOS system that are needed for the proper
 * functioning of the EFL.
 *
 * @section escape_main_compiling How to compile
 *
 * Escape is a library your application links to. The procedure for
 * this is very simple. You simply have to compile your application
 * with the appropriate compiler flags that the @p pkg-config script
 * outputs. For example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags escape`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs escape`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section escape_main_next_steps Next Steps
 *
 * After you understood what Escape is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Escape_Libgen_Group
 * @li @ref Escape_Unistd_Group
 * @li @ref Escape_Mman
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
