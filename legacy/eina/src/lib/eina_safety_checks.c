/* EINA - EFL data type library
 * Copyright (C) 2008 Gustavo Sverzut Barbieri
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

/**
 * @addtogroup Eina_Safety_Checks_Group Safety Checks
 *
 * Safety checks are a set of macros to check for parameters or values
 * that should never happen, it is similar in concept to assert(), but
 * will log and return instead of abort() your program.
 *
 * Since these cases should never happen, one may wantto keep safety
 * checks enabled during tests but disable then during deploy, not
 * doing any checks at all. This is a common requirement for embedded
 * systems. Whenever to check or not should be set during compile time
 * by using @c --disable-safety-checks or @c --enable-safety-checks
 * options to @c configure script.
 *
 * Whenever these macros capture an error, EINA_LOG_ERR() will be
 * called and @c eina_error set to @c EINA_ERROR_SAFETY_FAILED and can
 * be checked with eina_error_get() after call.
 *
 * @see EINA_SAFETY_ON_NULL_RETURN(), EINA_SAFETY_ON_NULL_RETURN_VAL()
 *      and other macros.
 *
 * @{
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_error.h"
#include "eina_private.h"
#include "eina_safety_checks.h"

/**
 * @cond LOCAL
 */

EAPI Eina_Error EINA_ERROR_SAFETY_FAILED = 0;

static int _eina_safety_checks_init_count = 0;

/**
 * @endcond
 */

/**
 * @brief Initialize the safety checks module.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up the safety checks module of Eina. It is
 * called by eina_init() and by all modules initialization
 * functions. It returns @c 0 on failure, otherwise it returns the
 * number of times it is called.
 *
 * @see eina_init()
 */
EAPI int
eina_safety_checks_init(void)
{
   if (_eina_safety_checks_init_count > 0)
     return ++_eina_safety_checks_init_count;

#ifdef EINA_SAFETY_CHECKS
   if (!eina_log_init())
     return 0;
#endif
   if (!eina_error_init())
     {
	eina_log_shutdown();
	return 0;
     }
   EINA_ERROR_SAFETY_FAILED = eina_error_msg_register("Safety check failed.");
   _eina_safety_checks_init_count = 1;
   return 1;
}

/**
 * @brief Shut down the safety checks module.
 *
 * @return 0 when the error module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the error module set up by
 * eina_safety_checks_init(). It is called by eina_shutdown() and by
 * all modules shutdown functions. It returns 0 when it is called the
 * same number of times than eina_safety_checks_init(). In that case
 * it clears the error list.
 *
 * @see eina_shutdown()
 */
EAPI int
eina_safety_checks_shutdown(void)
{
   _eina_safety_checks_init_count--;
   if (_eina_safety_checks_init_count != 0)
     return _eina_safety_checks_init_count;

#ifdef EINA_SAFETY_CHECKS
   eina_error_shutdown();
   eina_log_shutdown();
#endif
   return 0;
}

/**
 * @}
 */
