/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "Evas_Data.h"
#include <evas_mempool.h>

typedef struct _Evas_List_Accounting Evas_List_Accounting;

struct _Evas_List_Accounting
{
   Evas_List *last;
   int        count;
};

static int _evas_list_alloc_error = 0;

static Evas_Mempool _evas_list_mempool =
{
   sizeof(Evas_List),
   320,
   0, NULL, NULL
};
static Evas_Mempool _evas_list_accounting_mempool =
{
   sizeof(Evas_List_Accounting),
   80,
   0, NULL, NULL
};

/**
 * @defgroup Evas_List_Data_Group Linked List Creation Functions
 *
 * Functions that add data to an Evas_List.
 */

/**
 * Appends the given data to the given linked list.
 *
 * The following example code demonstrates how to ensure that the
 * given data has been successfully appended.
 *
 * @code
 * Evas_List *list = NULL;
 * extern void *my_data;
 *
 * list = evas_list_append(list, my_data);
 * if (evas_list_alloc_error())
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
 * @ingroup Evas_List_Data_Group
 */
EAPI Evas_List *
evas_list_append(Evas_List *list, const void *data)
{
   Evas_List *l, *new_l;

   _evas_list_alloc_error = 0;
   new_l = evas_mempool_malloc(&_evas_list_mempool, sizeof(Evas_List));
   if (!new_l)
     {
	_evas_list_alloc_error = 1;
	return list;
     }
   new_l->next = NULL;
   new_l->data = (void *)data;
   if (!list)
     {
	new_l->prev = NULL;
	new_l->accounting = evas_mempool_malloc(&_evas_list_accounting_mempool, sizeof(Evas_List_Accounting));
	if (!new_l->accounting)
	  {
	     _evas_list_alloc_error = 1;
	     evas_mempool_free(&_evas_list_mempool, new_l);
	     return list;
	  }
	new_l->accounting->last = new_l;
	new_l->accounting->count = 1;
	return new_l;
     }
   l = list->accounting->last;
   l->next = new_l;
   new_l->prev = l;
   new_l->accounting = list->accounting;
   list->accounting->last = new_l;
   list->accounting->count++;
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
 * Evas_List *list = NULL;
 * extern void *my_data;
 *
 * list = evas_list_prepend(list, my_data);
 * if (evas_list_alloc_error())
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
 * @ingroup Evas_List_Data_Group
 */
EAPI Evas_List *
evas_list_prepend(Evas_List *list, const void *data)
{
   Evas_List *new_l;

   _evas_list_alloc_error = 0;
   new_l = evas_mempool_malloc(&_evas_list_mempool, sizeof(Evas_List));
   if (!new_l)
     {
	_evas_list_alloc_error = 1;
	return list;
     }
   new_l->prev = NULL;
   new_l->data = (void *)data;
   if (!list)
     {
	new_l->next = NULL;
	new_l->accounting = evas_mempool_malloc(&_evas_list_accounting_mempool, sizeof(Evas_List_Accounting));
	if (!new_l->accounting)
	  {
	     _evas_list_alloc_error = 1;
	     evas_mempool_free(&_evas_list_mempool, new_l);
	     return list;
	  }
	new_l->accounting->last = new_l;
	new_l->accounting->count = 1;
	return new_l;
     }
   new_l->next = list;
   list->prev = new_l;
   new_l->accounting = list->accounting;
   list->accounting->count++;
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
 * Evas_List *list = NULL;
 * extern void *my_data;
 * extern void *relative_member;
 *
 * list = evas_list_append(list, relative_member);
 * if (evas_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * list = evas_list_append_relative(list, my_data, relative_member);
 * if (evas_list_alloc_error())
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
 * @ingroup Evas_List_Data_Group
 */
EAPI Evas_List *
evas_list_append_relative(Evas_List *list, const void *data, const void *relative)
{
   Evas_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  return evas_list_append_relative_list(list, data, l);
     }
   return evas_list_append(list, data);
}

EAPI Evas_List *
evas_list_append_relative_list(Evas_List *list, const void *data, Evas_List *relative)
{
   Evas_List *new_l;

   if ((!list) || (!relative)) return evas_list_append(list, data);
   _evas_list_alloc_error = 0;
   new_l = evas_mempool_malloc(&_evas_list_mempool, sizeof(Evas_List));
   if (!new_l)
     {
	_evas_list_alloc_error = 1;
	return list;
     }
   new_l->data = (void *)data;
   if (relative->next)
     {
	new_l->next = relative->next;
	relative->next->prev = new_l;
     }
   else
     new_l->next = NULL;

   relative->next = new_l;
   new_l->prev = relative;
   new_l->accounting = list->accounting;
   list->accounting->count++;
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
 * Evas_List *list = NULL;
 * extern void *my_data;
 * extern void *relative_member;
 *
 * list = evas_list_append(list, relative_member);
 * if (evas_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * list = evas_list_prepend_relative(list, my_data, relative_member);
 * if (evas_list_alloc_error())
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
 * @ingroup Evas_List_Data_Group
 */
EAPI Evas_List *
evas_list_prepend_relative(Evas_List *list, const void *data, const void *relative)
{
   Evas_List *l;

   _evas_list_alloc_error = 0;
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  return evas_list_prepend_relative_list(list, data, l);
     }
   return evas_list_prepend(list, data);
}

EAPI Evas_List *
evas_list_prepend_relative_list(Evas_List *list, const void *data, Evas_List *relative)
{
   Evas_List *new_l;

   if ((!list) || (!relative)) return evas_list_prepend(list, data);
   _evas_list_alloc_error = 0;
   new_l = evas_mempool_malloc(&_evas_list_mempool, sizeof(Evas_List));
   if (!new_l)
     {
	_evas_list_alloc_error = 1;
	return list;
     }
   new_l->data = (void *)data;
   new_l->prev = relative->prev;
   new_l->next = relative;
   if (relative->prev) relative->prev->next = new_l;
   relative->prev = new_l;
   new_l->accounting = list->accounting;
   list->accounting->count++;
   if (new_l->prev)
     return list;
   return new_l;
}

/**
 * @defgroup Evas_List_Remove_Group Linked List Remove Functions
 *
 * Functions that remove data from linked lists.
 */

/**
 * Removes the first instance of the specified data from the given list.
 *
 * If the specified data is not in the given list, nothing is done.
 *
 * @param   list The given list.
 * @param   data The specified data.
 * @return  A new list pointer that should be used in place of the one
 *          passed to this functions.
 * @ingroup Evas_List_Remove_Group
 */
EAPI Evas_List *
evas_list_remove(Evas_List *list, const void *data)
{
   Evas_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == data)
	  return evas_list_remove_list(list, l);
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
 * extern Evas_List *list;
 * Evas_List *l;
 * extern void *my_data;
 *
 * for (l = list; l; l= l->next)
 *   {
 *     if (l->data == my_data)
 *       {
 *         list = evas_list_remove_list(list, l);
 *         break;
 *       }
 *   }
 * @endcode
 * @ingroup Evas_List_Remove_Group
 */
EAPI Evas_List *
evas_list_remove_list(Evas_List *list, Evas_List *remove_list)
{
   Evas_List *return_l;

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
   list->accounting->count--;
   if (list->accounting->count == 0)
     evas_mempool_free(&_evas_list_accounting_mempool, list->accounting);
   evas_mempool_free(&_evas_list_mempool, remove_list);
   return return_l;
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
 * extern Evas_List *list;
 * Evas_List *l;
 * extern void *my_data;
 *
 * for (l = list; l; l= l->next)
 *   {
 *     if (l->data == my_data)
 *       {
 *         list = evas_list_promote_list(list, l);
 *         break;
 *       }
 *   }
 * @endcode
 * @ingroup Evas_List_Promote_Group
 */
EAPI Evas_List *
evas_list_promote_list(Evas_List *list, Evas_List *move_list)
{
   Evas_List *return_l;

   if (!list) return NULL;
   if (!move_list) return list;
   if (move_list == list) return list;
   if (move_list->next) move_list->next->prev = move_list->prev;
   if (move_list->prev)
     {
	move_list->prev->next = move_list->next;
	return_l = list;
     }
   else
     return_l = move_list->next;
   if (move_list == list->accounting->last)
     list->accounting->last = move_list->prev;
   move_list->prev = return_l->prev;
   if (return_l->prev)
     return_l->prev->next = move_list;
   return_l->prev = move_list;
   move_list->next = return_l;
   return move_list;
}



/**
 * @defgroup Evas_List_Find_Group Linked List Find Functions
 *
 * Functions that find specified data in a linked list.
 */

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
 * extern Evas_List *list;
 * extern void *my_data;
 *
 * if (evas_list_find(list, my_data) == my_data)
 *   {
 *     printf("Found member %p\n", my_data);
 *   }
 * @endcode
 * @ingroup Evas_List_Find_Group
 */
EAPI void *
evas_list_find(const Evas_List *list, const void *data)
{
   const Evas_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == data) return (void *)data;
     }
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
 * extern Evas_List *list;
 * extern void *my_data;
 * Evas_List *found_node;
 *
 * found_node = evas_list_find_list(list, my_data);
 * if (found_node)
 *   {
 *     printf("Found member %p\n", found_node->data);
 *   }
 * @endcode
 * @ingroup Evas_List_Find_Group
 */
EAPI Evas_List *
evas_list_find_list(const Evas_List *list, const void *data)
{
   const Evas_List *l;

   for (l = list; l; l = l->next)
     {
	if (l->data == data) return (Evas_List *)l;
     }
   return NULL;
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
 * extern Evas_List *list;
 *
 * list = evas_list_free(list);
 * @endcode
 * @ingroup Evas_List_Remove_Group
 */
EAPI Evas_List *
evas_list_free(Evas_List *list)
{
   Evas_List *l, *free_l;

   if (!list) return NULL;
   evas_mempool_free(&_evas_list_accounting_mempool, list->accounting);
   for (l = list; l;)
     {
	free_l = l;
	l = l->next;
	evas_mempool_free(&_evas_list_mempool, free_l);
     }
   return NULL;
}

/**
 * @defgroup Evas_List_Traverse_Group Linked List Traverse Functions
 *
 * Functions that you can use to traverse a linked list.
 */

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
 * extern Evas_List *list;
 * Evas_List *last, *l;
 *
 * last = evas_list_last(list);
 * printf("The list in reverse:\n");
 * for (l = last; l; l = l->prev)
 *   {
 *     printf("%p\n", l->data);
 *   }
 * @endcode
 * @ingroup Evas_List_Traverse_Group
 */
EAPI Evas_List *
evas_list_last(const Evas_List *list)
{
   if (!list) return NULL;
   return list->accounting->last;
}

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
 * extern Evas_List *list;
 * Evas_List *l;
 *
 * printf("The list:\n");
 * for (l = list; l; l = evas_list_next(l))
 *   {
 *     printf("%p\n", l->data);
 *   }
 * @endcode
 * @ingroup Evas_List_Traverse_Group
 */
EAPI Evas_List *
evas_list_next(const Evas_List *list)
{
   if (!list) return NULL;
   return list->next;
}

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
 * extern Evas_List *list;
 * Evas_List *last, *l;
 *
 * last = evas_list_last(list);
 * printf("The list in reverse:\n");
 * for (l = last; l; l = evas_list_prev(l))
 *   {
 *     printf("%p\n", l->data);
 *   }
 * @endcode
 * @ingroup Evas_List_Traverse_Group
 */
EAPI Evas_List *
evas_list_prev(const Evas_List *list)
{
   if (!list) return NULL;
   return list->prev;
}

/**
 * @defgroup Evas_List_General_Group Linked List General Functions
 *
 * Miscellaneous functions that work on linked lists.
 */

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
 * extern Evas_List *list;
 * Evas_List *l;
 *
 * printf("The list:\n");
 * for (l = list; l; l = evas_list_next(l))
 *   {
 *     printf("%p\n", evas_list_data(l));
 *   }
 * @endcode
 * @ingroup Evas_List_General_Group
 */
EAPI void *
evas_list_data(const Evas_List *list)
{
   if (!list) return NULL;
   return list->data;
}

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
 * extern Evas_List *list;
 *
 * printf("The list has %i members\n", evas_list_count(list));
 * @endcode
 * @ingroup Evas_List_General_Group
 */
EAPI int
evas_list_count(const Evas_List *list)
{
   if (!list) return 0;
   return list->accounting->count;
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
 * extern Evas_List *list;
 * extern int number;
 * void *data;
 *
 * data = evas_list_nth(list, number);
 * if (data)
 *   printf("Element number %i has data %p\n", number, data);
 * @endcode
 * @ingroup Evas_List_Find_Group
 */
EAPI void *
evas_list_nth(const Evas_List *list, int n)
{
   Evas_List *l;

   l = evas_list_nth_list(list, n);
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
 * extern Evas_List *list;
 * extern int number;
 * Evas_List *nth_list;
 *
 * nth_list = evas_list_nth_list(list, number);
 * if (nth_list)
 *   printf("Element number %i has data %p\n", number, nth_list->data);
 * @endcode
 * @ingroup Evas_List_Find_Group
 */
EAPI Evas_List *
evas_list_nth_list(const Evas_List *list, int n)
{
   int i;
   const Evas_List *l;

   /* check for non-existing nodes */
   if ((!list) || (n < 0) ||
       (n > (list->accounting->count - 1)))
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
	     if (i == n) return (Evas_List *)l;
	  }
     }
   else
     {
	for (i = 0, l = list; l; l = l->next, i++)
	  {
	     if (i == n) return (Evas_List *)l;
	  }
     }
   return NULL;
}

/**
 * @defgroup Evas_List_Ordering_Group Linked List Ordering Functions
 *
 * Functions that change the ordering of data in a linked list.
 */

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
 * extern Evas_List *list;
 *
 * list = evas_list_reverse(list);
 * @endcode
 * @ingroup Evas_List_Ordering_Group
 */
EAPI Evas_List *
evas_list_reverse(Evas_List *list)
{
   Evas_List *l1, *l2;

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
 * extern Evas_List *list;
 *
 * list = evas_list_sort(list, evas_list_count(list), sort_cb);
 * if (evas_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List Sorting failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 * @ingroup Evas_List_Ordering_Group
 */
EAPI Evas_List *
evas_list_sort(Evas_List *list, int size, int (*func)(void *, void *))
{
   Evas_List*   last;
   unsigned int	list_number;
   unsigned int	middle;
   int		list_size;

   if (!list || !func)
     return NULL;

   /* if the caller specified an invalid size, sort the whole list */
   if ((size <= 0) ||
       (size > list->accounting->count))
     size = list->accounting->count;

   last = list->accounting->last;
   middle = size - size / 2;

   for (list_number = middle, list_size = 1;
	list_size < middle * 2;
	list_number >>= 1, list_size <<= 1)
     {
	Evas_List	*head1 = list;
	unsigned int	limit = size;
	unsigned int	process_list;
	unsigned int	pass_number;
	unsigned int	split_size = list_size;

	for (process_list = 0; process_list < list_number + 1; ++process_list)
	  {
	     Evas_List		*head2;
	     unsigned int	size_sum;
	     int		size1, size2;
	     int		i;

	     size1 = limit < split_size ? limit : split_size;
	     limit -= size1;

	     size2 = limit < split_size ? limit : split_size;
	     limit -= size2;

	     size_sum = size1 + size2;

	     for (head2 = head1, i = 0; i < size1; ++i)
	       head2 = evas_list_next (head2);

	     for (pass_number = 0; pass_number < size_sum; ++pass_number)
	       {
		  Evas_List	*next;
		  Evas_List	*prev1;
		  Evas_List	*prev2;

		  if (size1 == 0 || head1 == NULL) /* List1 is empty, head1 is already at the end of the list. So only need to update head2 */
		    {
		       for (; pass_number < size_sum; ++pass_number)
			 head2 = evas_list_next (head2);
		       break;
		    }
		  else
		    if (size2 == 0 || head2 == NULL) /* List2 is empty, just leave */
		      break;
		    else
		      if (func (head1->data, head2->data) < 0)
			{
			   head1 = evas_list_next (head1);
			   --size1;
			}
		      else
			{
			   next = evas_list_next (head2);
			   prev1 = evas_list_prev (head1);
			   prev2 = evas_list_prev (head2);

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
/**
 * Return the memory allocation failure flag after any operation needin allocation
 * @return The state of the allocation flag
 *
 * This function returns the state of the memory allocation flag. This flag is
 * set if memory allocations during evas_list_append(), evas_list_prepend(),
 * evas_list_append_relative(), or evas_list_prepend_relative() fail. If they
 * do fail, 1 will be returned, otherwise 0 will be returned. The flag will
 * remain in its current state until the next call that requires allocation
 * is called, and is then reset.
 *
 * Example:
 * @code
 * Evas_List *list = NULL;
 * extern void *my_data;
 *
 * list = evas_list_append(list, my_data);
 * if (evas_list_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 * @ingroup Evas_List_General_Group
 */
EAPI int
evas_list_alloc_error(void)
{
   return _evas_list_alloc_error;
}
