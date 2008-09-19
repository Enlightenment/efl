/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Gustavo Sverzut Barbieri, Tilman Sauerbeck,
 *                         Vincent Torri, Cedric Bail, Jorge Luis Zapata Muga,
 *                         Corey Donohoe
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
 * Copyright (C) 2004 ncn
 * Copyright (C) 2006 Sebastian Dransfeld
 * Copyright (C) 2007 Christopher Michael
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in
 *  all copies of the Software and its Copyright notices. In addition publicly
 *  documented acknowledgment must be given that this software has been used if no
 *  source code of this software is made available publicly. This includes
 *  acknowledgments in either Copyright notices, Manuals, Publicity and Marketing
 *  documents or any documentation provided with any product containing this
 *  software. This License does not apply to any software that links to the
 *  libraries provided by this software (statically or dynamically), but only to
 *  the software provided.

 *  Please see the OLD-COPYING.PLAIN for a plain-english explanation of this notice
 *  and it's intent.

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

#include <assert.h>

#include "eina_error.h"
#include "eina_list.h"
#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eina_Iterator_List Eina_Iterator_List;
typedef struct _Eina_Accessor_List Eina_Accessor_List;

struct _Eina_Iterator_List
{
   Eina_Iterator iterator;

   const Eina_List *head;
   const Eina_List *current;
};

struct _Eina_Accessor_List
{
   Eina_Accessor accessor;

   const Eina_List *head;
   const Eina_List *current;

   unsigned int index;
};

static inline Eina_List_Accounting*
_eina_list_mempool_accounting_new(__UNUSED__ Eina_List *list)
{
   Eina_List_Accounting *tmp;

   tmp = malloc(sizeof (Eina_List_Accounting));
   if (!tmp) eina_error_set(EINA_ERROR_OUT_OF_MEMORY);

   return tmp;
}
static inline void
_eina_list_mempool_accounting_free(Eina_List_Accounting *accounting)
{
   free(accounting);
}

static inline Eina_List*
_eina_list_mempool_list_new(__UNUSED__ Eina_List *list)
{
   Eina_List *tmp;

   tmp = malloc(sizeof (Eina_List));
   if (!tmp) eina_error_set(EINA_ERROR_OUT_OF_MEMORY);

   return tmp;
}
static inline void
_eina_list_mempool_list_free(Eina_List *list)
{
   list->accounting->count--;
   if (list->accounting->count == 0)
     _eina_list_mempool_accounting_free(list->accounting);

   free(list);
}

static Eina_List *
_eina_list_setup_accounting(Eina_List *list)
{
   list->accounting = _eina_list_mempool_accounting_new(list);
   if (!list->accounting) goto on_error;

   list->accounting->last = list;
   list->accounting->count = 1;

   return list;

 on_error:
   _eina_list_mempool_list_free(list);
   return NULL;
}

static inline void
_eina_list_update_accounting(Eina_List *list, Eina_List *new_list)
{
   list->accounting->count++;
   new_list->accounting = list->accounting;
}

#if 0
static Eina_Mempool2 _eina_list_mempool =
{
   sizeof(Eina_List),
   320,
   0, NULL, NULL
};
static Eina_Mempool2 _eina_list_accounting_mempool =
{
   sizeof(Eina_List_Accounting),
   80,
   0, NULL, NULL
};
#endif

static Eina_Bool
eina_list_iterator_next(Eina_Iterator_List *it, void **data)
{
   if (it->current == NULL) return EINA_FALSE;
   if (data) *data = eina_list_data(it->current);

   it->current = eina_list_next(it->current);

   return EINA_TRUE;
}

static Eina_List *
eina_list_iterator_get_container(Eina_Iterator_List *it)
{
   return (Eina_List *) it->head;
}

static void
eina_list_iterator_free(Eina_Iterator_List *it)
{
   free(it);
}

