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
#include "eina_private.h"
#include "eina_types.h"
#include "eina_main.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_hash.h"
#include "eina_stringshare.h"
#include "eina_list.h"
#include "eina_matrixsparse.h"
#include "eina_array.h"
#include "eina_counter.h"
#include "eina_benchmark.h"
#include "eina_magic.h"
#include "eina_rectangle.h"
#include "eina_safety_checks.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static int _eina_main_count = 0;
static int _eina_log_dom = -1;
#define ERR(...) EINA_LOG_DOM_ERR(_eina_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_eina_log_dom, __VA_ARGS__)


/* place module init/shutdown functions here to avoid other modules
 * calling them by mistake.
 */
#define S(x) extern Eina_Bool eina_##x##_init(void); extern Eina_Bool eina_##x##_shutdown(void)
S(log);
S(error);
S(safety_checks);
S(magic_string);
S(array);
S(module);
S(mempool);
S(list);
S(stringshare);
S(matrixsparse);
S(convert);
S(counter);
S(benchmark);
S(rectangle);
#undef S

struct eina_desc_setup
{
   const char *name;
   Eina_Bool (*init)(void);
   Eina_Bool (*shutdown)(void);
};

static const struct eina_desc_setup _eina_desc_setup[] = {
#define S(x) {#x, eina_##x##_init, eina_##x##_shutdown}
  /* log is a special case as it needs printf */
  S(error),
  S(safety_checks),
  S(magic_string),
  S(array),
  S(module),
  S(mempool),
  S(list),
  S(stringshare),
  S(matrixsparse),
  S(convert),
  S(counter),
  S(benchmark),
  S(rectangle)
#undef S
};
static const size_t _eina_desc_setup_len = sizeof(_eina_desc_setup) / sizeof(_eina_desc_setup[0]);

static void
_eina_shutdown_from_desc(const struct eina_desc_setup *itr)
{
   for (itr--; itr >= _eina_desc_setup; itr--)
     {
	if (!itr->shutdown())
	  ERR("Problems shutting down eina module '%s', ignored.", itr->name);
     }

   eina_log_domain_unregister(_eina_log_dom);
   _eina_log_dom = -1;
   eina_log_shutdown();
}

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
 * called.
 *
 * When Eina is not used anymore, call eina_shutdown() to shut down
 * the Eina library.
 */
EAPI int
eina_init(void)
{
   const struct eina_desc_setup *itr, *itr_end;

   if (EINA_LIKELY(_eina_main_count > 0))
     return ++_eina_main_count;

   if (!eina_log_init())
     {
	fprintf(stderr, "Could not initialize eina logging system.\n");
	return 0;
     }
   _eina_log_dom = eina_log_domain_register("eina", EINA_LOG_COLOR_DEFAULT);
   if (_eina_log_dom < 0)
     {
	EINA_LOG_ERR("Could not register log domain: eina");
	eina_log_shutdown();
	return 0;
     }

   itr = _eina_desc_setup;
   itr_end = itr + _eina_desc_setup_len;
   for (; itr < itr_end; itr++)
     {
	if (!itr->init())
	  {
	     ERR("Could not initialize eina module '%s'.", itr->name);
	     _eina_shutdown_from_desc(itr);
	     return 0;
	  }
     }

   _eina_main_count = 1;
   return 1;
}

/**
 * @brief Shut down the Eina library.
 *
 * @return 0 when all the modules is completely shut down, 1 or
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
EAPI int
eina_shutdown(void)
{
   _eina_main_count--;
   if (EINA_UNLIKELY(_eina_main_count == 0))
     _eina_shutdown_from_desc(_eina_desc_setup + _eina_desc_setup_len);
   return _eina_main_count;
}

/**
 * @}
 */
