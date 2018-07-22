/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_MAIN_H_
#define EINA_MAIN_H_

#include <Efl_Config.h>

#include "eina_types.h"
#include "eina_error.h"

/**
 * @addtogroup Eina_Main_Group Main
 *
 * @brief These functions provide general initialization and shut down
 * functions.
 */

/**
 * @addtogroup Eina_Core_Group Core
 *
 * @{
 */

/**
 * @defgroup Eina_Main_Group Main
 *
 * @{
 */

/**
 * @def EINA_VERSION_MAJOR
 * @brief Definition for the major version of Eina.
 */
#define EINA_VERSION_MAJOR EFL_VERSION_MAJOR

/**
 * @def EINA_VERSION_MINOR
 * @brief Definition for the minor version of Eina.
 */
#define EINA_VERSION_MINOR EFL_VERSION_MINOR

/**
 * @typedef Eina_Version
 * The version of Eina.
 */
typedef struct _Eina_Version
{
   int major;    /**< Major component of the version */
   int minor;    /**< Minor component of the version */
   int micro;    /**< Micro component of the version */
   int revision; /**< Revision component of the version */
} Eina_Version;

EAPI extern Eina_Version *eina_version;

EAPI extern Eina_Error EINA_ERROR_NOT_IMPLEMENTED;

/**
 * @brief Initializes the Eina library.
 *
 * @return @c 1 or greater on success, @c 0 on error.
 *
 * This function sets up all the eina modules. It returns 0 on
 * failure (that is, when one of the module fails to initialize),
 * otherwise it returns the number of times it has already been
 * called.
 *
 * When Eina is not used anymore, call eina_shutdown() to shut down
 * the Eina library.
 *
 * This function is affected by the following environment variables.
 *
 * Logging environment variables:
 *
 *  @li @c EINA_LOG_ABORT=1 will cause abort(3) if @c
 *      EINA_LOG_ABORT_LEVEL, which defaults to 0 (critical), is
 *      reached.
 *
 *  @li @c EINA_LOG_ABORT_LEVEL=LEVEL, where level is an integer such
 *      as 0 (critical) to 4 (debug). This will cause any messages at
 *      that level or less (i.e.: if 4, then messages at level 0 also
 *      apply) to abort(3) the application if @c EINA_LOG_ABORT=1 was
 *      defined.
 *
 *  @li @c EINA_LOG_BACKTRACE=LEVEL, where level is an integer such as
 *      0 (critical) to 4 (debug). This will cause any messages at
 *      that level or less (i.e.: if 4, then messages at level 0 also
 *      apply) to dump a backtrace. Note that if systemd-journal is
 *      used, it's going to the journal as well.
 *
 *  @li @c EINA_LOG_COLOR_DISABLE=[0|1] if @c 1 it will disable color
 *      in log output. If it's @c 0, it will keep colors. By default
 *      it will use colors if it's a TTY, disabling colors if it's
 *      printing to a file.
 *
 *  @li @c EINA_LOG_FILE_DISABLE=[0|1] if @c 1 it will disable showing
 *      the source file name and line that generated the message. If
 *      @c 0, the default, it will show the source file and line.
 *
 *  @li @c EINA_LOG_FUNCTION_DISABLE=[0|1] if @c 1 it will disable
 *      showing the source function name that generated the
 *      message. If @c 0, the default, it will show the function name.
 *
 *  @li @c EINA_LOG_LEVEL=LEVEL, where level is an integer such as 0
 *      (critical) to 4 (debug), where the number is the the highest
 *      level to print, that is, using level=4 will lead to messages
 *      of level 1, 2 and 3 to be displayed as well, but not level
 *      5. This will control all log domains at once and has lower
 *      priority than @c EINA_LOG_LEVELS, that defines per-domain.
 *
 *  @li @c EINA_LOG_LEVELS=DOMAIN1:LEVEL1,DOMAIN2:LEVEL2 is a comma
 *      separated list of tuples of DOMAIN and LEVEL values separated
 *      by a colon. Level is an integer such as 0 (critical) to 4
 *      (debug), where the number is the the highest level to print,
 *      that is, using level=4 will lead to messages of level 1, 2 and
 *      3 to be displayed as well, but not level 5. This offers fine
 *      grained control, allowing a single domain to be printed at
 *      debug (4) while keeping others at error (1) only. For instance
 *      @c EINA_LOG_LEVELS=eina:2,eina_binshare:4 will change eina's
 *      general purpose logging domain to warning (2) while will make
 *      the specific eina_binshare debug (4).
 *
 *  @li @c EINA_LOG_LEVELS_GLOB=PATTERN1:LEVEL1,PATTERN2:LEVEL2 is a
 *      comma separated list of tuples of domain PATTERN and LEVEL
 *      values separated by a colon. PATTERN is a fnmatch(3) pattern
 *      that will try to match the domain name. Level is an integer
 *      such as 0 (critical) to 4 (debug), where the number is the the
 *      highest level to print, that is, using level=4 will lead to
 *      messages of level 1, 2 and 3 to be displayed as well, but not
 *      level 5. This offers fine grained control, allowing a single
 *      domain to be printed at debug (4) while keeping others at
 *      error (1) only. For instance @c EINA_LOG_LEVELS_GLOB=eina*:4
 *      will change all domain names starting with "eina", such as
 *      "eina" (general purpose log domain) and "eina_binshare".
 *
 * Memory allocator environment variables:
 *
 *  @li @c EINA_MEMPOOL=mempool_allocator will choose another memory
 *      allocator to use. Commonly this is given @c pass_through as
 *      the @c mempool_allocator to allow valgrind and other tools to
 *      debug it more easily.
 *
 *  @li @c EINA_MEMPOOL_PASS=1 has the almost the same effect of @c
 *      EINA_MEMPOOL=pass_through, but if pass_through allocator
 *      wasn't built, it will use the original allocator.
 *
 *  @li @c EINA_MTRACE=1 and @c MALLOC_TRACE=PATH will use mtrace(3)
 *      and store the memory allocation traces to given @c PATH.
 *
 * Module loading environment variables:
 *
 *  @li @c EINA_MODULE_LAZY_LOAD=1 will dlopen(3) with RTLD_LAZY, this
 *      results in faster startup since symbols will be resolved when
 *      needed, but may omit errors as they will be triggered later
 *      on.
 *
 * Threads environment variables:
 *
 *  @li @c EINA_DEBUG_THREADS=1 will print out debug information about
 *      threads, but requires Eina to be compiled with such feature.
 *
 */
