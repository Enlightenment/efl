/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
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

#ifndef EINA_LIST_H_
#define EINA_LIST_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"
#include "eina_magic.h"

/**
 * @page eina_list_01_example_page Adding elements to Eina_List
 * @dontinclude eina_list_01.c
 *
 * Creating an @ref Eina_List and adding elements to it is very easy and can be
 * understood from an example:
 * First thing is always to include Eina.h, for this example we also
 * include stdio.h so we can use printf.
 * @skip #include
 * @until Eina.h
 * 
 * Just some boilerplate code, declaring some variable and initializing eina.
 * @until eina_init
 * Here we add a sequence of elements to our list. By using append we add
 * elements to the end of the list, so the list will look like this:@n
 * @htmlonly
 * <img src="eina_list_example_01_a.png" style="max-width: 100%;" />
 * <a href="eina_list_example_01_a.png">Full-size</a>
 * @endhtmlonly
 * @image rtf eina_list_example_01_a.png
 * @image latex eina_list_example_01_a.eps width=\textwidth
 * @until roslin
 * There are a couple of interesting things happening here, first is that we are
 * passing a NULL pointer to the first @ref eina_list_append() call, when this
 * is done a list is created. The other @b very important detail to notice is
 * that the return value is attributed to the @a list variable, this needs to
 * be done every time we use a a function that alters the contents of the list.
 * 
 * Now that we have a list with some elements in it we can look at it's contents.
 * @until printf
 * 
 * There are many ways of accessing elements in the list, including by it's
 * index:
 * @until nth
 * @note It should be noted that the index starts at 0.
 * 
 * @ref eina_list_append() is not the only way to add elements to a a list. A
 * common requirement is to add an element in a specific position this can be
 * accomplished using @ref eina_list_append_relative() and
 * @ref eina_list_append_relative_list():
 * @until zarek
 * First @a "cain" is added after the second element(remember that indexes are
 * 0 based) and then we add @a "zarek" after @a "cain".
 * 
 * @ref Eina_List also has prepend analogs to append functions we have used so
 * far:
 * @until lampkin
 * With this additions our list now looks like this:@n
 * @htmlonly
 * <img src="eina_list_example_01_b.png" style="max-width: 100%;" />
 * <a href="eina_list_example_01_b.png">Full-size</a>
 * @endhtmlonly
 * @image rtf eina_list_example_01_b.png
 * @image latex eina_list_example_01_b.eps width=\textwidth
 * 
 * Once done using the list it needs to be freed, and since we are done with
 * eina that also need to be shutdown:
 * @until }
 * 
 * The full source code can be found on the examples folder
 * on the @ref eina_list_01_c "eina_list_01.c" file.
 */

/**
 * @page eina_list_01_c Adding elements to Eina_List example
 *
 * @include eina_list_01.c
 * @example eina_list_01.c
 */

/**
 * @page eina_list_02_example_page Sorting Eina_List elements
 * @dontinclude eina_list_02.c
 * 
 * If you don't know how to create lists see
 * @ref eina_list_01_example_page.
 * 
 * @skip #include
 * @until boomer
 * This is the code we have already seen to create a list. Now if we need to
 * search the list we can do it like this:
 * @until return
 * 
 * However if searching the list multiple times it probably is better to sort
 * the list since the sorted_search functions are much faster:
 * @until return
 * 
 * Once the list is sorted it's not a good idea to use append/prepend functions
 * since that would add the element in the wrong place, instead elements should
 * be added with @ref eina_list_sorted_insert():
 * @until sorted_insert
 * 
 * A noteworthy use case is adding an element to a list only if it doesn't exist
 * already, this can accomplished by searching for the element that is closest
 * to what is being added, and if that doesn't match add:
 * @until append
 * @note @ref eina_list_search_sorted_near_list() will tell you not only the
 * nearest node to what was searched for but how it compares to your term, this
 * way it is easy to know if you have to add before or after that node.
 * 
 * It is sometimes useful to get a portion of the list as another list, here we
 * take every element that comes after "boomer" and split it into "other_list":
 * @until split_list
 * 
 * It is also possible to add entire lists of elements using
 * @ref eina_list_sorted_merge():
 * @until sorted_merge
 * 
 * And as always release memory and shutdown eina before ending:
 * @until }
 * 
 * The full source code can be found on the examples folder
 * on the @ref eina_list_02_c "eina_list_02.c" file.
 */

/**
 * @page eina_list_02_c Sorting Eina_List elements example
 *
 * @include eina_list_02.c
 * @example eina_list_02.c
 */

/**
 * @page eina_list_03_example_page Reordering Eina_List elements
 * @dontinclude eina_list_03.c
 * 
 * If you don't know how to create lists see
 * @ref eina_list_01_example_page.
 * 
 * We start out with code that should be familiar by now:
 * @skip #include
 * @until gemenon
 * 
 * You can move elements around in a list using @ref eina_list_move() or using
 * @ref eina_list_promote_list() and @ref eina_list_demote_list() which move a
 * list node to the head and end of the list respectevely:
 * @until demote
 * 
 * Removing elements from a list can be done with ease:
 * @until sagitarius
 * 
 * To replace an element in the list it is not necessary to remove it and then
 * add with the new value, it is possible to just change the value of a node:
 * @until aquarius
 *
 * We will now take a peek to see if the list still has the right number of
 * elements:
 * @until printf
 *
 * Now that the list is in alphabetical order let's create a copy of it in
 * reverse order and print every element to see if worked as expected:
 * @until iterator_free
 * @note Always remember to free your iterators when done using them.
 * 
 * And as always release memory and shutdown eina before ending:
 * @until }
 * 
 * The full source code can be found on the examples folder
 * on the @ref eina_list_03_c "eina_list_03.c" file.
 */

