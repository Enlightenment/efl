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

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_error.h"
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
   char *string;
};

static int _eina_magic_string_log_dom = -1;

#define ERR(...) EINA_LOG_DOM_ERR(_eina_magic_string_log_dom, __VA_ARGS__)
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
   Eina_Magic a = (long)p1;
   const Eina_Magic_String *b = p2;
   return a - b->magic;
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
 * @addtogroup Eina_Magic_Group Magic
 *
 * @brief These functions provide magic checks management for projects.
 *
 * @{
 */

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
   size_t i;

   for (i = 0; i < _eina_magic_strings_count; i++)
     free(_eina_magic_strings[i].string);

   free(_eina_magic_strings);
   _eina_magic_strings = NULL;
   _eina_magic_strings_count = 0;
   _eina_magic_strings_allocated = 0;

   eina_log_domain_unregister(_eina_magic_string_log_dom);
   _eina_magic_string_log_dom = -1;

   return EINA_TRUE;
}

/**
 * @brief Return the string associated to the given magic identifier.
 *
 * @param magic The magic identifier.
 * @return The string associated to the identifier.
 *
 * This function returns the string associated to @p magic. If none
 * are found, the this function returns @c NULL. The returned value
 * must not be freed.
 */
EAPI const char*
eina_magic_string_get(Eina_Magic magic)
{
   Eina_Magic_String *ems;

   if (!_eina_magic_strings)
     return NULL;

   if (_eina_magic_strings_dirty)
     {
	qsort(_eina_magic_strings, _eina_magic_strings_count,
	      sizeof(Eina_Magic_String), _eina_magic_strings_sort_cmp);
	_eina_magic_strings_dirty = 0;
     }

   ems = bsearch((void *)(long)magic, _eina_magic_strings,
		 _eina_magic_strings_count, sizeof(Eina_Magic_String),
		 _eina_magic_strings_find_cmp);
   if (ems)
     return ems->string;
   return NULL;
}

/**
 * @brief Set the string associated to the given magic identifier.
 *
 * @param magic The magic identifier.
 * @param The string associated to the identifier, must not be @c NULL.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets the string @p magic_name to @p magic. It is not
 * checked if number or string are already set, then you might end
 * with duplicates in that case.
 */
EAPI Eina_Bool
eina_magic_string_set(Eina_Magic magic, const char *magic_name)
{
   Eina_Magic_String *ems;

   EINA_SAFETY_ON_NULL_RETURN_VAL(magic_name, EINA_FALSE);

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
	     return EINA_FALSE;
	  }
	_eina_magic_strings = tmp;
	_eina_magic_strings_allocated = size;
     }

   ems = _eina_magic_strings + _eina_magic_strings_count;
   ems->magic = magic;
   ems->string = strdup(magic_name);
   if (!ems->string)
     {
	ERR("could not allocate string '%s'", magic_name);
	return EINA_FALSE;
     }

   _eina_magic_strings_count++;
   _eina_magic_strings_dirty = 1;
   return EINA_TRUE;
}

#ifdef eina_magic_fail
# undef eina_magic_fail
#endif

/**
 * @brief Display a message or abort is a magic check failed.
 *
 * @param d The checked data pointer.
 * @param m The magic identifer to check.
 * @param req_m The requested magic identifier to check.
 * @param file The file in which the magic check failed.
 * @param fcn The function in which the magic check failed.
 * @param line The line at which the magic check failed.
 *
 * This function displays an error message if a magic check has
 * failed, using the following logic in the following order:
 * @li If @p d is @c NULL, a message warns about a @c NULL pointer.
 * @li Otherwise, if @p m is equal to #EINA_MAGIC_NONE, a message
 * warns about a handle that was already freed.
 * @li Otherwise, if @p m is equal to @p req_m, a message warns about
 * a handle that is of wrong type.
 * @li Otherwise, a message warns you about ab-using that function...
 *
 * If the environment variable EINA_ERROR_ABORT is set, abort() is
 * called and the program stops. It is useful for debugging programs
 * with gdb.
 */
EAPI void
eina_magic_fail(void *d, Eina_Magic m, Eina_Magic req_m, const char *file, const char *fnc, int line)
{
   if (!d)
     eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL,
		    file, fnc, line,
		    "*** Eina Magic Check Failed !!!\n"
		    "    Input handle pointer is NULL !\n"
		    "*** NAUGHTY PROGRAMMER!!!\n"
		    "*** SPANK SPANK SPANK!!!\n"
		    "*** Now go fix your code. Tut tut tut!\n"
		    "\n");
   else
     if (m == EINA_MAGIC_NONE)
       eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL,
		      file, fnc, line,
		      "*** Eina Magic Check Failed !!!\n"
		      "    Input handle has already been freed!\n"
		      "*** NAUGHTY PROGRAMMER!!!\n"
		      "*** SPANK SPANK SPANK!!!\n"
		      "*** Now go fix your code. Tut tut tut!\n"
		      "\n");
     else
       if (m != req_m)
       eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL,
		      file, fnc, line,
		      "*** Eina Magic Check Failed !!!\n"
		      "    Input handle is wrong type\n"
		      "    Expected: %08x - %s\n"
		      "    Supplied: %08x - %s\n"
		      "*** NAUGHTY PROGRAMMER!!!\n"
		      "*** SPANK SPANK SPANK!!!\n"
		      "*** Now go fix your code. Tut tut tut!\n"
		      "\n",
		      req_m, eina_magic_string_get(req_m),
		      m, eina_magic_string_get(m));
       else
       eina_log_print(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL,
		      file, fnc, line,
		      "*** Eina Magic Check Failed !!!\n"
		      "    Why did you call me !\n"
		      "*** NAUGHTY PROGRAMMER!!!\n"
		      "*** SPANK SPANK SPANK!!!\n"
		      "*** Now go fix your code. Tut tut tut!\n"
		      "\n");
}

/**
 * @}
 */
