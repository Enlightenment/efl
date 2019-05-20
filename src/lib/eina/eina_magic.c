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

#include <stdlib.h>
#include <string.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_magic.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eina_Magic_String Eina_Magic_String;
struct _Eina_Magic_String
{
   Eina_Magic magic;
   Eina_Bool string_allocated;
   const char *string;
};

static int _eina_magic_string_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_magic_string_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_magic_string_log_dom, __VA_ARGS__)

static Eina_Magic_String *_eina_magic_strings = NULL;
static size_t _eina_magic_strings_count = 0;
static size_t _eina_magic_strings_allocated = 0;
static Eina_Bool _eina_magic_strings_dirty = 0;

static int
_eina_magic_strings_sort_cmp(const void *p1, const void *p2)
{
   const Eina_Magic_String *a = p1, *b = p2;
   return a->magic - b->magic;
}

static int
_eina_magic_strings_find_cmp(const void *p1, const void *p2)
{
   Eina_Magic a = (Eina_Magic)(size_t)p1;
   const Eina_Magic_String *b = p2;
   return a - b->magic;
}

static Eina_Magic_String *
_eina_magic_strings_alloc(void)
{
   size_t idx;

   if (_eina_magic_strings_count == _eina_magic_strings_allocated)
     {
        void *tmp;
        size_t size;

        if (EINA_UNLIKELY(_eina_magic_strings_allocated == 0))
           size = 48;
        else
           size = _eina_magic_strings_allocated + 16;

        tmp = realloc(_eina_magic_strings, sizeof(Eina_Magic_String) * size);
        if (!tmp)
          {
             ERR("could not realloc magic_strings from %zu to %zu buckets.",
                 _eina_magic_strings_allocated, size);
             return NULL;
          }

        _eina_magic_strings = tmp;
        _eina_magic_strings_allocated = size;
     }

   idx = _eina_magic_strings_count;
   _eina_magic_strings_count++;
   return _eina_magic_strings + idx;
}

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

EAPI Eina_Error EINA_ERROR_MAGIC_FAILED = 0;

/**
 * @internal
 * @brief Initialize the magic string module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the magic string module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_magic_string_init(void)
{
   _eina_magic_string_log_dom = eina_log_domain_register
         ("eina_magic_string", EINA_LOG_COLOR_DEFAULT);
   if (_eina_magic_string_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_magic_string");
        return EINA_FALSE;
     }

   EINA_ERROR_MAGIC_FAILED = eina_error_msg_static_register("Magic check failed.");

   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the magic string module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the magic string module set up by
 * eina_magic string_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_magic_string_shutdown(void)
{
   Eina_Magic_String *ems, *ems_end;

   ems = _eina_magic_strings;
   ems_end = ems + _eina_magic_strings_count;

   for (; ems < ems_end; ems++)
      if (ems->string_allocated)
         free((char *)ems->string);

   free(_eina_magic_strings);
   _eina_magic_strings = NULL;
   _eina_magic_strings_count = 0;
   _eina_magic_strings_allocated = 0;

   eina_log_domain_unregister(_eina_magic_string_log_dom);
   _eina_magic_string_log_dom = -1;

   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/
EAPI const char *
eina_magic_string_get(Eina_Magic magic)
{
   Eina_Magic_String *ems;

   if (!_eina_magic_strings)
      return "(none)";

   if (_eina_magic_strings_dirty)
     {
        qsort(_eina_magic_strings, _eina_magic_strings_count,
              sizeof(Eina_Magic_String), _eina_magic_strings_sort_cmp);
        _eina_magic_strings_dirty = 0;
     }

   ems = bsearch((void *)(size_t)magic, _eina_magic_strings,
                 _eina_magic_strings_count, sizeof(Eina_Magic_String),
                 _eina_magic_strings_find_cmp);
   if (ems)
      return ems->string ? ems->string : "(undefined)";

   return "(unknown)";
}

EAPI Eina_Bool
eina_magic_string_set(Eina_Magic magic, const char *magic_name)
{
   Eina_Magic_String *ems;

   EINA_SAFETY_ON_NULL_RETURN_VAL(magic_name, EINA_FALSE);

   ems = _eina_magic_strings_alloc();
   if (!ems)
      return EINA_FALSE;

   ems->magic = magic;
   ems->string_allocated = EINA_TRUE;
   ems->string = strdup(magic_name);
   if (!ems->string)
   {
      ERR("could not allocate string '%s'", magic_name);
      _eina_magic_strings_count--;
      return EINA_FALSE;
   }

   _eina_magic_strings_dirty = 1;
   return EINA_TRUE;
}

EAPI Eina_Bool
eina_magic_string_static_set(Eina_Magic magic, const char *magic_name)
{
   Eina_Magic_String *ems;

   EINA_SAFETY_ON_NULL_RETURN_VAL(magic_name, EINA_FALSE);

   ems = _eina_magic_strings_alloc();
   if (!ems)
      return EINA_FALSE;

   ems->magic = magic;
   ems->string_allocated = EINA_FALSE;
   ems->string = magic_name;

   _eina_magic_strings_dirty = 1;
   return EINA_TRUE;
}

#ifdef eina_magic_fail
# undef eina_magic_fail
#endif

EAPI void
eina_magic_fail(void *d,
                Eina_Magic m,
                Eina_Magic req_m,
                const char *file,
                const char *fnc,
                int line)
{
   if (!d)
      eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_ERR,
                     file, fnc, line,
                     "*** Eina Magic Check Failed !!!\n"
                     "    Input handle pointer is NULL.\n"
                     "\n");
   else
   if (m == EINA_MAGIC_NONE)
      eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL,
                     file, fnc, line,
                     "*** Eina Magic Check Failed at %p !!!\n"
                     "    Input handle has already been freed.\n"
                     "\n", d);
   else
   if (m != req_m)
      eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL,
                     file, fnc, line,
                     "*** Eina Magic Check Failed at %p !!!\n"
                     "    Input handle is wrong type.\n"
                     "    Expected: %08x - %s\n"
                     "    Supplied: %08x - %s\n"
                     "\n",
                     d, req_m, eina_magic_string_get(req_m),
                     m, eina_magic_string_get(m));
   else
      eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL,
                     file, fnc, line,
                     "*** Eina Magic Check Failed !!!\n"
                     "    Unknown reason.\n"
                     "\n");
}

/**
 * @}
 */