static Eina_Bool
eina_list_accessor_get_at(Eina_Accessor_List *it, unsigned int index, void **data)
{
   const Eina_List *over;
   unsigned int middle;
   unsigned int i;

   if (index > eina_list_count(it->head)) return EINA_FALSE;

   if (it->index == index)
     {
	over = it->current;
     }
   else if (index > it->index)
     {
	/* After current position. */
	middle = ((eina_list_count(it->head) - it->index) >> 1) + it->index;

	if (index > middle)
	  {
	     /* Go backward from the end. */
	     for (i = eina_list_count(it->head) - 1, over = eina_list_last(it->head);
		  i > index && over != NULL;
		  --i, over = eina_list_prev(over))
	       ;
	  }
	else
	  {
	     /* Go forward from current. */
	     for (i = it->index, over = it->current;
		  i < index && over != NULL;
		  ++i, over = eina_list_next(over))
	       ;
	  }
     }
   else
     {
	/* Before current position. */
	middle = it->index >> 1;

	if (index > middle)
	  {
	     /* Go backward from current. */
	     for (i = it->index, over = it->current;
		  i > index && over != NULL;
		  --i, over = eina_list_prev(over))
	       ;
	  }
	else
	  {
	     /* Go forward from start. */
	     for (i = 0, over = it->head;
		  i < index && over != NULL;
		  ++i, over = eina_list_next(over))
	       ;
	  }
     }

   if (over == NULL) return EINA_FALSE;

   it->current = over;
   it->index = index;

   if (data) *data = eina_list_data(it->current);
   return EINA_TRUE;
}

static Eina_List *
eina_list_accessor_get_container(Eina_Accessor_List *it)
{
   return (Eina_List *) it->head;
}

static void
eina_list_accessor_free(Eina_Accessor_List *it)
{
   free(it);
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
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @addtogroup Eina_List_Group List
 *
 * @brief give a small description here : what it is for, what it does
 * , etc...
 *
 * List API. Give some hints about the use (functions that must be
 * used like init / shutdown), general use, etc... Give also a link to
 * tutorial below.
 *
 * @section listtable_algo Algorithm
 *
 * Give here the algorithm used in the implementation
 *
 * @section list_perf Performance
 *
 * Give some hints about performance if it is possible, and an image !
 *
 * @section list_tutorial Tutorial
 *
 * Here is a fantastic tutorial about our list
 *
 * @{
 */

/**
 * Initialize the eina list internal structure.
 * @return  Zero on failure, non-zero on successful initialization.
 */
EAPI int
eina_list_init(void)
{
   return eina_error_init();
}

/**
 * Shutdown the eina list internal structures
 */
EAPI int
eina_list_shutdown(void)
{
   return eina_error_shutdown();
}

/**
 * Appends the given data to the given linked list.
 *
 * The following example code demonstrates how to ensure that the
 * given data has been successfully appended.
 *
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 *
 * list = eina_list_append(list, my_data);
 * if (eina_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @param   list The given list.  If @c NULL is given, then a new list
 *               is created.
 * @param   data The data to append.
 * @return  A new list pointer that should be used in place of the one
 *          given to this function if successful.  Otherwise, the old
 *          pointer is returned.
 */
EAPI Eina_List *
eina_list_append(Eina_List *list, const void *data)
{
   Eina_List *l, *new_l;

   eina_error_set(0);
   new_l = _eina_list_mempool_list_new(list);
   if (!new_l) return list;
   new_l->next = NULL;
   new_l->data = (void *)data;
   if (!list)
     {
	new_l->prev = NULL;
	return _eina_list_setup_accounting(new_l);
     }

   l = list->accounting->last;
   list->accounting->last = new_l;

   l->next = new_l;
   new_l->prev = l;

   _eina_list_update_accounting(list, new_l);
   return list;
}

/**
 * Prepends the given data to the given linked list.
 *
 * The following example code demonstrates how to ensure that the
 * given data has been successfully prepended.
 *
 * Example:
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 *
 * list = eina_list_prepend(list, my_data);
 * if (eina_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @param   list The given list.
 * @param   data The given data.
 * @return  A new list pointer that should be used in place of the one
 *          given to this function, if successful.  Otherwise, the old
 *          pointer is returned.
 */
EAPI Eina_List *
eina_list_prepend(Eina_List *list, const void *data)
{
   Eina_List *new_l;

   eina_error_set(0);
   new_l = _eina_list_mempool_list_new(list);
   if (!new_l) return list;

   new_l->prev = NULL;
   new_l->next = list;
   new_l->data = (void *)data;

   if (!list) return _eina_list_setup_accounting(new_l);

   list->prev = new_l;

   _eina_list_update_accounting(list, new_l);

   return new_l;
}

/**
 * Inserts the given data into the given linked list after the specified data.
 *
 * If @p relative is not in the list, @p data is appended to the end of the
 * list.  If there are multiple instances of @p relative in the list,
 * @p data is inserted after the first instance.
 *
 * The following example code demonstrates how to ensure that the
 * given data has been successfully inserted.
 *
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 * extern void *relative_member;
 *
 * list = eina_list_append(list, relative_member);
 * if (eina_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * list = eina_list_append_relative(list, my_data, relative_member);
 * if (eina_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @param   list The given linked list.
 * @param   data The given data.
 * @param   relative The data to insert after.
 * @return  A new list pointer that should be used in place of the one
 *          given to this function if successful.  Otherwise, the old pointer
 *          is returned.
 */
EAPI Eina_List *
eina_list_append_relative(Eina_List *list, const void *data, const void *relative)
{
   Eina_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  return eina_list_append_relative_list(list, data, l);
     }
   return eina_list_append(list, data);
}

EAPI Eina_List *
eina_list_append_relative_list(Eina_List *list, const void *data, Eina_List *relative)
{
   Eina_List *new_l;

   if ((!list) || (!relative)) return eina_list_append(list, data);
   eina_error_set(0);
   new_l = _eina_list_mempool_list_new(list);
   if (!new_l) return list;

   new_l->next = relative->next;
   new_l->data = (void *)data;

   if (relative->next)
     relative->next->prev = new_l;

   relative->next = new_l;
   new_l->prev = relative;

   _eina_list_update_accounting(list, new_l);

   if (!new_l->next)
     new_l->accounting->last = new_l;

   return list;
}

/**
 * Prepend a data pointer to a linked list before the memeber specified
 * @param list The list handle to prepend @p data too
 * @param data The data pointer to prepend to list @p list before @p relative
 * @param relative The data pointer before which to insert @p data
 * @return A new list handle to replace the old one

 * Inserts the given data into the given linked list before the member
 * specified.
 *
 * If @p relative is not in the list, @p data is prepended to the
 * start of the list.  If there are multiple instances of @p relative
 * in the list, @p data is inserted before the first instance.
 *
 * The following code example demonstrates how to ensure that the
 * given data has been successfully inserted.
 *
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 * extern void *relative_member;
 *
 * list = eina_list_append(list, relative_member);
 * if (eina_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * list = eina_list_prepend_relative(list, my_data, relative_member);
 * if (eina_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @param   list The given linked list.
 * @param   data The given data.
 * @param   relative The data to insert before.
 * @return  A new list pointer that should be used in place of the one
 *          given to this function if successful.  Otherwise the old pointer
 *          is returned.
 */
EAPI Eina_List *
eina_list_prepend_relative(Eina_List *list, const void *data, const void *relative)
{
   Eina_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  return eina_list_prepend_relative_list(list, data, l);
     }
   return eina_list_prepend(list, data);
}

