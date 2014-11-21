/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_MAIN_H_
#define EINA_MAIN_H_

#include "eina_types.h"

/**
 * @internal
 * @defgroup Eina_Main_Group Main
 * @ingroup Eina_Core_Group
 *
 * @brief This group discusses the functions that provide general initialisation and shut down
 *        functions.
 *
 * @{
 */

/**
 * @def EINA_VERSION_MAJOR
 * @brief Definition of the major version of Eina.
 */
#define EINA_VERSION_MAJOR 1

/**
 * @def EINA_VERSION_MINOR
 * @brief Definition of the minor version of Eina.
 */
#define EINA_VERSION_MINOR 8

/**
 * @typedef Eina_Version
 * @brief The structure type containing the version of Eina.
 */
typedef struct _Eina_Version
{
   int major;    /**< Major component of the version */
   int minor;    /**< Minor component of the version */
   int micro;    /**< Micro component of the version */
   int revision; /**< Revision component of the version */
} Eina_Version;

EAPI extern Eina_Version *eina_version;

/**
 * @brief Initializes the Eina library.
 *
 * @details This function sets up all the eina modules. It returns @c 0 on
 *          failure (that is, when one of the module fails to initialize),
 *          otherwise it returns the number of times it has already been
 *          called.
 *
 * @since_tizen 2.3
 *
 * @remarks When Eina is not used anymore, call eina_shutdown() to shut down
 *          the Eina library.
 *
 * @return @c 1 or greater on success, otherwise @c 0 on error
 *
 */
EAPI int eina_init(void);

/**
 * @brief Shuts down the Eina library.
 *
 * @details This function shuts down the Eina library. It returns @c 0 when it has
 *          been called the same number of times as eina_init(). In that case,
 *          it shuts down all the Eina modules.
 *
 *          Once this function succeeds (that is, @c 0 is returned), you must
 *          not call any of the Eina functions anymore. You must call
 *          eina_init() again to use the Eina functions again.
 *
 * @since_tizen 2.3
 *
 * @return @c 0 when all the modules are completely shut down, otherwise @c 1 or greater
 *
 */
EAPI int eina_shutdown(void);

/**
 * @brief Initializes the mutexes of the Eina library.
 *
 * @details This function sets up all the mutexes in all the eina modules. It returns @c 0 on
 *          failure (that is, when one of the module fails to initialize),
 *          otherwise it returns the number of times it has already been
 *          called.
 *
 * @since_tizen 2.3
 *
 * @remarks When the mutexes are not used anymore, call eina_threads_shutdown() to shut down
 *          the mutexes.
 *
 * @remarks This function should never be called outside the main loop.
 *
 * @return @c 1 or greater on success, otherwise @c 0 on error
 *
 */
EAPI int eina_threads_init(void);

/**
 * @brief Shuts down mutexes in the Eina library.
 *
 * @details This function shuts down the mutexes in the Eina library. It returns @c 0 when it has
 *          been called the same number of times as eina_threads_init(). In that case
 *          it shuts down all the mutexes.
 *
 *          Once this function succeeds (that is, @c 0 is returned), you must
 *          not call any of the Eina functions in a thread anymore. You must call
 *          eina_threads_init() again to use the Eina functions in a thread again.
 *
 * @since_tizen 2.3
 *
 * @remarks This function should never be called outside the main loop.
 *
 * @return @c 0 when all the mutexes are completely shut down, otherwise @c 1 or greater
 *
 */
EAPI int eina_threads_shutdown(void);

/**
 * @brief Check whether you are calling this function from the same thread in which Eina is initialized.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @remarks Most EFL functions are not thread safe and all the calls need to happen in
 *          the main loop. With this call you could know if you can call an EFL
 *          function.
 *
 * @return @c EINA_TRUE if the calling function is in the same thread, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_main_loop_is(void);

/**
 * @brief You should never use this function unless you really know what you are doing.
 *
 * @since_tizen 2.3
 *
 * @remarks If you are reading this documentation, that certainly means you don't know the purpose of
 *          this call, so you should not use it.
 */
EAPI void eina_main_loop_define(void);

/**
 * @}
 */

#endif /* EINA_MAIN_H_ */
