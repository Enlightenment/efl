#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

static int _evas_list_alloc_error = 0;

/**
 * Append a data pointer to a linked list
 * @param list The list handle to append @p data too
 * @param data The data pointer to append to list @p list
 * @return A new list handle to replace the old one
 * 
 * This function appends the data pointed to by @p data to the end of the linked
 * list @p list, and returns a new list pointer to replace the old one. An
 * empty list should start as a NULL pointer.
 * 
 * In the event of a memory allocation failure, the old list pointer will be
 * returned and the application should use evas_list_alloc_error() to determine
 * if the allocation failed and sensibly recover from this condition.
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
 */
Evas_List *
evas_list_append(Evas_List *list, void *data)
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
   new_l->data = data;
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
 * Prepend a data pointer to a linked list
 * @param list The list handle to prepend @p data too
 * @param data The data pointer to prepend to list @p list
 * @return A new list handle to replace the old one
 * 
 * This function prepends the data pointed to by @p data to the start of the linked
 * list @p list, and returns a new list pointer to replace the old one. An
 * empty list should start as a NULL pointer.
 * 
 * In the event of a memory allocation failur, the old list pointer will be
 * returned and the application should use evas_list_alloc_error() to determine
 * if the allocation failed and sensibly recover from this condition.
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
 */
Evas_List *
evas_list_prepend(Evas_List *list, void *data)
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
   new_l->data = data;
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
 * Append a data pointer to a linked list after the memeber specified
 * @param list The list handle to append @p data too
 * @param data The data pointer to append to list @p list after @p relative
 * @param relative The data pointer after which to insert @p data
 * @return A new list handle to replace the old one
 * 
 * This function appends the data pointed to by @p data after the position in
 * of the list member whose data pointer is @p relative, in the list @p list, 
 * and returns a new list pointer to replace the old one. An empty list 
 * should start as a NULL pointer.
 * 
 * If no member in the list has a data pointer of @p relative, the item is
 * appended to the very end of the list. If there are multiple members of the
 * list with the data pointer @p relative, the @p data item is inserted after
 * the first instance of the member @p relative in the list.
 * 
 * In the event of a memory allocation failure, the old list pointer will be
 * returned and the application should use evas_list_alloc_error() to determine
 * if the allocation failed and sensibly recover from this condition.
 * 
 * Example:
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
 */
Evas_List *
evas_list_append_relative(Evas_List *list, void *data, void *relative)
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
	     new_l->data = data;
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
 * 
 * This function prepends the data pointed to by @p data before the position in
 * of the list member whose data pointer is @p relative, in the list @p list, 
 * and returns a new list pointer to replace the old one. An empty list 
 * should start as a NULL pointer.
 * 
 * If no member in the list has a data pointer of @p relative, the item is
 * prepended to the very start of the list. If there are multiple members of the
 * list with the data pointer @p relative, the @p data item is inserted before
 * the first instance of the member @p relative in the list.
 * 
 * In the event of a memory allocation failure, the old list pointer will be
 * returned and the application should use evas_list_alloc_error() to determine
 * if the allocation failed and sensibly recover from this condition.
 * 
 * Example:
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
 */
Evas_List *
evas_list_prepend_relative(Evas_List *list, void *data, void *relative)
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
	     new_l->data = data;
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
 * Remove a specified member from a list
 * @param list The list handle to remove @p data from
 * @param data The memebr to find and remove
 * @return A new list handle to replace the old one
 * 
 * Calling this function finds the first instance of the member whose data
 * pointer is @p data in the list @p list and removed that member from the
 * the list. An empty list should start as a NULL pointer.
 * 
 * If no member is found that matches, nothing is done.
 * 
 * Example:
 * @code
 * extern Evas_List *list;
 * extern void *my_data;
 * 
 * list = evas_list_remove(list, my_data);
 * @endcode
 */
Evas_List *
evas_list_remove(Evas_List *list, void *data)
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
 */
void *
evas_list_find(Evas_List *list, void *data)
{
   Evas_List *l;
   
   for (l = list; l; l = l->next)
     {
	if (l->data == data) return data;
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
 */
Evas_List *
evas_list_find_list(Evas_List *list, void *data)
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
 */
Evas_List *
evas_list_prev(Evas_List *list)
{
   if (!list) return NULL;
   return list->prev;
}

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
 */
void *
evas_list_nth(Evas_List *list, int n)
{
   int i;
   Evas_List *l;
   
   if (n < 0) return NULL;
   for (i = 0, l = list; l; l = l->next, i++)
     {
	if (i == n) return l->data;
     }
   return NULL;
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
 */
Evas_List *
evas_list_nth_list(Evas_List *list, int n)
{
   int i;
   Evas_List *l;
   
   if (n < 0) return NULL;
   for (i = 0, l = list; l; l = l->next, i++)
     {
	if (i == n) return l;
     }
   return NULL;
}

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
 */
int
evas_list_alloc_error(void)
{
   return _evas_list_alloc_error;
}
