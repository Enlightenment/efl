/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

static int _evas_list_alloc_error = 0;

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
Evas_List *
evas_list_append(Evas_List *list, const void *data)
{
   Evas_List *l, *new_l;
   
   _evas_list_alloc_error = 0;
   new_l = malloc(sizeof(Evas_List));
   if (!new_l)
     {
	_evas_list_alloc_error = 1;
	return list;
     }
   new_l->next = NULL;
   new_l->prev = NULL;
   new_l->data = (void *)data;
   if (!list) 
     {
	new_l->last = new_l;
	new_l->count = 1;
	return new_l;
     }
   if (list->last)
     {
	l = list->last;
	l->next = new_l;
	new_l->prev = l;
	list->last = new_l;
	list->count++;
	return list;	
     }
   else
     {
	for (l = list; l; l = l->next)
	  {
	     if (!l->next)
	       {
		  l->next = new_l;
		  new_l->prev = l;
		  list->last = new_l;
		  list->count++;
		  return list;
	       }
	  }
     }
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
Evas_List *
evas_list_prepend(Evas_List *list, const void *data)
{
   Evas_List *new_l;
   
   _evas_list_alloc_error = 0;
   new_l = malloc(sizeof(Evas_List));
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
	new_l->last = new_l;
	new_l->count = 1;
	return new_l;
     }
   new_l->next = list;
   list->prev = new_l;
   new_l->last = list->last;
   list->last = NULL;
   new_l->count = list->count + 1;
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
Evas_List *
evas_list_append_relative(Evas_List *list, const void *data, const void *relative)
{
   Evas_List *l;

   _evas_list_alloc_error = 0;
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List *new_l;
	     
	     new_l = malloc(sizeof(Evas_List));
	     if (!new_l)
	       {
		  _evas_list_alloc_error = 1;
		  return list;
	       }
	     new_l->data = (void *)data;
	     if (l->next)
	       {
		  new_l->next = l->next;
		  l->next->prev = new_l;
	       }
	     else
	       new_l->next = NULL;
	     
	     l->next = new_l;
	     new_l->prev = l;
	     if (!new_l->next) list->last = new_l;
	     list->count++;
	     return list;
	  }
     }
   return evas_list_append(list, data);
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
Evas_List *
evas_list_prepend_relative(Evas_List *list, const void *data, const void *relative)
{
   Evas_List *l;
   
   _evas_list_alloc_error = 0;
   for (l = list; l; l = l->next)
     {
	if (l->data == relative)
	  {
	     Evas_List *new_l;
	     
	     new_l = malloc(sizeof(Evas_List));
             if (!new_l)
	       {
		  _evas_list_alloc_error = 1;
		  return list;
	       }
	     new_l->data = (void *)data;
	     new_l->prev = l->prev;
	     new_l->next = l;
	     if (l->prev) l->prev->next = new_l;
	     l->prev = new_l;
	     if (new_l->prev)
	       {
		  if (!new_l->next) list->last = new_l;
		  list->count++;
		  return list;
	       }
	     else
	       {
		  new_l->last = list->last;
		  list->last = NULL;
		  new_l->count = list->count + 1;
		  return new_l;
	       }
	  }
     }
   return evas_list_prepend(list, data);
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
Evas_List *
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
 * Calling this function takes the list note @p remove_list and removes it
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
Evas_List *
evas_list_remove_list(Evas_List *list, Evas_List *remove_list)
{
   Evas_List *return_l;
   
   if (!remove_list) return list;
   if (remove_list->next) remove_list->next->prev = remove_list->prev;
   if (remove_list->prev)
     {
	remove_list->prev->next = remove_list->next;
	return_l = list;
     }
   else
     {
	if (remove_list->next)
	  remove_list->next->count = remove_list->count;
	return_l = remove_list->next;
	if (return_l) return_l->last = list->last;
     }
   if (remove_list == list->last) list->last = remove_list->prev;
   free(remove_list);
   if (return_l) return_l->count--;
   return return_l;
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
void *
evas_list_find(Evas_List *list, const void *data)
{
   Evas_List *l;
   
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
Evas_List *
evas_list_find_list(Evas_List *list, const void *data)
{
   Evas_List *l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data) return l;
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
Evas_List *
evas_list_free(Evas_List *list)
{
   Evas_List *l, *free_l;
   
   for (l = list; l;)
     {
	free_l = l;
	l = l->next;
	free(free_l);
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
Evas_List *
evas_list_last(Evas_List *list)
{
   if (!list) return NULL;
   return list->last;
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
Evas_List *
evas_list_next(Evas_List *list)
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
Evas_List *
evas_list_prev(Evas_List *list)
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
void *
evas_list_data(Evas_List *list)
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
int
evas_list_count(Evas_List *list)
{
   if (!list) return 0;
   return list->count;
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
void *
evas_list_nth(Evas_List *list, int n)
{
   Evas_List *l = evas_list_nth_list(list, n);

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
Evas_List *
evas_list_nth_list(Evas_List *list, int n)
{
   int i;
   Evas_List *l;

   /* check for non-existing nodes */
   if ((!list) || (n < 0) || (n > list->count - 1)) return NULL;

   /* if the node is in the 2nd half of the list, search from the end
    * else, search from the beginning.
    */
   if (n > list->count / 2)
     {
	for (i = list->count - 1, l = list->last; l; l = l->prev, i--)
	  {
	     if (i == n) return l;
	  }
     }
   else
     {
	for (i = 0, l = list; l; l = l->next, i++)
	  {
	     if (i == n) return l;
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
Evas_List *
evas_list_reverse(Evas_List *list)
{
   Evas_List *l1, *l2;

   if (!list) return NULL;
   l1 = list;
   l2 = list->last;
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

static Evas_List *
evas_list_combine(Evas_List *l, Evas_List *ll, int (*func)(void *, void*))
{
    Evas_List *result = NULL;
    Evas_List *l_head = NULL, *ll_head = NULL;

    l_head = l;
    ll_head = ll;
    while(l && ll)
    {
	switch(func(l->data, ll->data))
	    {
		case -1:
		    result = evas_list_append(result, l->data);
		    l = evas_list_next(l);
		    break;
		case 0:
		    result = evas_list_append(result, l->data);
		    l = evas_list_next(l);
		    result = evas_list_append(result, ll->data);
		    ll = evas_list_next(ll);
		    break;
		case 1:
		    result = evas_list_append(result, ll->data);
		    ll = evas_list_next(ll);
		    break;
	    }
    }
    while(l)
    {
	result = evas_list_append(result, l->data);
	l = evas_list_next(l);
    }
    evas_list_free(l_head);
    while(ll)
    {
	result = evas_list_append(result, ll->data);
	ll = evas_list_next(ll);
    }
    evas_list_free(ll_head);
    return(result);
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
 * In the event of a memory allocation failure, It might segv.
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
Evas_List *
evas_list_sort(Evas_List *list, int size, int (*func)(void *, void *))
{
    Evas_List *l = NULL, *ll = NULL;
    int range = (size / 2);

    if(range > 0)
    {
	/* bleh evas list splicing */
	ll = evas_list_nth_list(list, range);
	if(ll->prev)
	{
	    list->last = ll->prev;
	    list->count = size - range;
	}
	ll->prev->next = NULL;
	ll->count = range;

	/* merge sort */
	l = evas_list_sort(list, range, func);
	ll = evas_list_sort(ll, size - range, func);
	list = evas_list_combine(l, ll, func);
    }
    return(list);
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
int
evas_list_alloc_error(void)
{
   return _evas_list_alloc_error;
}
