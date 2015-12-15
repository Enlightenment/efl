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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_private.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_safety_checks.h"

EAPI Eina_Error EINA_ERROR_SAFETY_FAILED = 0;

static int EINA_SAFETY_LOG_DOMAIN = 0;
static int initcnt = 0;

/**
 * Log entry-point called every time an eina safety check fails.
 *
 * One purpose of this dedicated function is to provide a convenient breakpoint
 * for GDB debugging. Also, this gives it a dedicated log domain, rather than
 * using the default one.
 *
 * @since 1.17
 * @internal
 */
Eina_Bool
eina_safety_checks_shutdown(void)
{
   if (!initcnt) return EINA_FALSE;
   if (!(--initcnt))
     {
        eina_log_domain_unregister(EINA_SAFETY_LOG_DOMAIN);
        EINA_SAFETY_LOG_DOMAIN = 0;
     }
   return EINA_TRUE;
}

/**
 * @internal
 * @brief Initialize the safety checks module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the safety checks module of Eina. It is
 * called by eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_safety_checks_init(void)
{
   if (!(initcnt++))
     {
        EINA_SAFETY_LOG_DOMAIN = eina_log_domain_register("eina_safety", EINA_COLOR_RED);
        EINA_ERROR_SAFETY_FAILED = eina_error_msg_static_register("Safety check failed");
     }
   return EINA_TRUE;
}

EAPI void
_eina_safety_error(const char *file, const char *func, int line, const char *str)
{
   eina_error_set(EINA_ERROR_SAFETY_FAILED);
   if (EINA_SAFETY_LOG_DOMAIN)
     {
        eina_log_print(EINA_SAFETY_LOG_DOMAIN, EINA_LOG_LEVEL_ERR,
                       file, func, line, "%s", str);
     }
   else
     {
        eina_log_print(EINA_LOG_DOMAIN_DEFAULT, EINA_LOG_LEVEL_ERR,
                       file, func, line, "%s", str);
     }
}