/**
 * @page eina_list_03_c Reordering Eina_List elements example
 *
 * @include eina_list_03.c
 * @example eina_list_03.c
 */

/**
 * @page eina_list_04_example_page Eina_List and memory allocation
 * @dontinclude eina_list_04.c
 *
 * If you don't know how to create lists see
 * @ref eina_list_01_example_page. In this example we also use
 * @ref Eina_Stringshare_Group, however it should be possible to understand the code
 * regardless of previous knowledge about it.
 *
 * Here we have the usual list creation code with a twist, now we are using as
 * data for the list memory that has to be freed later on.
 * @skip #include
 * @until Sharon
 *
 * This time we are going to iterate over our list in a different way:
 * @until printf
 *
 * And now we are going to iterate over the list backwards:
 * @until printf
 *
 * And now we need to free up the memory allocated during creation of the list:
 * @until stringshare_del
 * @note We don't need to use eina_list_free() since @ref EINA_LIST_FREE takes
 * care of that.
 *
 * And shut everything down:
 * @until }
 * 
 * The full source code can be found on the examples folder
 * on the @ref eina_list_04_c "eina_list_04.c" file.
 */

/**
 * @page eina_list_04_c Eina_List and memory allocation example
 *
 * @include eina_list_04.c
 * @example eina_list_04.c
 */

