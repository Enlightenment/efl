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


/**
 * @page tutorial_stringshare_page Stringshare Tutorial
 *
 * to be written...
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eina_stringshare.h"
#include "eina_hash.h"
#include "eina_rbtree.h"
#include "eina_error.h"
#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eina_Stringshare             Eina_Stringshare;
typedef struct _Eina_Stringshare_Node        Eina_Stringshare_Node;
typedef struct _Eina_Stringshare_Head        Eina_Stringshare_Head;

struct _Eina_Stringshare
{
   Eina_Stringshare_Head *buckets[256];
};

struct _Eina_Stringshare_Head
{
   Eina_Rbtree            node;
   int                    hash;

   Eina_Stringshare_Node *head;
};

struct _Eina_Stringshare_Node
{
   Eina_Stringshare_Node *next;

   int			  length;
   int                    references;
};

static Eina_Stringshare *share = NULL;
static int _eina_stringshare_init_count = 0;

static int
_eina_stringshare_cmp(const Eina_Stringshare_Head *node, const int *hash, __UNUSED__ int length, __UNUSED__ void *data)
{
   return node->hash - *hash;
}

static Eina_Rbtree_Direction
_eina_stringshare_node(const Eina_Stringshare_Head *left, const Eina_Stringshare_Head *right, __UNUSED__ void *data)
{
   if (left->hash - right->hash < 0)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
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
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Stringshare_Group Stringshare
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
 *
 * For more information, you can look at the @ref tutorial_stringshare_page.
 *
 * @{
 */

/**
 * @brief Initialize the eina stringshare internal structure.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function allocates the memory needed by the stringshare
 * internal structure and sets up the error module of Eina. It is also
 * called by eina_init(). It returns 0 on failure, otherwise it
 * returns the number of times it has already been called.
 */
EAPI int
eina_stringshare_init()
{
   /*
    * No strings have been loaded at this point, so create the hash
    * table for storing string info for later.
    */
   if (!_eina_stringshare_init_count)
     {
	share = calloc(1, sizeof(Eina_Stringshare));
	if (!share)
	  return 0;
	eina_error_init();
     }
   _eina_stringshare_init_count++;

   return _eina_stringshare_init_count;
}

/**
 * @brief Shut down the eina stringshare internal structures
 *
 * @return 0 when the stringshare module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function frees the memory allocated by eina_stringshare_init()
 * and shuts down the error module. It is also called by
 * eina_shutdown(). It returns 0 when it is called the same number of
 * times than eina_stringshare_init().
 */
EAPI int
eina_stringshare_shutdown()
{
   --_eina_stringshare_init_count;
   if (!_eina_stringshare_init_count)
     {
	int i;
	/* remove any string still in the table */
	for (i = 0; i < 256; i++)
	  {
	     Eina_Stringshare_Head *ed = share->buckets[i];
	     Eina_Stringshare_Head *save;

	     while (ed)
	       {
		  save = ed;
		  ed = (Eina_Stringshare_Head*) eina_rbtree_inline_remove(&ed->node, &ed->node,
									  EINA_RBTREE_CMP_NODE_CB(_eina_stringshare_node), NULL);
		  while (save->head)
		    {
		       Eina_Stringshare_Node *el = save->head;

		       save->head = el->next;
		       free(el);
		    }
		  free(save);
	       }
	     share->buckets[i] = NULL;
	  }
	free(share);
	share = NULL;

	eina_error_shutdown();
     }

   return _eina_stringshare_init_count;
}

/**
 * @brief Retrieve an instance of a string for use in a program.
 *
 * @param   str The string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p str. If @p str is
 * @c NULL, then @c NULL is returned. If @p str is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * it is added to the strings to be searched and a duplicated string
 * of @p str is returned.
 */
EAPI const char *
eina_stringshare_add(const char *str)
{
   Eina_Stringshare_Head *ed;
   Eina_Stringshare_Node *el;
   char *el_str;
   int hash_num, slen, hash;

   if (!str) return NULL;
   hash = eina_hash_djb2_len(str, &slen);
   hash_num = hash & 0xFF;
   hash &= 0xFFF;

   ed = (Eina_Stringshare_Head*) eina_rbtree_inline_lookup((Eina_Rbtree*) share->buckets[hash_num],
							   &hash, sizeof (hash),
							   EINA_RBTREE_CMP_KEY_CB(_eina_stringshare_cmp), NULL);
   if (!ed)
     {
	ed = malloc(sizeof (Eina_Stringshare_Head));
	if (!ed) return NULL;
	ed->hash = hash;
	ed->head = NULL;

	share->buckets[hash_num] = (Eina_Stringshare_Head*) eina_rbtree_inline_insert((Eina_Rbtree*) share->buckets[hash_num],
										      &ed->node,
										      EINA_RBTREE_CMP_NODE_CB(_eina_stringshare_node), NULL);
     }

   for (el = ed->head;
	el && slen != el->length && memcmp(str, (const char*) (el + 1), slen) != 0;
	el = el->next)
     ;

   if (el)
     {
	el->references++;
	return (const char*) (el + 1);
     }

   el = malloc(sizeof (Eina_Stringshare_Node) + slen);
   if (!el) return NULL;
   el->references = 1;
   el->length = slen;

   el_str = (char*) (el + 1);
   memcpy(el_str, str, slen);

   el->next = ed->head;
   ed->head = el;

   return el_str;
}

/**
 * @brief Note that the given string has lost an instance.
 *
 * @param str string The given string.
 *
 * This function decreases the reference counter associated to @p str
 * if it exists. If that counter reaches 0, the memory associated to
 * @p str is freed. If @p str is NULL, the function returns
 * immediatly.
 */
EAPI void
eina_stringshare_del(const char *str)
{
   Eina_Stringshare_Head *ed;
   Eina_Stringshare_Node *el;
   Eina_Stringshare_Node *prev;
   int hash_num, slen, hash;

   if (!str) return;
   hash = eina_hash_djb2_len(str, &slen);
   hash_num = hash & 0xFF;
   hash &= 0xFFF;

   ed = (Eina_Stringshare_Head*) eina_rbtree_inline_lookup(&share->buckets[hash_num]->node,
							   &hash, sizeof (hash),
							   EINA_RBTREE_CMP_KEY_CB(_eina_stringshare_cmp), NULL);
   if (!ed) goto on_error;

   for (prev = NULL, el = ed->head;
	el && (const char*) (el + 1) != str;
	el = el->next)
     ;

   if (el)
     {
	el->references--;
	if (el->references == 0)
	  {
	     if (prev) prev->next = el->next;
	     else ed->head = el->next;
	     free(el);

	     if (ed->head == NULL)
	       {
		  share->buckets[hash_num] = (Eina_Stringshare_Head*) eina_rbtree_inline_remove(&share->buckets[hash_num]->node,
												&ed->node,
												EINA_RBTREE_CMP_NODE_CB(_eina_stringshare_node),
												NULL);
		  free(ed);
	       }
	  }
	return ;
     }

 on_error:
   EINA_ERROR_PWARN("EEEK trying to del non-shared stringshare \"%s\"\n", str);
   if (getenv("EINA_ERROR_ABORT")) abort();
}

/**
 * @}
 */

/**
 * @}
 */
