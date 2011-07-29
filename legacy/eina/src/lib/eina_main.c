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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#ifdef EFL_HAVE_WIN32_THREADS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif

#include "eina_lock.h"
#include "eina_config.h"
#include "eina_private.h"
#include "eina_types.h"
#include "eina_main.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_hash.h"
#include "eina_binshare.h"
#include "eina_stringshare.h"
#include "eina_ustringshare.h"
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

static Eina_Version _version = { VMAJ, VMIN, VMIC, VREV };

static int _eina_main_count = 0;
#ifdef EFL_HAVE_THREADS
static int _eina_main_thread_count = 0;
#endif
static int _eina_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_log_dom, __VA_ARGS__)

EAPI Eina_Bool _eina_threads_activated = EINA_FALSE;

#ifdef EINA_HAVE_DEBUG_THREADS
EAPI int _eina_threads_debug = 0;
EAPI pthread_t _eina_main_loop;;
EAPI pthread_mutex_t _eina_tracking_lock;
EAPI Eina_Inlist *_eina_tracking = NULL;
#endif

/* place module init/shutdown functions here to avoid other modules
 * calling them by mistake.
 */
#define S(x) extern Eina_Bool eina_ ## x ## _init(void); \
   extern Eina_Bool eina_ ## x ## _shutdown(void)
   S(log);
   S(error);
   S(safety_checks);
   S(magic_string);
   S(iterator);
   S(accessor);
   S(array);
   S(module);
   S(mempool);
   S(list);
   S(binshare);
   S(stringshare);
   S(ustringshare);
   S(matrixsparse);
   S(convert);
   S(counter);
   S(benchmark);
   S(rectangle);
   S(strbuf);
   S(ustrbuf);
   S(quadtree);
   S(simple_xml);
   S(file);
#undef S

struct eina_desc_setup
{
   const char *name;
   Eina_Bool (*init)(void);
   Eina_Bool (*shutdown)(void);
};

static const struct eina_desc_setup _eina_desc_setup[] = {
#define S(x) {# x, eina_ ## x ## _init, eina_ ## x ## _shutdown}
   /* log is a special case as it needs printf */
   S(error),
   S(safety_checks),
   S(magic_string),
   S(iterator),
   S(accessor),
   S(array),
   S(module),
   S(mempool),
   S(list),
   S(binshare),
   S(stringshare),
   S(ustringshare),
   S(matrixsparse),
   S(convert),
   S(counter),
   S(benchmark),
   S(rectangle),
   S(strbuf),
   S(ustrbuf),
   S(quadtree),
   S(simple_xml),
   S(file)
#undef S
};
static const size_t _eina_desc_setup_len = sizeof(_eina_desc_setup) /
   sizeof(_eina_desc_setup[0]);

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
 * @var eina_version
 * @brief Eina version (defined at configuration time)
 */
EAPI Eina_Version *eina_version = &_version;

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

#ifdef EINA_HAVE_DEBUG_THREADS
   _eina_main_loop = pthread_self();
   pthread_mutex_init(&_eina_tracking_lock, NULL);

   if (getenv("EINA_DEBUG_THREADS"))
     _eina_threads_debug = atoi(getenv("EINA_DEBUG_THREADS"));
#endif

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

EAPI int
eina_shutdown(void)
{
   _eina_main_count--;
   if (EINA_UNLIKELY(_eina_main_count == 0))
     {
        _eina_shutdown_from_desc(_eina_desc_setup + _eina_desc_setup_len);

#ifdef EINA_HAVE_DEBUG_THREADS
	pthread_mutex_destroy(&_eina_tracking_lock);
#endif
     }

   return _eina_main_count;
}


EAPI int
eina_threads_init(void)
{
#ifdef EFL_HAVE_THREADS
   int ret;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   ++_eina_main_thread_count;
   ret = _eina_main_thread_count;

   if(_eina_main_thread_count > 1)
     return ret;

   eina_share_common_threads_init();
   eina_log_threads_init();
   _eina_threads_activated = EINA_TRUE;

   return ret;
#else
   return 0;
#endif
}

EAPI int
eina_threads_shutdown(void)
{
#ifdef EFL_HAVE_THREADS
   int ret;

#ifdef EINA_HAVE_DEBUG_THREADS
   const Eina_Lock *lk;

   assert(pthread_equal(_eina_main_loop, pthread_self()));
   assert(_eina_main_thread_count > 0);
#endif

   ret = --_eina_main_thread_count;
   if(_eina_main_thread_count > 0)
     return ret;

#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_mutex_lock(&_eina_tracking_lock);
   if (_eina_tracking)
     {
       fprintf(stderr, "*************************\n");
       fprintf(stderr, "* The IMPOSSIBLE HAPPEN *\n");
       fprintf(stderr, "* LOCK STILL TAKEN :    *\n");
       fprintf(stderr, "*************************\n");
       EINA_INLIST_FOREACH(_eina_tracking, lk)
	 eina_lock_debug(lk);
       fprintf(stderr, "*************************\n");
       abort();
     }
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   eina_share_common_threads_shutdown();
   eina_log_threads_shutdown();

   _eina_threads_activated = EINA_FALSE;

   return ret;
#else
   return 0;
#endif
}

EAPI Eina_Bool
eina_main_loop_is(void)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   if (pthread_equal(_eina_main_loop, pthread_self()))
     return EINA_TRUE;
   return EINA_FALSE;
#else
   /* FIXME: need to check how to do this on windows */
   return EINA_TRUE;
#endif
}

/**
 * @}
 */