EAPI Eina_List *
eina_list_prepend_relative_list(Eina_List *list, const void *data, Eina_List *relative)
{
   Eina_List *new_l;

   if ((!list) || (!relative)) return eina_list_prepend(list, data);
   eina_error_set(0);
   new_l = _eina_list_mempool_list_new(list);
   if (!new_l) return list;

   new_l->prev = relative->prev;
   new_l->next = relative;
   new_l->data = (void *)data;

   if (relative->prev) relative->prev->next = new_l;
   relative->prev = new_l;

   _eina_list_update_accounting(list, new_l);

   if (new_l->prev)
     return list;

   return new_l;
}

/**
 * Removes the first instance of the specified data from the given list.
 *
 * If the specified data is not in the given list, nothing is done.
 *
 * @param   list The given list.
 * @param   data The specified data.
 * @return  A new list pointer that should be used in place of the one
 *          passed to this functions.
 */
EAPI Eina_List *
eina_list_remove(Eina_List *list, const void *data)
{
   Eina_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == data)
	  return eina_list_remove_list(list, l);
     }
   return list;
}

/**
 * Removes the specified data
 *
 * Remove a specified member from a list
 * @param list The list handle to remove @p remove_list from
 * @param remove_list The list node which is to be removed
 * @return A new list handle to replace the old one
 *
 * Calling this function takes the list node @p remove_list and removes it
 * from the list @p list, freeing the list node structure @p remove_list.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 * extern void *my_data;
 *
 * for (l = list; l; l= l->next)
 *   {
 *     if (l->data == my_data)
 *       {
 *         list = eina_list_remove_list(list, l);
 *         break;
 *       }
 *   }
 * @endcode
 */