/**
 * @addtogroup Eina_List_Group List
 *
 * @brief These functions provide double linked list management.
 * 
 * Eina_List is a doubly linked list. It can store data of any type in the
 * form of void pointers. It has convenience functions to do all the common
 * operations which means it should rarely if ever be necessary to directly
 * access the struct's fields. Nevertheless it can be useful to understand the
 * inner workings of the data structure being used.
 * 
 * @ref Eina_List nodes keep references to the previous node, the next node, its
 * data and to an accounting structure.
 *
 * @htmlonly
 * <img src="eina_list.png" style="max-width: 100%;" />
 * <a href="eina_list.png">Full-size</a>
 * @endhtmlonly
 * @image rtf eina_list.png
 * @image latex eina_list.eps width=5cm
 *
 * @ref Eina_List_Accounting is used to improve the performance of some
 * functions. It is private and <b>should not</b> be modified. It contains a
 * reference to the end of the list and the number of elements in the list.
 * 
 * @note Every function that modifies the contents of the list returns a pointer
 * to the head of the list and it is essential that this be pointer be used in
 * any future references to the list.
 * 
 * Most functions have two versions that have the same effect but operate on
 * different arguments, the @a plain functions operate over data(eg.: 
 * @ref eina_list_append_relative, @ref eina_list_remove,
 * @ref eina_list_data_find), the @a list versions of these functions operate
 * on @ref Eina_List nodes.
 *
 * @warning You must @b always use the pointer to the first element of the list
 * as the list!
 * @warning You must @b never use a pointer to an element in the middle of the
 * list as the list!
 *
 * Here are some examples of @ref Eina_List usage:
 * @li @ref eina_list_01_example_page
 * @li @ref eina_list_02_example_page
 * @li @ref eina_list_03_example_page
 * @li @ref eina_list_04_example_page
 */

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
 * @defgroup Eina_List_Group List
 *
 * @{
 */

/**
 * @typedef Eina_List
 * Type for a generic double linked list.
 */
typedef struct _Eina_List            Eina_List;

/**
 * @typedef Eina_List_Accounting
 * Cache used to store the last element of a list and the number of
 * elements, for fast access.
 */
typedef struct _Eina_List_Accounting Eina_List_Accounting;

/**
 * @struct _Eina_List
 * Type for a generic double linked list.
 */
struct _Eina_List
{
   void                 *data; /**< Pointer to list element payload */
   Eina_List            *next; /**< Next member in the list */
   Eina_List            *prev; /**< Previous member in the list */
   Eina_List_Accounting *accounting; /**< Private list accounting info - don't touch */

   EINA_MAGIC
};

/**
 * @struct _Eina_List_Accounting
 * Cache used to store the last element of a list and the number of
 * elements, for fast access. It is for private used and must not be
 * touched.
 */
struct _Eina_List_Accounting
{
   Eina_List   *last; /**< Pointer to the last element of the list - don't touch */
   unsigned int count; /**< Number of elements of the list - don't touch */
   EINA_MAGIC
};


/**
 * @brief Append the given data to the given linked list.
 *
 * @param list The given list.
 * @param data The data to append.
 * @return A list pointer.
 *
 * This function appends @p data to @p list. If @p list is @c NULL, a
 * new list is returned. On success, a new list pointer that should be
 * used in place of the one given to this function is
 * returned. Otherwise, the old pointer is returned.
 *
 * The following example code demonstrates how to ensure that the
 * given data has been successfully appended.
 *
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 *
 * list = eina_list_append(list, my_data);
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list(or NULL).
 */
EAPI Eina_List            *eina_list_append(Eina_List *list, const void *data) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Prepends the given data to the given linked list.
 *
 * @param list The given list.
 * @param data The data to prepend.
 * @return A list pointer.
 *
 * This function prepends @p data to @p list. If @p list is @c NULL, a
 * new list is returned. On success, a new list pointer that should be
 * used in place of the one given to this function is
 * returned. Otherwise, the old pointer is returned.
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
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_prepend(Eina_List *list, const void *data) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Insert the given data into the given linked list after the specified data.
 *
 * @param list The given linked list.
 * @param data The data to insert.
 * @param relative The data to insert after.
 * @return A list pointer.
 *
 * This function inserts @p data to @p list after @p relative. If
 * @p relative is not in the list, @p data is appended to the end of
 * the list.  If @p list is @c NULL, a  new list is returned. If there
 * are multiple instances of @p relative in the list, @p data is
 * inserted after the first instance.On success, a new list pointer
 * that should be used in place of the one given to this function is
 * returned. Otherwise, the old pointer is returned.
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
 * list = eina_list_append_relative(list, my_data, relative_member);
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_append_relative(Eina_List *list, const void *data, const void *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Append a list node to a linked list after the specified member
 *
 * @param list The given linked list.
 * @param data The data to insert.
 * @param relative The list node to insert after.
 * @return A list pointer.
 *
 * This function inserts @p data to @p list after the list node
 * @p relative. If @p list or @p relative are @c NULL, @p data is just
 * appended to @p list using eina_list_append(). If @p list is
 * @c NULL, a  new list is returned. If there are multiple instances
 * of @p relative in the list, @p data is inserted after the first
 * instance. On success, a new list pointer that should be used in
 * place of the one given to this function is returned. Otherwise, the
 * old pointer is returned.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_append_relative_list(Eina_List *list, const void *data, Eina_List *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Prepend a data pointer to a linked list before the specified member
 *
 * @param list The given linked list.
 * @param data The data to insert.
 * @param relative The data to insert before.
 * @return A list pointer.
 *
 * This function inserts @p data to @p list before @p relative. If
 * @p relative is not in the list, @p data is prepended to the list
 * with eina_list_prepend(). If @p list is @c NULL, a  new list is
 * returned. If there are multiple instances of @p relative in the
 * list, @p data is inserted before the first instance. On success, a
 * new list pointer that should be used in place of the one given to
 * this function is returned. Otherwise, the old pointer is returned.
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
 * list = eina_list_prepend_relative(list, my_data, relative_member);
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_prepend_relative(Eina_List *list, const void *data, const void *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Prepend a list node to a linked list before the specified member
 *
 * @param list The given linked list.
 * @param data The data to insert.
 * @param relative The list node to insert before.
 * @return A list pointer.
 *
 * This function inserts @p data to @p list before the list node
 * @p relative. If @p list or @p relative are @c NULL, @p data is just
 * prepended to @p list using eina_list_prepend(). If @p list is
 * @c NULL, a  new list is returned. If there are multiple instances
 * of @p relative in the list, @p data is inserted before the first
 * instance. On success, a new list pointer that should be used in
 * place of the one given to this function is returned. Otherwise, the
 * old pointer is returned.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_prepend_relative_list(Eina_List *list, const void *data, Eina_List *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Insert a new node into a sorted list.
 *
 * @param list The given linked list, @b must be sorted.
 * @param func The function called for the sort.
 * @param data The data to insert sorted.
 * @return A list pointer.
 *
 * This function inserts values into a linked list assuming it was
 * sorted and the result will be sorted. If @p list is @c NULLL, a new
 * list is returned. On success, a new list pointer that should be
 * used in place of the one given to this function is
 * returned. Otherwise, the old pointer is returned.
 *
 * @note O(log2(n)) comparisons (calls to @p func) average/worst case
 * performance as it uses eina_list_search_sorted_near_list() and thus
 * is bounded to that. As said in eina_list_search_sorted_near_list(),
 * lists do not have O(1) access time, so walking to the correct node
 * can be costly, consider worst case to be almost O(n) pointer
 * dereference (list walk).
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_sorted_insert(Eina_List *list, Eina_Compare_Cb func, const void *data) EINA_ARG_NONNULL(2, 3) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Remove the first instance of the specified data from the given list.
 *
 * @param list The given list.
 * @param data The specified data.
 * @return A list pointer.
 *
 * This function removes the first instance of @p data from
 * @p list. If the specified data is not in the given list (this
 * includes the case where @p data is @c NULL), nothing is done and the
 * specified @p list returned. If @p list is @c NULL, @c NULL is returned,
 * otherwise a new list pointer that should be used in place of the one
 * passed to this function.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_remove(Eina_List *list, const void *data) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Remove the specified list node.
 *
 * @param list The given linked list.
 * @param remove_list The list node which is to be removed.
 * @return A list pointer.
 *
 * This function removes the list node @p remove_list from @p list and
 * frees the list node structure @p remove_list. If @p list is
 * @c NULL, this function returns @c NULL. If @p remove_list is
 * @c NULL, it returns @p list, otherwise, a new list pointer that
 * should be used in place of the one passed to this function.
 *
 * The following code gives an example (notice we use EINA_LIST_FOREACH
 * instead of EINA_LIST_FOREACH_SAFE because we stop the loop after
 * removing the current node).
 *
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 * extern void *my_data;
 * void *data
 *
 * EINA_LIST_FOREACH(list, l, data)
 *   {
 *      if (data == my_data)
 *        {
 *           list = eina_list_remove_list(list, l);
 *           break;
 *        }
 *   }
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_remove_list(Eina_List *list, Eina_List *remove_list) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Move the specified data to the head of the list.
 *
 * @param list The list handle to move the data.
 * @param move_list The list node to move.
 * @return A new list handle to replace the old one
 *
 * This function move @p move_list to the front of @p list. If list is
 * @c NULL, @c NULL is returned. If @p move_list is @c NULL,
 * @p list is returned. Otherwise, a new list pointer that should be
 * used in place of the one passed to this function.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 * extern void *my_data;
 * void *data;
 *
 * EINA_LIST_FOREACH(list, l, data)
 *   {
 *      if (data == my_data)
 *        {
 *           list = eina_list_promote_list(list, l);
 *           break;
 *        }
 *   }
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_promote_list(Eina_List *list, Eina_List *move_list) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Move the specified data to the tail of the list.
 *
 * @param list The list handle to move the data.
 * @param move_list The list node to move.
 * @return A new list handle to replace the old one
 *
 * This function move @p move_list to the back of @p list. If list is
 * @c NULL, @c NULL is returned. If @p move_list is @c NULL,
 * @p list is returned. Otherwise, a new list pointer that should be
 * used in place of the one passed to this function.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 * extern void *my_data;
 * void *data;
 *
 * EINA_LIST_FOREACH(list, l, data)
 *   {
 *      if (data == my_data)
 *        {
 *           list = eina_list_demote_list(list, l);
 *           break;
 *        }
 *   }
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_demote_list(Eina_List *list, Eina_List *move_list);


/**
 * @brief Find a member of a list and return the member.
 *
 * @param list The list to search for a data.
 * @param data The data pointer to find in the list.
 * @return The found member data pointer if found, @c NULL otherwise.
 *
 * This function searches in @p list from beginning to end for the
 * first member whose data pointer is @p data. If it is found, @p data
 * will be returned, otherwise @c NULL will be returned.
 *
 * Example:
 * @code
 * extern Eina_List *list;
 * extern void *my_data;
 *
 * if (eina_list_data_find(list, my_data) == my_data)
 *   {
 *      printf("Found member %p\n", my_data);
 *   }
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI void                 *eina_list_data_find(const Eina_List *list, const void *data) EINA_PURE EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Find a member of a list and return the list node containing that member.
 *
 * @param list The list to search for data.
 * @param data The data pointer to find in the list.
 * @return The found members list node on success, @c NULL otherwise.
 *
 * This function searches in @p list from beginning to end for the
 * first member whose data pointer is @p data. If it is found, the
 * list node containing the specified member is returned, otherwise
 * @c NULL is returned.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_data_find_list(const Eina_List *list, const void *data) EINA_PURE EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Move a data pointer from one list to another
 *
 * @param to The list to move the data to
 * @param from The list to move from
 * @param data The data to move
 * @return #EINA_TRUE on success, else #EINA_FALSE
 *
 * This function is a shortcut for doing the following:
 * to = eina_list_append(to, data);
 * from = eina_list_remove(from, data);
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_Bool             eina_list_move(Eina_List **to, Eina_List **from, void *data);

/**
 * @brief Move a list node from one list to another
 *
 * @param to The list to move the data to
 * @param from The list to move from
 * @param data The list node containing the data to move
 * @return #EINA_TRUE on success, else #EINA_FALSE
 *
 * This function is a shortcut for doing the following:
 * to = eina_list_append(to, data->data);
 * from = eina_list_remove_list(from, data);
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_Bool             eina_list_move_list(Eina_List **to, Eina_List **from, Eina_List *data);


/**
 * @brief Free an entire list and all the nodes, ignoring the data contained.

 * @param list The list to free
 * @return A @c NULL pointer
 *
 * This function frees all the nodes of @p list. It does not free the
 * data of the nodes. To free them, use #EINA_LIST_FREE.
 */
EAPI Eina_List            *eina_list_free(Eina_List *list);


/**
 * @brief Get the nth member's data pointer in a list.
 *
 * @param list The list to get the specified member number from.
 * @param n The number of the element (0 being the first).
 * @return The data pointer stored in the specified element.
 *
 * This function returns the data pointer of element number @p n, in
 * the @p list. The first element in the array is element number 0. If
 * the element number @p n does not exist, @c NULL is
 * returned. Otherwise, the data of the found element is returned.
 *
 * @note Worst case is O(n).
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI void                 *eina_list_nth(const Eina_List *list, unsigned int n) EINA_PURE EINA_WARN_UNUSED_RESULT;


/**
 * @brief Get the nth member's list node in a list.
 *
 * @param list The list to get the specfied member number from.
 * @param n The number of the element (0 being the first).
 * @return The list node stored in the numbered element.
 *
 * This function returns the list node of element number @p n, in
 * @p list. The first element in the array is element number 0. If the
 * element number @p n does not exist or @p list is @c NULL or @p n is
 * greater than the count of elements in @p list minus 1, @c NULL is
 * returned. Otherwise the list node stored in the numbered element is
 * returned.
 *
 * @note Worst case is O(n).
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_nth_list(const Eina_List *list, unsigned int n) EINA_PURE EINA_WARN_UNUSED_RESULT;


/**
 * @brief Reverse all the elements in the list.
 *
 * @param list The list to reverse.
 * @return The list head after it has been reversed.
 *
 * This function reverses the order of all elements in @p list, so the
 * last member is now first, and so on. If @p list is @c NULL, this
 * functon returns @c NULL.
 *
 * @note @b in-place: this will change the given list, so you should
 * now point to the new list head that is returned by this function.
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_reverse_clone()
 * @see eina_list_iterator_reversed_new()
 */
EAPI Eina_List            *eina_list_reverse(Eina_List *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Clone (copy) all the elements in the list in reverse order.
 *
 * @param list The list to reverse.
 * @return The new list that has been reversed.
 *
 * This function reverses the order of all elements in @p list, so the
 * last member is now first, and so on. If @p list is @c NULL, this
 * functon returns @c NULL. This returns a copy of the given list.
 *
 * @note @b copy: this will copy the list and you should then
 * eina_list_free() when it is not required anymore.
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_reverse()
 * @see eina_list_clone()
 */
EAPI Eina_List            *eina_list_reverse_clone(const Eina_List *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Clone (copy) all the elements in the list in exactly same order.
 *
 * @param list The list to clone.
 * @return The new list that has been cloned.
 *
 * This function clone in order of all elements in @p list. If @p list
 * is @c NULL, this functon returns @c NULL. This returns a copy of
 * the given list.
 *
 * @note @b copy: this will copy the list and you should then
 * eina_list_free() when it is not required anymore.
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_reverse_clone()
 */
EAPI Eina_List            *eina_list_clone(const Eina_List *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Sort a list according to the ordering func will return.
 *
 * @param list The list handle to sort.
 * @param limit The maximum number of list elements to sort.
 * @param func A function pointer that can handle comparing the list data
 * nodes.
 * @return the new head of list.
 *
 * This function sorts @p list. If @p limit is 0 or greater than the number of
 * elements in @p list, all the elements are sorted. @p func is used to
 * compare two elements of @p list. If @p func is @c NULL, this function returns
 * @p list.
 *
 * @note @b in-place: this will change the given list, so you should
 * now point to the new list head that is returned by this function.
 *
 * @note Worst case is O(n * log2(n)) comparisons (calls to func()).
 * That means that for 1,000,000 list sort will do 20,000,000 comparisons.
 *
 * Example:
 * @code
 * int
 * sort_cb(const void *d1, const void *d2)
 * {
 *    const char *txt = d1;
 *    const char *txt2 = d2;
 *
 *    if(!txt) return(1);
 *    if(!txt2) return(-1);
 *
 *    return(strcmp(txt, txt2));
 * }
 * extern Eina_List *list;
 *
 * list = eina_list_sort(list, eina_list_count(list), sort_cb);
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_sort(Eina_List *list, unsigned int limit, Eina_Compare_Cb func) EINA_ARG_NONNULL(3) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Shuffle list.
 *
 * @param list The list handle to shuffle.
 * @param func A function pointer that can return an int between 2 inclusives values
 * @return the new head of list.
 *
 * This function shuffles @p list.
 * @p func is used to generate random list indexes within the range of
 * unshuffled list items. If @p func is @c NULL, rand is used.
 *
 * @note @b in-place: this will change the given list, so you should
 * now point to the new list head that is returned by this function.
 *
 * @since 1.8
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_shuffle(Eina_List *list, Eina_Random_Cb func) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Merge two list.
 *
 * @param left Head list to merge.
 * @param right Tail list to merge.
 * @return A new merged list.
 *
 * This function puts right at the end of left and returns the head.
 *
 * Both left and right do not exist anymore after the merge.
 *
 * @note merge cost is O(n), being @b n the size of the smallest
 * list. This is due the need to fix accounting of that segment,
 * making count and last access O(1).
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_merge(Eina_List *left, Eina_List *right) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Merge two sorted list according to the ordering func will return.
 *
 * @param left First list to merge.
 * @param right Second list to merge.
 * @param func A function pointer that can handle comparing the list data
 * nodes.
 * @return A new sorted list.
 *
 * This function compares the head of @p left and @p right, and choose the
 * smallest one to be head of the returned list. It will continue this process
 * for all entry of both list.
 *
 * Both left and right lists are not vailid anymore after the merge and should
 * not be used. If @p func is @c NULL, it will return @c NULL.
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
 * extern Eina_List *sorted1;
 * extern Eina_List *sorted2;
 *
 * list = eina_list_sorted_merge(sorted1, sorted2, sort_cb);
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_sorted_merge(Eina_List *left, Eina_List *right, Eina_Compare_Cb func) EINA_ARG_NONNULL(3) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Split a list into 2 lists.
 *
 * @param list List to split.
 * @param relative The list will be split after @p relative.
 * @param right The head of the new right list.
 * @return The new left list
 *
 * This function splits @p list into two lists ( left and right ) after the node @p relative. @p Relative
 * will become the last node of the left list. If @p list or @p right are @c NULL list is returns.
 * If @p relative is NULL right is set to @p list and @c NULL is returns.
 * If @p relative is the last node of @p list list is returns and @p right is set to @c NULL.
 *
 * list does not exist anymore after the split.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_List            *eina_list_split_list(Eina_List *list, Eina_List *relative, Eina_List **right) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returns node nearest to data is in the sorted list.
 *
 * @param list The list to search for data, @b must be sorted.
 * @param func A function pointer that can handle comparing the list data nodes.
 * @param data reference value to search.
 * @param result_cmp if provided returns the result of
 * func(node->data, data) node being the last (returned) node. If node
 * was found (exact match), then it is 0. If returned node is smaller
 * than requested data, it is less than 0 and if it's bigger it's
 * greater than 0. It is the last value returned by func().
 * @return the nearest node, @c NULL if not found.
 *
 * This function searches for a node containing @p data as it's data in @p list,
 * if such a node exists it will be returned and @p result_cmp will be @p 0. If
 * the data of no node in @p list is equal to @p data, the node with the nearest
 * value to that will be returned and @p result_cmp will be the return value of
 * @p func with @p data and the returned node's data as arguments.
 * 
 * This function is useful for inserting an element in the list only in case it
 * isn't already present in the list, the naive way of doing this would be:
 * @code
 * void *ptr = eina_list_data_find(list, "my data");
 * if (!ptr)
 *   eina_list_sorted_insert(list, "my data");
 * @endcode
 * 
 * However this has the downside of walking through the list twice, once to
 * check if the data is already present and another to insert the element in the
 * corret position. This can be done more eficiently:
 * @code
 * int cmp_result;
 * l = eina_list_search_sorted_near_list(list, cmp_func, "my data",
 *                                       &cmp_result);
 * if (cmp_result > 0)
 *   list = eina_list_prepend_relative_list(list, "my data", l);
 * else if (cmp_result < 0)
 *   list = eina_list_append_relative_list(list, "my data", l);
 * @endcode
 * 
 * If @a cmp_result is 0 the element is already in the list and we need not
 * insert it, if @a cmp_result is greater than zero @a "my @a data" needs to
 * come after @a l(the nearest node present), if less than zero before.
 *
 * @note O(log2(n)) average/worst case performance, for 1,000,000
 * elements it will do a maximum of 20 comparisons. This is much
 * faster than the 1,000,000 comparisons made naively walking the list
 * from head to tail, so depending on the number of searches and
 * insertions, it may be worth to eina_list_sort() the list and do the
 * searches later. As lists do not have O(1) access time, walking to
 * the correct node can be costly, consider worst case to be almost
 * O(n) pointer dereference (list walk).
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_search_sorted_list()
 * @see eina_list_sort()
 * @see eina_list_sorted_merge()
 */
EAPI Eina_List            *eina_list_search_sorted_near_list(const Eina_List *list, Eina_Compare_Cb func, const void *data, int *result_cmp);


/**
 * @brief Returns node if data is in the sorted list.
 *
 * @param list The list to search for data, @b must be sorted.
 * @param func A function pointer that can handle comparing the list data nodes.
 * @param data reference value to search.
 * @return the node if func(node->data, data) == 0, @c NULL if not found.
 *
 * This can be used to check if some value is inside the list and get
 * the container node in this case. It should be used when list is
 * known to be sorted as it will do binary search for results.
 *
 * Example: imagine user gives a string, you check if it's in the list
 * before duplicating its contents.
 *
 * @note O(log2(n)) average/worst case performance, for 1,000,000
 * elements it will do a maximum of 20 comparisons. This is much
 * faster than the 1,000,000 comparisons made by
 * eina_list_search_unsorted_list(), so depending on the number of
 * searches and insertions, it may be worth to eina_list_sort() the
 * list and do the searches later. As said in
 * eina_list_search_sorted_near_list(), lists do not have O(1) access
 * time, so walking to the correct node can be costly, consider worst
 * case to be almost O(n) pointer dereference (list walk).
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_search_sorted()
 * @see eina_list_sort()
 * @see eina_list_sorted_merge()
 * @see eina_list_search_unsorted_list()
 * @see eina_list_search_sorted_near_list()
 */
EAPI Eina_List            *eina_list_search_sorted_list(const Eina_List *list, Eina_Compare_Cb func, const void *data);


/**
 * @brief Returns node data if it is in the sorted list.
 *
 * @param list The list to search for data, @b must be sorted.
 * @param func A function pointer that can handle comparing the list data nodes.
 * @param data reference value to search.
 * @return the node value (@c node->data) if func(node->data, data) == 0,
 * NULL if not found.
 *
 * This can be used to check if some value is inside the list and get
 * the existing instance in this case. It should be used when list is
 * known to be sorted as it will do binary search for results.
 *
 * Example: imagine user gives a string, you check if it's in the list
 * before duplicating its contents.
 *
 * @note O(log2(n)) average/worst case performance, for 1,000,000
 * elements it will do a maximum of 20 comparisons. This is much
 * faster than the 1,000,000 comparisons made by
 * eina_list_search_unsorted(), so depending on the number of
 * searches and insertions, it may be worth to eina_list_sort() the
 * list and do the searches later. As said in
 * eina_list_search_sorted_near_list(), lists do not have O(1) access
 * time, so walking to the correct node can be costly, consider worst
 * case to be almost O(n) pointer dereference (list walk).
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_search_sorted_list()
 * @see eina_list_sort()
 * @see eina_list_sorted_merge()
 * @see eina_list_search_unsorted_list()
 */
EAPI void                 *eina_list_search_sorted(const Eina_List *list, Eina_Compare_Cb func, const void *data);


/**
 * @brief Returns node if data is in the unsorted list.
 *
 * @param list The list to search for data, may be unsorted.
 * @param func A function pointer that can handle comparing the list data nodes.
 * @param data reference value to search.
 * @return the node if func(node->data, data) == 0, @c NULL if not found.
 *
 * This can be used to check if some value is inside the list and get
 * the container node in this case.
 *
 * Example: imagine user gives a string, you check if it's in the list
 * before duplicating its contents.
 *
 * @note this is expensive and may walk the whole list, it's order-N,
 * that is for 1,000,000 elements list it may walk and compare
 * 1,000,000 nodes.
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_search_sorted_list()
 * @see eina_list_search_unsorted()
 */
EAPI Eina_List            *eina_list_search_unsorted_list(const Eina_List *list, Eina_Compare_Cb func, const void *data);


/**
 * @brief Returns node data if it is in the unsorted list.
 *
 * @param list The list to search for data, may be unsorted.
 * @param func A function pointer that can handle comparing the list data nodes.
 * @param data reference value to search.
 * @return the node value (@c node->data) if func(node->data, data) == 0,
 * @c NULL if not found.
 *
 * This can be used to check if some value is inside the list and get
 * the existing instance in this case.
 *
 * Example: imagine user gives a string, you check if it's in the list
 * before duplicating its contents.
 *
 * @note this is expensive and may walk the whole list, it's order-N,
 * that is for 1,000,000 elements list it may walk and compare
 * 1,000,000 nodes.
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_search_sorted()
 * @see eina_list_search_unsorted_list()
 */
EAPI void                 *eina_list_search_unsorted(const Eina_List *list, Eina_Compare_Cb func, const void *data);

/**
 * @brief Get the last list node in the list.
 *
 * @param list The list to get the last list node from.
 * @return The last list node in the list.
 *
 * This function returns the last list node in the list @p list. If
 * @p list is @c NULL or empty, @c NULL is returned.
 *
 * This is a order-1 operation (it takes the same short time
 * regardless of the length of the list).
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
static inline Eina_List   *eina_list_last(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the next list node after the specified list node.
 *
 * @param list The list node to get the next list node from
 * @return The next list node on success, @c NULL otherwise.
 *
 * This function returns the next list node after the current one in
 * @p list. It is equivalent to list->next. If @p list is @c NULL or
 * if no next list node exists, it returns @c NULL.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
static inline Eina_List   *eina_list_next(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the previous list node before the specified list node.
 *
 * @param list The list node to get the previous list node from.
 * @return The previous list node o success, @c NULL otherwise.
 * if no previous list node exists
 *
 * This function returns the previous list node before the current one
 * in @p list. It is equivalent to list->prev. If @p list is @c NULL or
 * if no previous list node exists, it returns @c NULL.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
static inline Eina_List   *eina_list_prev(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the list node data member.
 *
 * @param list The list node to get the data member of.
 * @return The data member from the list node.
 *
 * This function returns the data member of the specified list node @p
 * list. It is equivalent to list->data. If @p list is @c NULL, this
 * function returns @c NULL.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
static inline void        *eina_list_data_get(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set the list node data member.
 *
 * @param list The list node to get the data member of.
 * @param data The data member to the list node.
 * @return The previous data value.
 *
 * This function set the data member @p data of the specified list node
 * @p list. It returns the previous data of the node. If @p list is
 * @c NULL, this function returns @c NULL.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
static inline void        *eina_list_data_set(Eina_List *list, const void *data);

/**
 * @brief Get the count of the number of items in a list.
 *
 * @param list The list whose count to return.
 * @return The number of members in the list.
 *
 * This function returns how many members @p list contains. If the
 * list is @c NULL, @c 0 is returned.
 *
 * NB: This is an order-1 operation and takes the same time regardless
 * of the length of the list.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
static inline unsigned int eina_list_count(const Eina_List *list) EINA_PURE;


/**
 * @brief Returned a new iterator associated to a list.
 *
 * @param list The list.
 * @return A new iterator.
 *
 * This function returns a newly allocated iterator associated to @p
 * list. If @p list is @c NULL or the count member of @p list is less
 * or equal than 0, this function still returns a valid iterator that
 * will always return false on eina_iterator_next(), thus keeping API
 * sane.
 *
 * If the memory can not be allocated, NULL is returned.
 * Otherwise, a valid iterator is returned.
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @warning if the list structure changes then the iterator becomes
 *    invalid! That is, if you add or remove nodes this iterator
 *    behavior is undefined and your program may crash!
 */
EAPI Eina_Iterator        *eina_list_iterator_new(const Eina_List *list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returned a new reversed iterator associated to a list.
 *
 * @param list The list.
 * @return A new iterator.
 *
 * This function returns a newly allocated iterator associated to @p
 * list. If @p list is @c NULL or the count member of @p list is less
 * or equal than 0, this function still returns a valid iterator that
 * will always return false on eina_iterator_next(), thus keeping API
 * sane.
 *
 * Unlike eina_list_iterator_new(), this will walk the list backwards.
 *
 * If the memory can not be allocated, NULL is returned.
 * Otherwise, a valid iterator is returned.
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @warning if the list structure changes then the iterator becomes
 *    invalid! That is, if you add or remove nodes this iterator
 *    behavior is undefined and your program may crash!
 */
EAPI Eina_Iterator        *eina_list_iterator_reversed_new(const Eina_List *list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returned a new accessor associated to a list.
 *
 * @param list The list.
 * @return A new accessor.
 *
 * This function returns a newly allocated accessor associated to
 * @p list. If @p list is @c NULL or the count member of @p list is
 * less or equal than 0, this function returns @c NULL. If the memory can
 * not be allocated, @c NULL is returned Otherwise, a valid accessor is
 * returned.
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
EAPI Eina_Accessor        *eina_list_accessor_new(const Eina_List *list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @def EINA_LIST_FOREACH
 * @brief Macro to iterate over a list.
 *
 * @param list The list to iterate over.
 * @param l A list that is used as an iterator and points to the current node.
 * @param data Current item's data.
 *
 * This macro iterates over @p list from the first element to
 * the last. @p data is the data related to the current element.
 * @p l is an #Eina_List used as the list iterator.
 *
 * The following diagram illustrates this macro iterating over a list of four
 * elements("one", "two", "three" and "four"):
 * @htmlonly
 * <img src="eina-list-foreach.png" style="max-width: 100%;" />
 * <a href="eina-list-foreach.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-foreach.eps width=\textwidth
 *
 * It can be used to free list data, as in the following example:
 *
 * @code
 * Eina_List *list;
 * Eina_List *l;
 * char      *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings,
 * // EINA_LIST_FOREACH will be used to free those strings
 *
 * EINA_LIST_FOREACH(list, l, data)
 *   free(data);
 * eina_list_free(list);
 * @endcode
 *
 * @note This is not the optimal way to release memory allocated to
 *       a list, since it iterates over the list twice.
 *       For an optimized algorithm, use EINA_LIST_FREE().
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @warning Be careful when deleting list nodes.
 *          If you remove the current node and continue iterating,
 *          the code will fail because the macro will not be able
 *          to get the next node. Notice that it's OK to remove any
 *          node if you stop the loop after that.
 *          For destructive operations such as this, consider
 *          using EINA_LIST_FOREACH_SAFE().
 */
#define EINA_LIST_FOREACH(list, l, data) \
  for (l = list,                         \
       data = eina_list_data_get(l);     \
       l;                                \
       l = eina_list_next(l),            \
       data = eina_list_data_get(l))

/**
 * @def EINA_LIST_REVERSE_FOREACH
 * @brief Macro to iterate over a list in the reverse order.
 *
 * @param list The list to iterate over.
 * @param l A list that is used as an iterator and points to the current node.
 * @param data Current item's data.
 *
 * This macro works like EINA_LIST_FOREACH, but iterates from the
 * last element of a list to the first.
 * @p data is the data related to the current element, while @p l
 * is an #Eina_List that is used as the list iterator.
 *
 * The following diagram illustrates this macro iterating over a list of four
 * elements("one", "two", "three" and "four"):
 * @htmlonly
 * <img src="eina-list-reverse-foreach.png" style="max-width: 100%;" />
 * <a href="eina-list-reverse-foreach.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-reverse-foreach.eps width=\textwidth
 *
 * It can be used to free list data, as in the following example:
 *
 * @code
 * Eina_List *list;
 * Eina_List *l;
 * char      *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings,
 * // EINA_LIST_REVERSE_FOREACH will be used to free those strings
 *
 * EINA_LIST_REVERSE_FOREACH(list, l, data)
 *   free(data);
 * eina_list_free(list);
 * @endcode
 *
 * @note This is not the optimal way to release memory allocated to
 *       a list, since it iterates over the list twice.
 *       For an optimized algorithm, use EINA_LIST_FREE().
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @warning Be careful when deleting list nodes.
 *          If you remove the current node and continue iterating,
 *          the code will fail because the macro will not be able
 *          to get the next node. Notice that it's OK to remove any
 *          node if you stop the loop after that.
 *          For destructive operations such as this, consider
 *          using EINA_LIST_REVERSE_FOREACH_SAFE().
 */
#define EINA_LIST_REVERSE_FOREACH(list, l, data) \
  for (l = eina_list_last(list),                 \
       data = eina_list_data_get(l);             \
       l;                                        \
       l = eina_list_prev(l),                    \
       data = eina_list_data_get(l))

/**
 * @def EINA_LIST_FOREACH_SAFE
 * @brief Macro to iterate over a list with support for node deletion.
 *
 * @param list The list to iterate over.
 * @param l A list that is used as an iterator and points to the current node.
 * @param l_next A list that is used as an iterator and points to the next node.
 * @param data Current item's data.
 *
 * This macro iterates over @p list from the first element to
 * the last. @p data is the data related to the current element.
 * @p l is an #Eina_List used as the list iterator.
 *
 * Since this macro stores a pointer to the next list node in @p l_next,
 * deleting the current node and continuing looping is safe.
 *
 * The following diagram illustrates this macro iterating over a list of four
 * elements("one", "two", "three" and "four"):
 * @htmlonly
 * <img src="eina-list-foreach-safe.png" style="max-width: 100%;" />
 * <a href="eina-list-foreach-safe.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-foreach-safe.eps width=\textwidth
 *
 * This macro can be used to free list nodes, as in the following example:
 *
 * @code
 * Eina_List *list;
 * Eina_List *l;
 * Eina_List *l_next;
 * char      *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings,
 * // EINA_LIST_FOREACH_SAFE will be used to free elements that match "key".
 *
 * EINA_LIST_FOREACH_SAFE(list, l, l_next, data)
 *   if (strcmp(data, "key") == 0)
 *     {
 *        free(data);
 *        list = eina_list_remove_list(list, l);
 *     }
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
#define EINA_LIST_FOREACH_SAFE(list, l, l_next, data) \
  for (l = list,                                      \
       l_next = eina_list_next(l),                    \
       data = eina_list_data_get(l);                  \
       l;                                             \
       l = l_next,                                    \
       l_next = eina_list_next(l),                    \
       data = eina_list_data_get(l))

/**
 * @def EINA_LIST_REVERSE_FOREACH_SAFE
 * @brief Macro to iterate over a list in the reverse order with support
 *        for deletion.
 *
 * @param list The list to iterate over.
 * @param l A list that is used as an iterator and points to the current node.
 * @param l_prev A list that is used as an iterator and points to the previous node.
 * @param data Current item's data.
 *
 * This macro works like EINA_LIST_FOREACH_SAFE, but iterates from the
 * last element of a list to the first.
 * @p data is the data related to the current element, while @p l
 * is an #Eina_List that is used as the list iterator.
 *
 * Since this macro stores a pointer to the previous list node in @p l_prev,
 * deleting the current node and continuing looping is safe.
 *
 * The following diagram illustrates this macro iterating over a list of four
 * elements("one", "two", "three" and "four"):
 * @htmlonly
 * <img src="eina-list-reverse-foreach-safe.png" style="max-width: 100%;" />
 * <a href="eina-list-reverse-foreach-safe.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-reverse-foreach-safe.eps width=\textwidth
 *
 * This macro can be used to free list nodes, as in the following example:
 *
 * @code
 * Eina_List *list;
 * Eina_List *l;
 * Eina_List *l_prev;
 * char       *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings,
 * // EINA_LIST_REVERSE_FOREACH_SAFE will be used to free elements that match "key".
 *
 * EINA_LIST_REVERSE_FOREACH_SAFE(list, l, l_prev, data)
 *   if (strcmp(data, "key") == 0)
 *     {
 *        free(data);
 *        list = eina_list_remove_list(list, l);
 *     }
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 */
#define EINA_LIST_REVERSE_FOREACH_SAFE(list, l, l_prev, data) \
  for (l = eina_list_last(list),                              \
       l_prev = eina_list_prev(l),                            \
       data = eina_list_data_get(l);                          \
       l;                                                     \
       l = l_prev,                                            \
       l_prev = eina_list_prev(l),                            \
       data = eina_list_data_get(l))

/**
 * @def EINA_LIST_FREE
 * @brief Macro to remove each list node while having access to each node's data.
 *
 * @param list The list that will be cleared.
 * @param data Current node's data.
 *
 * This macro will call #eina_list_remove_list for each list node, and store
 * the data contained in the current node in @p data.
 *
 * The following diagram illustrates this macro iterating over a list of four
 * elements("one", "two", "three" and "four"):
 * @htmlonly
 * <img src="eina-list-free.png" style="max-width: 100%;" />
 * <a href="eina-list-free.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-free.eps width=\textwidth
 *
 * If you do not need to release node data, it is easier to call #eina_list_free().
 *
 * @code
 * Eina_List *list;
 * char      *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings,
 *
 * EINA_LIST_FREE(list, data)
 *   free(data);
 * @endcode
 *
 * @warning @p list must be a pointer to the first element of the list.
 *
 * @see eina_list_free()
 */
#define EINA_LIST_FREE(list, data)               \
  for (data = eina_list_data_get(list);          \
       list;                                     \
       list = eina_list_remove_list(list, list), \
       data = eina_list_data_get(list))

#include "eina_inline_list.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_LIST_H_ */
