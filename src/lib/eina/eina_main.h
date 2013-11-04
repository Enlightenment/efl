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

/**
 * @addtogroup Eina_Main_Group Main
 *
 * @brief These functions provide general initialisation and shut down
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
 * @brief Major version of Eina
 */
#define EINA_VERSION_MAJOR EFL_VERSION_MAJOR

/**
 * @def EINA_VERSION_MINOR
 * @brief Minor version of Eina
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

/**
 * @brief Initialize the Eina library.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up all the eina modules. It returns 0 on
 * failure (that is, when one of the module fails to initialize),
 * otherwise it returns the number of times it has already been
 * called.
 *
 * When Eina is not used anymore, call eina_shutdown() to shut down
 * the Eina library.
 */
EAPI int eina_init(void);

/**
 * @brief Shut down the Eina library.
 *
 * @return 0 when all the modules are completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Eina library. It returns 0 when it has
 * been called the same number of times than eina_init(). In that case
 * it shut down all the Eina modules.
 *
 * Once this function succeeds (that is, @c 0 is returned), you must
 * not call any of the Eina function anymore. You must call
 * eina_init() again to use the Eina functions again.
 */
EAPI int eina_shutdown(void);

/**
 * @brief Initialize the mutexes of the Eina library.
 *
 * @return 1 or greater on success, 0 on error.
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
 * @brief Shut down mutexes in the Eina library.
 *
 * @return 0 when all mutexes are completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the mutexes in the Eina library. It returns 0 when it has
 * been called the same number of times than eina_threads_init(). In that case
 * it shut down all the mutexes.
 *
 * Once this function succeeds (that is, @c 0 is returned), you must
 * not call any of the Eina function in a thread anymore. You must call
 * eina_threads_init() again to use the Eina functions in a thread again.
 *
 * This function should never be called outside of the main loop.
 */
EAPI int eina_threads_shutdown(void);

/**
 * @brief Check if you are calling this function from the same thread Eina was initialized or not
 *
 * @return #EINA_TRUE is the calling function is the same thread, #EINA_FALSE otherwise.
 *
 * @since 1.1.0
 *
 * Most EFL function are not thread safe and all the call need to happen in
 * the main loop. With this call you could know if you can call an EFL
 * function or not.
 */
EAPI Eina_Bool eina_main_loop_is(void);

/**
 * @brief You should never use that function excpet if you really really know what your are doing.
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