EAPI Eina_List *
eina_list_remove_list(Eina_List *list, Eina_List *remove_list)
{
   Eina_List *return_l;

   if (!list) return NULL;
   if (!remove_list) return list;
   if (remove_list->next) remove_list->next->prev = remove_list->prev;
   if (remove_list->prev)
     {
	remove_list->prev->next = remove_list->next;
	return_l = list;
     }
   else
     return_l = remove_list->next;
   if (remove_list == list->accounting->last)
     list->accounting->last = remove_list->prev;

   _eina_list_mempool_list_free(remove_list);
   return return_l;
}

/**
 * Free an entire list and all the nodes, ignoring the data contained
 * @param list The list to free
 * @return A NULL pointer
 *
 * This function will free all the list nodes in list specified by @p list.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 *
 * list = eina_list_free(list);
 * @endcode
 */
EAPI Eina_List *
eina_list_free(Eina_List *list)
{
   Eina_List *l, *free_l;

   if (!list) return NULL;
   for (l = list; l;)
     {
	free_l = l;
	l = l->next;

	_eina_list_mempool_list_free(free_l);
     }
   return NULL;
}

/**
 * Moves the specified data to the head of the list
 *
 * Move a specified member to the head of the list
 * @param list The list handle to move @p inside
 * @param move_list The list node which is to be moved
 * @return A new list handle to replace the old one
 *
 * Calling this function takes the list node @p move_list and moves it
 * to the front of the @p list.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 * extern void *my_data;
 *
 * for (l = list; l; l= l->next)
 *   {
 *     if (l->data == my_data)
 *       {
 *         list = eina_list_promote_list(list, l);
 *         break;
 *       }
 *   }
 * @endcode
 */
EAPI Eina_List *
eina_list_promote_list(Eina_List *list, Eina_List *move_list)
{
   if (!list) return NULL;
   if (!move_list) return list;
   /* Promoting head to be head. */
   if (move_list == list) return list;

   /* Update pointer to the last entry if necessary. */
   if (move_list == list->accounting->last)
     list->accounting->last = move_list->prev;

   /* Remove the promoted item from the list. */
   if (move_list->next) move_list->next->prev = move_list->prev;
   if (move_list->prev) move_list->prev->next = move_list->next;
   else list = move_list->next;

   assert(list);

   move_list->prev = list->prev;
   if (list->prev)
     list->prev->next = move_list;
   list->prev = move_list;
   move_list->next = list;
   return move_list;
}

/**
 * Find a member of a list and return the member
 * @param list The list handle to search for @p data
 * @param data The data pointer to find in the list @p list
 * @return The found member data pointer
 *
 * A call to this function will search the list @p list from beginning to end
 * for the first member whose data pointer is @p data. If it is found, @p data
 * will be returned, otherwise NULL will be returned.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * extern void *my_data;
 *
 * if (eina_list_find(list, my_data) == my_data)
 *   {
 *     printf("Found member %p\n", my_data);
 *   }
 * @endcode
 */
EAPI void *
eina_list_find(const Eina_List *list, const void *data)
{
   if (eina_list_find_list(list, data)) return (void*) data;
   return NULL;
}

/**
 * Find a member of a list and return the list node containing that member
 * @param list The list handle to search for @p data
 * @param data The data pointer to find in the list @p list
 * @return The found members list node
 *
 * A call to this function will search the list @p list from beginning to end
 * for the first member whose data pointer is @p data. If it is found, the
 * list node containing the specified member will be returned, otherwise NULL
 * will be returned.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * extern void *my_data;
 * Eina_List *found_node;
 *
 * found_node = eina_list_find_list(list, my_data);
 * if (found_node)
 *   {
 *     printf("Found member %p\n", found_node->data);
 *   }
 * @endcode
 */
EAPI Eina_List *
eina_list_find_list(const Eina_List *list, const void *data)
{
   const Eina_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == data) return (Eina_List *)l;
     }
   return NULL;
}

/**
 * Get the nth member's data pointer in a list
 * @param list The list to get member number @p n from
 * @param n The number of the element (0 being the first)
 * @return The data pointer stored in the specified element
 *
 * This function returns the data pointer of element number @p n, in the list
 * @p list. The first element in the array is element number 0. If the element
 * number @p n does not exist, NULL will be returned.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * extern int number;
 * void *data;
 *
 * data = eina_list_nth(list, number);
 * if (data)
 *   printf("Element number %i has data %p\n", number, data);
 * @endcode
 */
EAPI void *
eina_list_nth(const Eina_List *list, unsigned int n)
{
   Eina_List *l;

   l = eina_list_nth_list(list, n);
   return l ? l->data : NULL;
}