EAPI int eina_init(void);

/**
 * @brief Shuts down the Eina library.
 *
 * @return @c 0 when all the modules are completely shut down, @c 1 or
 * greater otherwise.
 *
 * This function shuts down the Eina library. It returns 0 when it has
 * been called the same number of times than eina_init(). In that case
 * it shuts down all the Eina modules.
 *
 * Once this function succeeds (that is, @c 0 is returned), you must
 * not call any of the Eina functions anymore. You must call
 * eina_init() again to use the Eina functions again.
 */
EAPI int eina_shutdown(void);

/**
 * @brief Initializes the mutexes of the Eina library.
 *
 * @return @c 1 or greater on success, @c 0 on error.
 *
 * This function sets up all the mutexes in all eina modules. It returns 0 on
 * failure (that is, when one of the module fails to initialize),
 * otherwise it returns the number of times it has already been
 * called.
 *
 * When the mutexes are not used anymore, call eina_threads_shutdown() to shut down
 * the mutexes.
 *
 * This function should never be called outside of the main loop.
 */
EAPI int eina_threads_init(void);

/**
 * @brief Shuts down mutexes in the Eina library.
 *
 * @return @c 0 when all mutexes are completely shut down, @c 1 or
 * greater otherwise.
 *
 * This function shuts down the mutexes in the Eina library. It returns 0 when it has
 * been called the same number of times than eina_threads_init(). In that case
 * it shut down all the mutexes.
 *
 * Once this function succeeds (that is, @c 0 is returned), you must
 * not call any of the Eina functions in a thread anymore. You must call
 * eina_threads_init() again to use the Eina functions in a thread again.
 *
 * This function should never be called outside of the main loop.
 */
EAPI int eina_threads_shutdown(void);

/**
 * @brief Checks if you are calling this function from the same thread Eina was initialized or not.
 *
 * @return #EINA_TRUE is the calling function is the same thread, #EINA_FALSE otherwise.
 *
 * @since 1.1.0
 *
 * Most EFL functions are not thread safe and all the calls need to happen in
 * the main loop. With this call you could know if you can call an EFL
 * function or not.
 */
EAPI Eina_Bool eina_main_loop_is(void);

/**
 * @brief You should never use this function except if you really really know what your are doing.
 * @since 1.1.0
 *
 * If you are reading this documentation, that certainly means you don't know what is the purpose of
 * this call and you should just not use it.
 */
EAPI void eina_main_loop_define(void);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_MAIN_H_ */
