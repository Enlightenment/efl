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
#include "eina_inlist.h"
#include "eina_magic.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifdef EINA_MAGIC_DEBUG

typedef struct _Eina_Magic_String Eina_Magic_String;
struct _Eina_Magic_String
{
   EINA_INLIST;

   char *string;
   Eina_Magic magic;
};

static int _eina_magic_string_count = 0;
static Eina_Inlist *strings = NULL;

#endif

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
 * @brief Initialize the magic module.
 *
 * @return 1 or greater.
 *
 * This function just increases a reference counter. If the magic
 * module is disabled at configure time, then it always returns @c 1.
 *
 * Once the magic module is not used anymore, then
 * eina_magic_shutdown() must be called to shut down the magic
 * module.
 *
 * @see eina_init()
 */
EAPI int
eina_magic_string_init(void)
{
#ifdef EINA_MAGIC_DEBUG
   ++_eina_magic_string_count;

   return _eina_magic_string_count;
#else
   return 1;
#endif
}

/**
 * @brief Shut down the magic module.
 *
 * @return 0 when the magic module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the magic module set up by
 * eina_magic_string_init(). It is called by eina_shutdown() and by
 * all modules shutdown functions. It returns 0 when it is called the
 * same number of times than eina_magic_string_init(). In that case it
 * clears the magic list and return @c 0. If the magic module is
 * disabled at configure time, then it always returns @c 0.
 *
 * @see eina_shutdown()
 */
EAPI int
eina_magic_string_shutdown(void)
{
#ifdef EINA_MAGIC_DEBUG
   --_eina_magic_string_count;

   if (_eina_magic_string_count == 0)
     {
	/* Free all strings. */
	while (strings)
	  {
	     Eina_Magic_String *tmp;

	     tmp = (Eina_Magic_String*) strings;
	     strings = eina_inlist_remove(strings, strings);

	     free(tmp->string);
	     free(tmp);
	  }
     }

   return _eina_magic_string_count;
#else
   return 0;
#endif
}

#ifdef EINA_MAGIC_DEBUG

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

   EINA_INLIST_FOREACH(strings, ems)
     if (ems->magic == magic)
       return ems->string;

   return NULL;
}

/**
 * @brief Set the string associated to the given magic identifier.
 *
 * @param magic The magic identifier.
 * @param The string associated to the identifier.
 *
 * This function sets the string @p magic_name to @p magic. If a
 * string is already associated to @p magic, then it is freed and @p
 * magic_name is duplicated. Otherwise, it is added to the list of
 * magic strings.
 */
EAPI void
eina_magic_string_set(Eina_Magic magic, const char *magic_name)
{
   Eina_Magic_String *ems;

   EINA_INLIST_FOREACH(strings, ems)
     if (ems->magic == magic)
       {
	  free(ems->string);
	  if (magic_name)
	    ems->string = strdup(magic_name);
	  else
	    ems->string = NULL;
	  return ;
       }

   ems = malloc(sizeof (Eina_Magic_String));
   if (!ems)
     return;
   ems->magic = magic;
   if (magic_name)
     ems->string = strdup(magic_name);
   else
     ems->string = NULL;

   strings = eina_inlist_prepend(strings, EINA_INLIST_GET(ems));
}

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
     eina_error_print(EINA_ERROR_LEVEL_ERR, file, fnc, line,
		      "*** Eina Magic Check Failed !!!\n"
		      "    Input handle pointer is NULL !\n"
		      "*** NAUGHTY PROGRAMMER!!!\n"
	              "*** SPANK SPANK SPANK!!!\n"
	              "*** Now go fix your code. Tut tut tut!\n"
		      "\n");
   else
     if (m == EINA_MAGIC_NONE)
       eina_error_print(EINA_ERROR_LEVEL_ERR, file, fnc, line,
			"*** Eina Magic Check Failed !!!\n"
			"    Input handle has already been freed!\n"
			"*** NAUGHTY PROGRAMMER!!!\n"
			"*** SPANK SPANK SPANK!!!\n"
			"*** Now go fix your code. Tut tut tut!\n"
			"\n");
     else
       if (m != req_m)
	 eina_error_print(EINA_ERROR_LEVEL_ERR, file, fnc, line,
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
	 eina_error_print(EINA_ERROR_LEVEL_ERR, file, fnc, line,
			  "*** Eina Magic Check Failed !!!\n"
			  "    Why did you call me !\n"
			  "*** NAUGHTY PROGRAMMER!!!\n"
			  "*** SPANK SPANK SPANK!!!\n"
			  "*** Now go fix your code. Tut tut tut!\n"
			  "\n");
   if (getenv("EINA_ERROR_ABORT")) abort();
}

#endif

/**
 * @}
 */