/**
 * Get the nth member's list node in a list
 * @param list The list to get member number @p n from
 * @param n The number of the element (0 being the first)
 * @return The list node stored in the numbered element
 *
 * This function returns the list node of element number @p n, in the list
 * @p list. The first element in the array is element number 0. If the element
 * number @p n does not exist, NULL will be returned.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * extern int number;
 * Eina_List *nth_list;
 *
 * nth_list = eina_list_nth_list(list, number);
 * if (nth_list)
 *   printf("Element number %i has data %p\n", number, nth_list->data);
 * @endcode
 */
EAPI Eina_List *
eina_list_nth_list(const Eina_List *list, unsigned int n)
{
   const Eina_List *l;
   unsigned int i;

   /* check for non-existing nodes */
   if ((!list) || (n > (list->accounting->count - 1)))
     return NULL;

   /* if the node is in the 2nd half of the list, search from the end
    * else, search from the beginning.
    */
   if (n > (list->accounting->count / 2))
     {
	for (i = list->accounting->count - 1,
	     l = list->accounting->last;
	     l;
	     l = l->prev, i--)
	  {
	     if (i == n) return (Eina_List *)l;
	  }
     }
   else
     {
	for (i = 0, l = list; l; l = l->next, i++)
	  {
	     if (i == n) return (Eina_List *)l;
	  }
     }
   abort();
}

/**
 * Get the last list node in the list
 * @param list The list to get the last list node from
 * @return The last list node in the list @p list
 *
 * This function will return the last list node in the list (or NULL if the
 * list is empty).
 *
 * NB: This is a order-1 operation (it takes the same short time regardless of
 * the length of the list).
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *last, *l;
 *
 * last = eina_list_last(list);
 * printf("The list in reverse:\n");
 * for (l = last; l; l = l->prev)
 *   {
 *     printf("%p\n", l->data);
 *   }
 * @endcode
 */
static inline Eina_List *eina_list_last(const Eina_List *list);

/**
 * Get the next list node after the specified list node
 * @param list The list node to get the next list node from
 * @return The next list node, or NULL if no next list node exists
 *
 * This function returns the next list node after the current one. It is
 * equivalent to list->next.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 *
 * printf("The list:\n");
 * for (l = list; l; l = eina_list_next(l))
 *   {
 *     printf("%p\n", l->data);
 *   }
 * @endcode
 */
static inline Eina_List *eina_list_next(const Eina_List *list);

/**
 * Get the previous list node before the specified list node
 * @param list The list node to get the previous list node from
 * @return The previous list node, or NULL if no previous list node exists
 *
 * This function returns the previous list node before the current one. It is
 * equivalent to list->prev.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *last, *l;
 *
 * last = eina_list_last(list);
 * printf("The list in reverse:\n");
 * for (l = last; l; l = eina_list_prev(l))
 *   {
 *     printf("%p\n", l->data);
 *   }
 * @endcode
 */
static inline Eina_List *eina_list_prev(const Eina_List *list);

/**
 * Get the list node data member
 * @param list The list node to get the data member of
 * @return The data member from the list node @p list
 *
 * This function returns the data member of the specified list node @p list.
 * It is equivalent to list->data.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 *
 * printf("The list:\n");
 * for (l = list; l; l = eina_list_next(l))
 *   {
 *     printf("%p\n", eina_list_data(l));
 *   }
 * @endcode
 */
static inline void *eina_list_data(const Eina_List *list);

/**
 * Get the count of the number of items in a list
 * @param list The list whose count to return
 * @return The number of members in the list @p list
 *
 * This function returns how many members in the specified list: @p list. If
 * the list is empty (NULL), 0 is returned.
 *
 * NB: This is an order-1 operation and takes the same tiem regardless of the
 * length of the list.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 *
 * printf("The list has %i members\n", eina_list_count(list));
 * @endcode
 */
static inline unsigned int eina_list_count(const Eina_List *list);

/**
 * Reverse all the elements in the list
 * @param list The list to reverse
 * @return The list after it has been reversed
 *
 * This takes a list @p list, and reverses the order of all elements in the
 * list, so the last member is now first, and so on.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 *
 * list = eina_list_reverse(list);
 * @endcode
 */
EAPI Eina_List *
eina_list_reverse(Eina_List *list)
{
   Eina_List *l1, *l2;

   if (!list) return NULL;
   l1 = list;
   l2 = list->accounting->last;
   while (l1 != l2)
     {
	void *data;

	data = l1->data;
	l1->data = l2->data;
	l2->data = data;
	l1 = l1->next;
	if (l1 == l2) break;
	l2 = l2->prev;
     }

   return list;
}

