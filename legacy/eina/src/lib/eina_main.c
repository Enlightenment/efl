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

#include <stdio.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_config.h"
#include "eina_types.h"
#include "eina_main.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_hash.h"
#include "eina_stringshare.h"
#include "eina_list.h"
#include "eina_array.h"
#include "eina_counter.h"
#include "eina_benchmark.h"
#include "eina_magic.h"
#include "eina_rectangle.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static int _eina_main_count = 0;

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Main_Group Main
 *
 * @brief These functions provide general initialisation and shut down
 * functions.
 *
 * @{
 */

/**
 * @brief Initialize the Eina library.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up all the eina modules. It returns 0 on
 * failure (that is, when one of the module fails to initialize),
 * otherwise it returns the number of times it has already been
 * called. The list of initialisation functions that are called are
 * (in that order):
 *
 * @li eina_error_init()
 * @li eina_hash_init()
 * @li eina_stringshare_init()
 * @li eina_list_init()
 * @li eina_array_init()
 * @li eina_counter_init()
 * @li eina_benchmark_init()
 * @li eina_magic_string_init()
 * @li eina_rectangle_init()
 *
 * When Eina is not used anymore, call eina_shutdown() to shut down
 * the Eina library.
 */
EAPI int
eina_init(void)
{
   if (_eina_main_count) goto finish_init;

   if (!eina_error_init())
     {
        fprintf(stderr, "Could not initialize eina error module.\n");
        return 0;
     }

   if (!eina_log_init())
     {
	fprintf(stderr, "Could not initialize eina log module.\n");
	goto log_init_error;
     }

   if (!eina_hash_init())
     {
        EINA_ERROR_PERR("Could not initialize eina hash module.\n");
        goto hash_init_error;
     }
   if (!eina_stringshare_init())
     {
        EINA_ERROR_PERR("Could not initialize eina stringshare module.\n");
        goto stringshare_init_error;
     }
   if (!eina_list_init())
     {
        EINA_ERROR_PERR("Could not initialize eina list module.\n");
        goto list_init_error;
     }
   if (!eina_array_init())
     {
        EINA_ERROR_PERR("Could not initialize eina array module.\n");
        goto array_init_error;
     }
   if (!eina_counter_init())
     {
        EINA_ERROR_PERR("Could not initialize eina counter module.\n");
        goto counter_init_error;
     }
   if (!eina_benchmark_init())
     {
        EINA_ERROR_PERR("Could not initialize eina benchmark module.\n");
        goto benchmark_init_error;
     }
   if (!eina_magic_string_init())
     {
        EINA_ERROR_PERR("Could not initialize eina magic string module.\n");
        goto magic_string_init_error;
     }
   if (!eina_rectangle_init())
     {
        EINA_ERROR_PERR("Could not initialize eina rectangle module.\n");
        goto rectangle_init_error;
     }

 finish_init:
   return ++_eina_main_count;

 rectangle_init_error:
   eina_magic_string_shutdown();
 magic_string_init_error:
   eina_benchmark_shutdown();
 benchmark_init_error:
   eina_counter_shutdown();
 counter_init_error:
   eina_array_shutdown();
 array_init_error:
   eina_list_shutdown();
 list_init_error:
   eina_stringshare_shutdown();
 stringshare_init_error:
   eina_hash_shutdown();
 hash_init_error:
   eina_log_shutdown();
 log_init_error:
   eina_error_shutdown();

   return 0;
}

/**
 * @brief Shut down the Eina library.
 *
 * @return 0 when all the modules is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Eina library. It returns 0 when it has
 * been called the same number of times than eina_init(). In that case
 * it shut down all the Eina modules. The list of shut down functions
 * that are called are (in that order):
 *
 * @li eina_rectangle_init()
 * @li eina_magic_string_init()
 * @li eina_benchmark_init()
 * @li eina_counter_init()
 * @li eina_array_init()
 * @li eina_list_init()
 * @li eina_stringshare_init()
 * @li eina_hash_init()
 * @li eina_error_init()
 *
 * Once this function succeeds (that is, @c 0 is returned), you must
 * not call any of the Eina function anymore. You must call
 * eina_init() again to use the Eina functions again.
 */
EAPI int
eina_shutdown(void)
{
   if (_eina_main_count != 1) goto finish_shutdown;

   eina_rectangle_shutdown();
   eina_magic_string_shutdown();
   eina_benchmark_shutdown();
   eina_counter_shutdown();
   eina_array_shutdown();
   eina_list_shutdown();
   eina_stringshare_shutdown();
   eina_hash_shutdown();
   eina_log_shutdown();
   eina_error_shutdown();

 finish_shutdown:
   return --_eina_main_count;
}

/**
 * @}
 */
