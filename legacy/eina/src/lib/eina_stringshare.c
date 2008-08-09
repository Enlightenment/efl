/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Jorge Luis Zapata Muga, Cedric Bail
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (C) 2008 Peter Wehrfritz
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies of the Software and its Copyright notices. In addition publicly
 *  documented acknowledgment must be given that this software has been used if no
 *  source code of this software is made available publicly. This includes
 *  acknowledgments in either Copyright notices, Manuals, Publicity and Marketing
 *  documents or any documentation provided with any product containing this
 *  software. This License does not apply to any software that links to the
 *  libraries provided by this software (statically or dynamically), but only to
 *  the software provided.
 *
 *  Please see the OLD-COPYING.PLAIN for a plain-english explanation of this notice
 *  and it's intent.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eina_stringshare.h"
#include "eina_hash.h"
#include "eina_error.h"
#include "eina_private.h"

typedef struct _Eina_Stringshare             Eina_Stringshare;
typedef struct _Eina_Stringshare_Node        Eina_Stringshare_Node;

struct _Eina_Stringshare
{
   Eina_Stringshare_Node *buckets[1024];
};

struct _Eina_Stringshare_Node
{
   Eina_Stringshare_Node *next;
   int                  references;
};

static Eina_Stringshare *share = NULL;
static int eina_stringshare_init_count = 0;

/**
 * @defgroup Eina_Stringshare_Group String Instance Functions
 *
 * These functions allow you to store one copy of a string, and use it
 * throughout your program.
 *
 * This is a method to reduce the number of duplicated strings kept in
 * memory. It's pretty common for the same strings to be dynamically
 * allocated repeatedly between applications and libraries, especially in
 * circumstances where you could have multiple copies of a structure that
 * allocates the string. So rather than duplicating and freeing these
 * strings, you request a read-only pointer to an existing string and
 * only incur the overhead of a hash lookup.
 *
 * It sounds like micro-optimizing, but profiling has shown this can have
 * a significant impact as you scale the number of copies up. It improves
 * string creation/destruction speed, reduces memory use and decreases
 * memory fragmentation, so a win all-around.
 */

/**
 * Initialize the eina stringshare internal structure.
 * @return  Zero on failure, non-zero on successful initialization.
 */
EAPI int
eina_stringshare_init()
{
   /*
    * No strings have been loaded at this point, so create the hash
    * table for storing string info for later.
    */
   if (!eina_stringshare_init_count)
     {
	share = calloc(1, sizeof(Eina_Stringshare));
	if (!share)
	  return 0;
	eina_error_init();
     }
   eina_stringshare_init_count++;

   return 1;
}

/**
 * Retrieves an instance of a string for use in a program.
 * @param   str The string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 * @ingroup Eina_Stringshare_Group
 */
EAPI const char *
eina_stringshare_add(const char *str)
{
   int hash_num, slen;
   char *el_str;
   Eina_Stringshare_Node *el, *pel = NULL;

   if (!str) return NULL;
   slen = strlen(str) + 1;
   hash_num = eina_hash_superfast(str, slen) & 0x3FF;
   for (el = share->buckets[hash_num]; el; pel = el, el = el->next)
     {
	el_str = ((char *)el) + sizeof(Eina_Stringshare_Node);
	if (!strcmp(el_str, str))
	  {
	     if (pel)
	       {
		  pel->next = el->next;
		  el->next = share->buckets[hash_num];
		  share->buckets[hash_num] = el;
	       }
	     el->references++;
	     return el_str;
	  }
     }
   if (!(el = malloc(sizeof(Eina_Stringshare_Node) + slen + 1))) return NULL;
   el_str = ((char *)el) + sizeof(Eina_Stringshare_Node);
   strcpy(el_str, str);
   el->references = 1;
   el->next = share->buckets[hash_num];
   share->buckets[hash_num] = el;
   return el_str;
}

/**
 * Notes that the given string has lost an instance.
 *
 * It will free the string if no other instances are left.
 *
 * @param str string The given string.
 * @ingroup Eina_Stringshare_Group
 */
EAPI void
eina_stringshare_del(const char *str)
{
   int hash_num, slen;
   char *el_str;
   Eina_Stringshare_Node *el, *pel = NULL;

   if (!str) return;
   slen = strlen(str) + 1;
   hash_num = eina_hash_superfast(str, slen) & 0x3FF;
   for (el = share->buckets[hash_num]; el; pel = el, el = el->next)
     {
	el_str = ((char *)el) + sizeof(Eina_Stringshare_Node);
	if (el_str == str)
	  {
	     el->references--;
	     if (el->references == 0)
	       {
		  if (pel) pel->next = el->next;
		  else share->buckets[hash_num] = el->next;
		  free(el);
	       }
	     else
	       {
		  if (pel)
		    {
		       pel->next = el->next;
		       el->next = share->buckets[hash_num];
		       share->buckets[hash_num] = el;
		    }
	       }
	     return;
	  }
     }
   EINA_ERROR_PWARN("EEEK trying to del non-shared stringshare \"%s\"\n", str);
   if (getenv("EINA_ERROR_ABORT")) abort();
}

/**
 * Shutdown the eina string internal structures
 */
EAPI int
eina_stringshare_shutdown()
{
   --eina_stringshare_init_count;
   if (!eina_stringshare_init_count)
     {
	int i;
	/* remove any string still in the table */
	for (i = 0; i < 1024; i++)
	  {
	     Eina_Stringshare_Node *el = share->buckets[i];
	     while (el)
	       {
		  Eina_Stringshare_Node *cur = el;
		  el = el->next;
		  cur->next = NULL;
		  free(cur);
	       }
	     share->buckets[i] = NULL;
	  }
	free(share);
	share = NULL;

	eina_error_shutdown();
     }

   return eina_stringshare_init_count;
}