/**
 * Sort a list according to the ordering func will return
 * @param list The list handle to sort
 * @param size The length of the list to sort
 * @param func A function pointer that can handle comparing the list data
 * nodes
 * @return A new sorted list
 *
 * This function sorts your list.  The data in your nodes can be arbitrary,
 * you just have to be smart enough to know what kind of data is in your
 * lists
 *
 * Example:
 * @code
 * int
 * sort_cb(void *d1, void *d2)
 * {
 *   const char *txt = NULL;
 *    const char *txt2 = NULL;
 *
 *    if(!d1) return(1);
 *    if(!d2) return(-1);
 *
 *    return(strcmp((const char*)d1, (const char*)d2));
 * }
 * extern Eina_List *list;
 *
 * list = eina_list_sort(list, eina_list_count(list), sort_cb);
 * if (eina_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List Sorting failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 */
EAPI Eina_List *
eina_list_sort(Eina_List *list, unsigned int size, int (*func)(void *, void *))
{
   Eina_List*   last;
   unsigned int	list_number;
   unsigned int	middle;
   unsigned int	list_size;

   if (!list || !func) return NULL;

   /* if the caller specified an invalid size, sort the whole list */
   if ((size == 0) ||
       (size > list->accounting->count))
     size = list->accounting->count;

   last = list->accounting->last;
   middle = size - size / 2;

   for (list_number = middle, list_size = 1;
	list_size < middle * 2;
	list_number >>= 1, list_size <<= 1)
     {
	Eina_List	*head1 = list;
	unsigned int	limit = size;
	unsigned int	process_list;
	unsigned int	pass_number;
	unsigned int	split_size = list_size;

	for (process_list = 0; process_list < list_number + 1; ++process_list)
	  {
	     Eina_List		*head2;
	     unsigned int	size_sum;
	     int		size1, size2;
	     int		i;

	     size1 = limit < split_size ? limit : split_size;
	     limit -= size1;

	     size2 = limit < split_size ? limit : split_size;
	     limit -= size2;

	     size_sum = size1 + size2;

	     for (head2 = head1, i = 0; i < size1; ++i)
	       head2 = eina_list_next (head2);

	     for (pass_number = 0; pass_number < size_sum; ++pass_number)
	       {
		  Eina_List	*next;
		  Eina_List	*prev1;
		  Eina_List	*prev2;

		  if (size1 == 0 || head1 == NULL) /* List1 is empty, head1 is already at the end of the list. So only need to update head2 */
		    {
		       for (; pass_number < size_sum; ++pass_number)
			 head2 = eina_list_next (head2);
		       break;
		    }
		  else
		    if (size2 == 0 || head2 == NULL) /* List2 is empty, just leave */
		      break;
		    else
		      if (func (head1->data, head2->data) < 0)
			{
			   head1 = eina_list_next (head1);
			   --size1;
			}
		      else
			{
			   next = eina_list_next (head2);
			   prev1 = eina_list_prev (head1);
			   prev2 = eina_list_prev (head2);

			   if (next)
			     next->prev = prev2;
			   if (prev1)
			     prev1->next = head2;
			   if (prev2)
			     prev2->next = next;

			   head2->prev = prev1;
			   head2->next = head1;
			   head1->prev = head2;

			   --size2;

                           if (head1 == list)
                             list = head2;
                           if (head2 == last)
                             last = prev2;

			   head2 = next;
			}
	       }
	     head1 = head2;
	  }
     }

   list->accounting->last = last;
   return list;
}

EAPI Eina_Iterator *
eina_list_iterator_new(const Eina_List *list)
{
   Eina_Iterator_List *it;

   if (!list) return NULL;

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Iterator_List));
   if (!it) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   it->head = list;
   it->current = list;

   it->iterator.next = FUNC_ITERATOR_NEXT(eina_list_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(eina_list_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_list_iterator_free);

   return &it->iterator;
}

EAPI Eina_Accessor *
eina_list_accessor_new(const Eina_List *list)
{
   Eina_Accessor_List *it;

   if (!list) return NULL;

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Accessor_List));
   if (!it) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   it->head = list;
   it->current = list;
   it->index = 0;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(eina_list_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(eina_list_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(eina_list_accessor_free);

   return &it->accessor;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
