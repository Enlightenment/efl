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

#include "eina_array.h"
#include "eina_error.h"

#include "eina_private.h"

#ifndef DEBUG
#define DEBUG
#endif
#include "eina_magic.h"

typedef struct _Eina_Magic_String Eina_Magic_String;
struct _Eina_Magic_String
{
   char *string;
   Eina_Magic magic;
};

static int _eina_magic_string_count = 0;
static Eina_Array *strings = NULL;

EAPI int
eina_magic_string_init()
{
   ++_eina_magic_string_count;

   if (_eina_magic_string_count == 1)
     strings = eina_array_new(8);

   return _eina_magic_string_count;
}

EAPI int
eina_magic_string_shutdown()
{
   --_eina_magic_string_count;

   if (_eina_magic_string_count == 0)
     {
	eina_array_free(strings);
	strings = NULL;
     }

   return _eina_magic_string_count;
}


EAPI const char*
eina_magic_string_get(Eina_Magic magic)
{
   Eina_Magic_String *ems;
   unsigned int i;

   if (!strings) return NULL;

   EINA_ARRAY_ITER_NEXT(strings, i, ems)
     if (ems->magic == magic)
       return ems->string;

   return NULL;
}

EAPI void
eina_magic_string_set(Eina_Magic magic, const char *magic_name)
{
   Eina_Magic_String *ems;
   unsigned int i;

   if (!strings) return ;

   EINA_ARRAY_ITER_NEXT(strings, i, ems)
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
   ems->magic = magic;
   if (magic_name)
     ems->string = strdup(magic_name);
   else
     ems->string = NULL;

   _eina_array_append(strings, ems);
}

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

