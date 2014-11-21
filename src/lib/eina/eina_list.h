/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
 * @defgroup Eina_List_Group List
 * @ingroup Eina_Containers_Group
 *
 * @brief This group discusses the functions that provide double linked list management.
 *
 * @remarks Eina_List is a doubly linked list. It can store data of any type in the
 *          form of void pointers. It has convenience functions to do all the common
 *          operations, which means it should rarely, if ever, be necessary to directly
 *          access the struct's fields. Nevertheless it can be useful to understand the
 *          inner workings of the data structure being used.
 *
 * @remarks @ref Eina_List nodes keep references to the previous node, the next node, its
 *          data, and to an accounting structure.
 *
 * @image html eina_list.png
 * @htmlonly
 * <a href="eina_list.png">Full-size</a>
 * @endhtmlonly
 * @image rtf eina_list.png
 * @image latex eina_list.eps width=5cm
 *
 * @remarks  @ref Eina_List_Accounting is used to improve the performance of some
 *           functions. It is private and <b>should not</b> be modified. It contains a
 *           reference to the end of the list and the number of elements in the list.
 *
 * @remarks  Every function that modifies the contents of the list returns a pointer
 *           to the head of the list and it is essential that this pointer be used in
 *           any future references to the list.
 *
 * @remarks Most functions have two versions that have the same effect but operate on
 *          different arguments, the @a plain functions operate over data(eg.:
 *          @ref eina_list_append_relative, @ref eina_list_remove,
 *          @ref eina_list_data_find), the @a list versions of these functions operate
 *          on @ref Eina_List nodes.
 *
 * @remarks You must @b always use the pointer to the first element of the list,
 *          as the list.
 * @remarks You must @b never use a pointer to an element in the middle of the
 *          list, as the list.
 *
 * @{
 */

/**
 * @typedef Eina_List
 * @brief The structure type for a generic double linked list.
 */
typedef struct _Eina_List            Eina_List;

/**
 * @typedef Eina_List_Accounting
 * @brief The structure type of the cache used to store the last element of a list and the number of
 *        elements, for fast access.
 */
typedef struct _Eina_List_Accounting Eina_List_Accounting;

/**
 * @struct _Eina_List
 * @brief The structure type for a generic double linked list.
 */
struct _Eina_List
{
   void                 *data; /**< Pointer to the list element payload */
   Eina_List            *next; /**< Next member in the list */
   Eina_List            *prev; /**< Previous member in the list */
   Eina_List_Accounting *accounting; /**< Private list accounting info - don't touch */

   EINA_MAGIC
};

/**
 * @struct _Eina_List_Accounting
 * @brief The structure type for the cache used to store the last element of a list and the number of
 *        elements, for fast access. It is for private use and must not be
 *        touched.
 */
struct _Eina_List_Accounting
{
   Eina_List   *last; /**< Pointer to the last element of the list - don't touch */
   unsigned int count; /**< Number of elements in the list - don't touch */
   EINA_MAGIC
};


/**
 * @brief Appends the given data to the given linked list.
 *
 * @details This function appends @a data to @a list. If @a list is @c NULL, a
 *          new list is returned. On success, a new list pointer that should be
 *          used in place of the one given to this function is
 *          returned. Otherwise, the old pointer is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The following example code demonstrates how to ensure that the
 *          given data has been successfully appended.
 *
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 *
 * list = eina_list_append(list, my_data);
 * if (eina_error_get())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list(or NULL).
 *
 * @param[in] list The given list
 * @param[in] data The data to append
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_append(Eina_List *list, const void *data) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Prepends the given data to the given linked list.
 *
 * @details This function prepends @a data to @a list. If @a list is @c NULL, a
 *          new list is returned. On success, a new list pointer that should be
 *          used in place of the one given to this function is
 *          returned. Otherwise, the old pointer is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The following example code demonstrates how to ensure that the
 *          given data has been successfully prepended.
 *
 * Example:
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 *
 * list = eina_list_prepend(list, my_data);
 * if (eina_error_get())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given list
 * @param[in] data The data to prepend.
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_prepend(Eina_List *list, const void *data) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Inserts the given data into the given linked list after the specified data.
 *
 * @details This function inserts @a data to @a list after @a relative. If
 *          @a relative is not in the list, @a data is appended to the end of
 *          the list. If @a list is @c NULL, a new list is returned. If there
 *          are multiple instances of @a relative in the list, @a data is
 *          inserted after the first instance.On success, a new list pointer
 *          that should be used in place of the one given to this function is
 *          returned. Otherwise, the old pointer is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The following example code demonstrates how to ensure that the
 *          given data has been successfully inserted.
 *
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 * extern void *relative_member;
 *
 * list = eina_list_append(list, relative_member);
 * if (eina_error_get())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * list = eina_list_append_relative(list, my_data, relative_member);
 * if (eina_error_get())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given linked list
 * @param[in] data The data to insert
 * @param[in] relative The data to insert after
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_append_relative(Eina_List *list, const void *data, const void *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Appends a list node to a linked list after the specified member.
 *
 * @details This function inserts @a data to @a list after the list node
 *          @a relative. If @a list or @a relative is @c NULL, @a data is just
 *          appended to @a list using eina_list_append(). If @a list is
 *          @c NULL, a  new list is returned. If there are multiple instances
 *          of @a relative in the list, @a data is inserted after the first
 *          instance. On success, a new list pointer that should be used in
 *          place of the one given to this function is returned. Otherwise, the
 *          old pointer is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given linked list
 * @param[in] data The data to insert
 * @param[in] relative The list node to insert after
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_append_relative_list(Eina_List *list, const void *data, Eina_List *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Prepends a data pointer to a linked list before the specified member.
 *
 * @details This function inserts @a data to @a list before @a relative. If
 *          @a relative is not in the list, @a data is prepended to the list
 *          with eina_list_prepend(). If @a list is @c NULL, a  new list is
 *          returned. If there are multiple instances of @a relative in the
 *          list, @a data is inserted before the first instance. On success, a
 *          new list pointer that should be used in place of the one given to
 *          this function is returned. Otherwise, the old pointer is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The following code example demonstrates how to ensure that the
 *          given data has been successfully inserted.
 *
 * @code
 * Eina_List *list = NULL;
 * extern void *my_data;
 * extern void *relative_member;
 *
 * list = eina_list_append(list, relative_member);
 * if (eina_error_get_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * list = eina_list_prepend_relative(list, my_data, relative_member);
 * if (eina_error_get())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
 *     exit(-1);
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given linked list
 * @param[in] data The data to insert
 * @param[in] relative The data to insert before
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_prepend_relative(Eina_List *list, const void *data, const void *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Prepends a list node to a linked list before the specified member.
 *
 * @details This function inserts @a data to @a list before the list node
 *          @a relative. If @a list or @a relative is @c NULL, @a data is just
 *          prepended to @a list using eina_list_prepend(). If @a list is
 *          @c NULL, a new list is returned. If there are multiple instances
 *          of @a relative in the list, @a data is inserted before the first
 *          instance. On success, a new list pointer that should be used in
 *          place of the one given to this function is returned. Otherwise, the
 *          old pointer is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given linked list
 * @param[in] data The data to insert
 * @param[in] relative The list node to insert before
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_prepend_relative_list(Eina_List *list, const void *data, Eina_List *relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Inserts a new node into a sorted list.
 *
 * @details This function inserts values into a linked list assuming it is
 *          sorted and the result is sorted. If @a list is @c NULLL, a new
 *          list is returned. On success, a new list pointer that should be
 *          used in place of the one given to this function is
 *          returned. Otherwise, the old pointer is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks Average/worst case performance is O(log2(n)) comparisons (calls to @a func)
 *          as it uses eina_list_search_sorted_near_list() and thus
 *          is bounded to that. As said in eina_list_search_sorted_near_list(),
 *          lists do not have O(1) access time, so walking to the correct node
 *          can be costly, consider worst case to be almost O(n) pointer
 *          dereference (list walk).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given linked list, @b must be sorted
 * @param[in] func The function called for the sort
 * @param[in] data The data to insert in the sorted list
 * @return A list pointer
 *
 * @see eina_error_get()
 */
EAPI Eina_List            *eina_list_sorted_insert(Eina_List *list, Eina_Compare_Cb func, const void *data) EINA_ARG_NONNULL(2, 3) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Removes the first instance of the specified data from the given list.
 *
 * @details This function removes the first instance of @a data from
 *          @a list. If the specified data is not in the given list (this
 *          includes the case where @a data is @c NULL), nothing is done and the
 *          specified @a list is returned. If @a list is @c NULL, @c NULL is returned,
 *          otherwise a new list pointer that should be used in place of the one
 *          passed to this function is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given list
 * @param[in] data The specified data
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_remove(Eina_List *list, const void *data) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Removes the specified list node.
 *
 * @details This function removes the list node @a remove_list from @a list and
 *          frees the list node structure @a remove_list. If @a list is
 *          @c NULL, this function returns @c NULL. If @a remove_list is
 *          @c NULL, it returns @a list, otherwise, a new list pointer that
 *          should be used in place of the one passed to this function is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The following code gives an example (notice we use EINA_LIST_FOREACH
 *          instead of EINA_LIST_FOREACH_SAFE because we stop the loop after
 *          removing the current node).
 *
 * @code
 * extern Eina_List *list;
 * Eina_List *l;
 * extern void *my_data;
 * void *data
 *
 * EINA_LIST_FOREACH(list, l, data)
 *   {
 *     if (data == my_data)
 *       {
 *         list = eina_list_remove_list(list, l);
 *         break;
 *       }
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The given linked list
 * @param[in] remove_list The list node to be removed
 * @return A list pointer
 *
 */
EAPI Eina_List            *eina_list_remove_list(Eina_List *list, Eina_List *remove_list) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Moves the specified data to the head of the list.
 *
 * @details This function moves @a move_list to the front of @a list. If the list is
 *          @c NULL, @c NULL is returned. If @a move_list is @c NULL,
 *          @a list is returned. Otherwise, a new list pointer that should be
 *          used in place of the one passed to this function is returned.
 *
 * @since_tizen 2.3
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
 *     if (data == my_data)
 *       {
 *         list = eina_list_promote_list(list, l);
 *         break;
 *       }
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list handle to move the data
 * @param[in] move_list The list node to move
 * @return A new list handle to replace the old one
 *
 */
EAPI Eina_List            *eina_list_promote_list(Eina_List *list, Eina_List *move_list) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Moves the specified data to the tail of the list.
 *
 * @details This function moves @a move_list to the back of @a list. If the list is
 *          @c NULL, @c NULL is returned. If @a move_list is @c NULL,
 *          @a list is returned. Otherwise, a new list pointer that should be
 *          used in place of the one passed to this function is returned.
 *
 * @since_tizen 2.3
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
 *     if (data == my_data)
 *       {
 *         list = eina_list_demote_list(list, l);
 *         break;
 *       }
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list handle to move the data
 * @param[in] move_list The list node to move
 * @return A new list handle to replace the old one
 *
 */
EAPI Eina_List            *eina_list_demote_list(Eina_List *list, Eina_List *move_list);


/**
 * @brief Finds a member of a list and returns the member.
 *
 * @details This function searches in @a list from beginning to end for the
 *          first member whose data pointer is @a data. If it is found, @a data
 *          is returned, otherwise @c NULL is returned.
 *
 * @since_tizen 2.3
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
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to search for data
 * @param[in] data The data pointer to find in the list
 * @return The found member data pointer, otherwise @c NULL
 *
 */
EAPI void                 *eina_list_data_find(const Eina_List *list, const void *data) EINA_PURE EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Finds a member of a list and returns the list node containing that member.
 *
 * @details This function searches in @a list from beginning to end for the
 *          first member whose data pointer is @a data. If it is found, the
 *          list node containing the specified member is returned, otherwise
 *          @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to search for data
 * @param[in] data The data pointer to find in the list
 * @return The found members list node on success, otherwise @c NULL
 *
 */
EAPI Eina_List            *eina_list_data_find_list(const Eina_List *list, const void *data) EINA_PURE EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Moves a data pointer from one list to another.
 *
 * @details This function is a shortcut for doing the following:
 *          to = eina_list_append(to, data);
 *          from = eina_list_remove(from, data);
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[out] to The list to move the data to
 * @param[out] from The list to move from
 * @param[in] data The data to move
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool             eina_list_move(Eina_List **to, Eina_List **from, void *data);

/**
 * @brief Moves a list node from one list to another.
 *
 * @details This function is a shortcut for doing the following:
 *          to = eina_list_append(to, data->data);
 *          from = eina_list_remove_list(from, data);
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[out] to The list to move the data to
 * @param[out] from The list to move from
 * @param[in] data The list node containing the data to move
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool             eina_list_move_list(Eina_List **to, Eina_List **from, Eina_List *data);


/**
 * @brief Frees an entire list and all the nodes, ignoring the data contained.
 *
 * @details This function frees all the nodes of @a list. It does not free the
 *          data of the nodes. To free them, use #EINA_LIST_FREE.
 *
 * @since_tizen 2.3
 *
 * @param[in] list The list to free
 * @return A @c NULL pointer
 *
 */
EAPI Eina_List            *eina_list_free(Eina_List *list);


/**
 * @brief Gets the nth member's data pointer in a list.
 *
 * @details This function returns the data pointer of element number @a n, in
 *          the @a list. The first element in the array is element number 0. If
 *          the element number @a n does not exist, @c NULL is
 *          returned. Otherwise, the data of the found element is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks Worst case is O(n).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to get the specified member number from
 * @param[in] n The number of the element (0 being the first)
 * @return The data pointer stored in the specified element
 *
 */
EAPI void                 *eina_list_nth(const Eina_List *list, unsigned int n) EINA_PURE EINA_WARN_UNUSED_RESULT;


/**
 * @brief Gets the nth member's list node in a list.
 *
 * @details This function returns the list node of element number @a n, in
 *          @a list. The first element in the array is element number 0. If the
 *          element number @a n does not exist or @a list is @c NULL, or @a n is
 *          greater than the count of the elements in @a list minus 1, @c NULL is
 *          returned. Otherwise the list node stored in the numbered element is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @remarks Worst case is O(n).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to get the specfied member number from
 * @param[in] n The number of the element (0 being the first)
 * @return The list node stored in the numbered element
 *
 */
EAPI Eina_List            *eina_list_nth_list(const Eina_List *list, unsigned int n) EINA_PURE EINA_WARN_UNUSED_RESULT;


/**
 * @brief Reverses all the elements in the list.
 *
 * @details This function reverses the order of all the elements in @a list, so the
 *          last member is now first, and so on. If @a list is @c NULL, this
 *          function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @remarks @b in-place: This changes the given list, so you should
 *          now point to the new list head that is returned by this function.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to reverse
 * @return The list head after it has been reversed
 *
 * @see eina_list_reverse_clone()
 * @see eina_list_iterator_reversed_new()
 */
EAPI Eina_List            *eina_list_reverse(Eina_List *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Clones (copies) all the elements in the list in the reverse order.
 *
 * @details This function reverses the order of all the elements in @a list, so the
 *          last member is now first, and so on. If @a list is @c NULL, this
 *          function returns @c NULL. This returns a copy of the given list.
 *
 * @since_tizen 2.3
 *
 * @remarks @b copy: This copies the list and you should then use
 *          eina_list_free() when it is not required anymore.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to reverse
 * @return The new list that has been reversed
 *
 * @see eina_list_reverse()
 * @see eina_list_clone()
 */
EAPI Eina_List            *eina_list_reverse_clone(const Eina_List *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Clones (copies) all the elements in the list in the exactly same order.
 *
 * @details This function clones in an order which is same as the order of all the elements in @a list.
 *          If @a list is @c NULL, this functon returns @c NULL. This returns a copy of
 *          the given list.
 *
 * @since_tizen 2.3
 *
 * @remarks @b copy: This copies the list and you should then use
 *          eina_list_free() when it is not required anymore.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to clone
 * @return The new list that has been cloned
 *
 * @see eina_list_reverse_clone()
 */
EAPI Eina_List            *eina_list_clone(const Eina_List *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Sorts a list according to the ordering that @a func returns.
 *
 * @details This function sorts @a list.
 *          If @a limit is @c 0 or greater than the number of
 *          elements in @a list, all the elements are sorted. @a func is used to
 *          compare two elements of @a list. If @a func is @c NULL, this function returns
 *          @a list.
 *
 * @since_tizen 2.3
 *
 * @remarks @b in-place: This changes the given list, so you should
 *          now point to the new list head that is returned by this function.
 *
 * @remarks Worst case is O(n * log2(n)) comparisons (calls to func()).
 *          That means, for 1,000,000 list sort we do 20,000,000 comparisons.
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
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list handle to sort
 * @param[in] limit The maximum number of list elements to sort
 * @param[in] func A function pointer that can handle comparing the list data
 *             nodes
 * @return The new head of the list
 *
 */
EAPI Eina_List            *eina_list_sort(Eina_List *list, unsigned int limit, Eina_Compare_Cb func) EINA_ARG_NONNULL(3) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Merges two lists.
 *
 * @details This function puts @a right at the end of @a left and returns the head.
 *          Both @a left and @a right do not exist anymore after the merge.
 *
 * @since_tizen 2.3
 *
 * @remarks Merge cost is O(n), @b n being the size of the smallest
 *          list. This is due to the need to fix accounting of that segment,
 *          making count and last access O(1).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] left The head list to merge
 * @param[in] right The tail list to merge
 * @return A new merged list
 *
 */
EAPI Eina_List            *eina_list_merge(Eina_List *left, Eina_List *right) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Merges two sorted lists according to the ordering that @a func returns.
 *
 * @details This function compares the head of @a left and @a right, and chooses the
 *          smallest one to be the head of the returned list. It continues this process
 *          for all the entries of both the lists.
 *
 *          Both the left and the right lists are not vaild anymore after the merge and should
 *          not be used. If @a func is @c NULL, it returns @c NULL.
 *
 * @since_tizen 2.3
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
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] left The first list to merge
 * @param[in] right The second list to merge
 * @param[in] func A function pointer that can handle comparing the list data
 *        nodes
 * @return A new sorted list
 *
 */
EAPI Eina_List            *eina_list_sorted_merge(Eina_List *left, Eina_List *right, Eina_Compare_Cb func) EINA_ARG_NONNULL(3) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Splits a list into 2 lists.
 *
 * @details This function splits @a list into two lists ( left and right ) after the node @a relative. @a relative
 *          becomes the last node of the left list. If @a list or @a right is @c NULL, @a list is returned.
 *          If @a relative is @c NULL, @a right is set to @a list and @c NULL is returned.
 *          If @a relative is the last node of @a list, @a list is returned and @a right is set to @c NULL.
 *
 *          List does not exist anymore after the split.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to split
 * @param[in] relative The list is split after @a relative
 * @param[out] right The head of the new right list
 * @return The new left list
 *
 */
EAPI Eina_List            *eina_list_split_list(Eina_List *list, Eina_List *relative, Eina_List **right) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returns the node nearest to data in the sorted list.
 *
 * @details This function searches for a node containing @a data as its data in @a list,
 *          if such a node exists it is returned and @a result_cmp is @c 0. If
 *          the data of no node in @a list is equal to @a data, the node with the nearest
 *          value to that is returned and @a result_cmp is the return value of
 *          @a func with @a data and the returned node's data as arguments.
 *
 * @since_tizen 2.3
 *
 * @remarks This function is useful for inserting an element in the list only in case it
 *          isn't already present in the list, the naive way of doing this would be:
 * @code
 * void *ptr = eina_list_data_find(list, "my data");
 * if (!ptr)
 *   eina_list_sorted_insert(list, "my data");
 * @endcode
 *
 * @remarks However, this has the downside of walking through the list twice, once to
 *          check if the data is already present and another to insert the element in the
 *          correct position. This can be done more efficiently by:
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
 *          If @a cmp_result is 0 the element is already in the list and we need not
 *          insert it, if @a cmp_result is greater than zero @a "my @a data" needs to
 *          come after @a l(the nearest node present), if less than zero it needs to come before.
 *
 * @remarks Average/worst case performance is O(log2(n)), for 1,000,000
 *          elements it does a maximum of 20 comparisons. This is much
 *          faster than the 1,000,000 comparisons made naively by walking the list
 *          from head to tail, so depending on the number of searches and
 *          insertions, it may be better to eina_list_sort() the list and do the
 *          searches later. As lists do not have O(1) access time, walking to
 *          the correct node can be costly, consider worst case to be almost
 *          O(n) pointer dereference (list walk).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to search for data, @b must be sorted
 * @param[in] func A function pointer that can handle comparing the list data nodes
 * @param[in] data The reference value to search
 * @param[in] result_cmp If provided it returns the result of
 *                   the func(node->data, data) node being the last (returned) node. If node
 *                   is found (exact match), then it is @c 0. If the returned node is smaller
 *                   than the requested data, it is less than @c 0 and if it's bigger it's
 *                   greater than @c 0. It is the last value returned by func().
 * @return The nearest node, otherwise @c NULL if not found
 *
 *
 * @see eina_list_search_sorted_list()
 * @see eina_list_sort()
 * @see eina_list_sorted_merge()
 */
EAPI Eina_List            *eina_list_search_sorted_near_list(const Eina_List *list, Eina_Compare_Cb func, const void *data, int *result_cmp);


/**
 * @brief Returns the node if data is in the sorted list.
 *
 * @since_tizen 2.3
 *
 * @remarks This can be used to check if some value is inside the list and get
 *          the container node in this case. It should be used when the list is
 *          known to be sorted as it does a binary search for results.
 *
 *          Example: Imagine a user gives a string, you check if it's in the list
 *          before duplicating its contents.
 *
 * @remarks  Average/worst case performance is O(log2(n)), for 1,000,000
 *           elements it does a maximum of 20 comparisons. This is much
 *           faster than the 1,000,000 comparisons made by
 *           eina_list_search_unsorted_list(), so depending on the number of
 *           searches and insertions, it may be better to eina_list_sort() the
 *           list and do the searches later. As said in
 *           eina_list_search_sorted_near_list(), lists do not have O(1) access
 *           time, so walking to the correct node can be costly, consider worst
 *           case to be almost O(n) pointer dereference (list walk).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to search for data, @b must be sorted
 * @param[in] func A function pointer that can handle comparing the list data nodes
 * @param[in] data The reference value to search
 * @return The node if func(node->data, data) == 0, otherwise @c NULL if not found
 *
 * @see eina_list_search_sorted()
 * @see eina_list_sort()
 * @see eina_list_sorted_merge()
 * @see eina_list_search_unsorted_list()
 * @see eina_list_search_sorted_near_list()
 */
EAPI Eina_List            *eina_list_search_sorted_list(const Eina_List *list, Eina_Compare_Cb func, const void *data);


/**
 * @brief Returns the node data if it is in the sorted list.
 *
 * @since_tizen 2.3
 *
 * @remarks This can be used to check if some value is inside the list and get
 *          the existing instance in this case. It should be used when a list is
 *          known to be sorted as it does a binary search for results.
 *
 *          Example: Imagine a user gives a string, you check if it's in the list
 *          before duplicating its contents.
 *
 * @remarks Average/worst case performance is O(log2(n)), for 1,000,000
 *          elements it does a maximum of 20 comparisons. This is much
 *          faster than the 1,000,000 comparisons made by
 *          eina_list_search_unsorted(), so depending on the number of
 *          searches and insertions, it may be better to eina_list_sort() the
 *          list and do the searches later. As said in
 *          eina_list_search_sorted_near_list(), lists do not have O(1) access
 *          time, so walking to the correct node can be costly, consider worst
 *          case to be almost O(n) pointer dereference (list walk).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to search for data, @b must be sorted
 * @param[in] func A function pointer that can handle comparing the list data nodes.
 * @param[in] data The reference value to search
 * @return The node value (@c node->data) if func(node->data, data) == 0,
 *         otherwise @c NULL if not found
 *
 * @see eina_list_search_sorted_list()
 * @see eina_list_sort()
 * @see eina_list_sorted_merge()
 * @see eina_list_search_unsorted_list()
 */
EAPI void                 *eina_list_search_sorted(const Eina_List *list, Eina_Compare_Cb func, const void *data);


/**
 * @brief Returns the node if data is in the unsorted list.
 *
 * @since_tizen 2.3
 *
 * @remarks This can be used to check if some value is inside the list and get
 *          the container node in this case.
 *
 *          Example: Imagine a user gives a string, you check if it's in the list
 *          before duplicating its contents.
 *
 * @remarks This is expensive and may walk the whole list, it's order-N,
 *          that is for 1,000,000 elements list it may walk and compare
 *          1,000,000 nodes.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to search for data, may be unsorted
 * @param[in] func A function pointer that can handle comparing the list data nodes
 * @param[in] data The reference value to search
 * @return The node if func(node->data, data) == 0, otherwise @c NULL if not found
 *
 * @see eina_list_search_sorted_list()
 * @see eina_list_search_unsorted()
 */
EAPI Eina_List            *eina_list_search_unsorted_list(const Eina_List *list, Eina_Compare_Cb func, const void *data);


/**
 * @brief Returns the node data if it is in the unsorted list.
 *
 * @since_tizen 2.3
 *
 * @remarks This can be used to check if some value is inside the list and get
 *          the existing instance in this case.
 *
 *          Example: Imagine a user gives a string, you check if it's in the list
 *          before duplicating its contents.
 *
 * @remarks This is expensive and may walk the whole list, it's order-N,
 *          that is for 1,000,000 elements list it may walk and compare
 *          1,000,000 nodes.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to search for data, may be unsorted
 * @param[in] func A function pointer that can handle comparing the list data nodes
 * @param[in] data The reference value to search.
 * @return The node value (@c node->data) if func(node->data, data) == 0,
 *         otherwise @c NULL if not found
 *
 * @see eina_list_search_sorted()
 * @see eina_list_search_unsorted_list()
 */
EAPI void                 *eina_list_search_unsorted(const Eina_List *list, Eina_Compare_Cb func, const void *data);

/**
 * @brief Gets the last list node in the list.
 *
 * @details This function returns the last list node in the list @a list. If
 *          @a list is @c NULL or empty, @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks This is an order-1 operation (it takes the same short time
 *          regardless of the length of the list).
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list to get the last list node from
 * @return The last list node in the list
 *
 */
static inline Eina_List   *eina_list_last(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets the next list node after the specified list node.
 *
 * @details This function returns the next list node after the current one in
 *          @a list. It is equivalent to list->next. If @a list is @c NULL or
 *          if no next list node exists, it returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list node to get the next list node from
 * @return The next list node on success, otherwise @c NULL
 *
 */
static inline Eina_List   *eina_list_next(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets the previous list node before the specified list node.
 *
 * @details This function returns the previous list node before the current one
 *          in @a list. It is equivalent to list->prev. If @a list is @c NULL or
 *          if no previous list node exists, it returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list node to get the previous list node from
 * @return The previous list node on success, otherwise @c NULL
 *         if no previous list node exists
 *
 */
static inline Eina_List   *eina_list_prev(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets the list node data member.
 *
 * @details This function returns the data member of the specified list node
 *          @a list. It is equivalent to list->data. If @p list is @c NULL, this
 *          function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list node to get the data member of
 * @return The data member from the list node
 *
 */
static inline void        *eina_list_data_get(const Eina_List *list) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Sets the list node data member.
 *
 * @details This function sets the data member @a data of the specified list node
 *          @a list. It returns the previous data of the node. If @a list is
 *          @c NULL, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list node to get the data member of
 * @param[in] data The data member for the list node
 * @return The previous data value
 *
 */
static inline void        *eina_list_data_set(Eina_List *list, const void *data);

/**
 * @brief Gets the count of the number of items in a list.
 *
 * @details This function returns the number of members that @a list contains. If the
 *          @a list is @c NULL, @c 0 is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks This is an order-1 operation and takes the same time regardless
 *          of the length of the list.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list whose count to return
 * @return The number of members in the list
 *
 */
static inline unsigned int eina_list_count(const Eina_List *list) EINA_PURE;


/**
 * @brief Returns a new iterator associated to a list.
 *
 * @details This function returns a newly allocated iterator associated to @a
 *          list. If @a list is @c NULL or the count member of @a list is less than
 *          or equal to @c 0, this function still returns a valid iterator that
 *          always returns @c false on eina_iterator_next(), thus keeping the API
 *          sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator
 *          is returned.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @remarks If the list structure changes then the iterator becomes
 *          invalid. That is, if you add or remove nodes this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] list The list
 * @return A new iterator
 *
 */
EAPI Eina_Iterator        *eina_list_iterator_new(const Eina_List *list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returns a new reversed iterator associated to a list.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a list. If @a list is @c NULL or the count member of @a list is less than
 *          or equal to 0, this function still returns a valid iterator that
 *          always returns @c false on eina_iterator_next(), thus keeping the API
 *          sane.
 *
 * @since_tizen 2.3
 *
 * @remarks Unlike eina_list_iterator_new(), this walks the list backwards.
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator
 *          is returned.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @remarks If the list structure changes then the iterator becomes
 *          invalid. That is, if you add or remove nodes this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] list The list
 * @return A new iterator
 *
 */
EAPI Eina_Iterator        *eina_list_iterator_reversed_new(const Eina_List *list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returns a new accessor associated to a list.
 *
 * @details This function returns a newly allocated accessor associated to
 *          @a list. If @a list is @c NULL or the count member of @a list is
 *          less than or equal to 0, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @remarks	If the memory cannot be allocated,
 *          @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 *          set. Otherwise, a valid accessor is returned.
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param[in] list The list
 * @return A new accessor
 *
 */
EAPI Eina_Accessor        *eina_list_accessor_new(const Eina_List *list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @def EINA_LIST_FOREACH
 * @brief Definition of the macro to iterate over a list.
 *
 * @details This macro iterates over @a list from the first element to
 *          the last. @a data is the data related to the current element.
 *          @a l is an #Eina_List used as the list iterator.
 *
 * @since_tizen 2.3
 *
 * @remarks The following diagram ilustrates this macro iterating over a list of four
 *          elements("one", "two", "three" and "four"):
 *
 * @image html eina-list-foreach.png
 * @htmlonly
 * <a href="eina-list-foreach.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-foreach.eps "eina list foreach" width=\textwidth
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
 * @remarks This is not the optimal way to release memory allocated to
 *          a list, since it iterates over the list twice.
 *          For an optimized algorithm, use EINA_LIST_FREE().
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @remarks Be careful when deleting list nodes.
 *          If you remove the current node and continue iterating,
 *          the code fails because the macro is not able
 *          to get the next node. Notice that it's OK to remove any
 *          node if you stop the loop after that.
 *          For destructive operations such as this, consider
 *          using EINA_LIST_FOREACH_SAFE().
 *
 * @param list The list to iterate over
 * @param l A list that is used as an iterator and points to the current node
 * @param data The current item's data
 *
 */
#define EINA_LIST_FOREACH(list, l, data) \
  for (l = list,                         \
       data = eina_list_data_get(l);     \
       l;                                \
       l = eina_list_next(l),            \
       data = eina_list_data_get(l))

/**
 * @def EINA_LIST_REVERSE_FOREACH
 * @brief Definition of the macro to iterate over a list in the reverse order.
 *
 * @details This macro works like EINA_LIST_FOREACH, but iterates from the
 *          last element of a list to the first.
 *          @a data is the data related to the current element, while @a l
 *          is an #Eina_List that is used as the list iterator.
 *
 * @since_tizen 2.3
 *
 * @remarks The following diagram ilustrates this macro iterating over a list of four
 *          elements("one", "two", "three" and "four"):
 *
 * @image html eina-list-reverse-foreach.png
 * @htmlonly
 * <a href="eina-list-reverse-foreach.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-reverse-foreach.eps "eina list reverse foreach" width=\textwidth
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
 * @remarks This is not the optimal way to release memory allocated to
 *          a list, since it iterates over the list twice.
 *          For an optimized algorithm, use EINA_LIST_FREE().
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @remarks Be careful when deleting list nodes.
 *          If you remove the current node and continue iterating,
 *          the code fails because the macro is not able
 *          to get the next node. Notice that it's OK to remove any
 *          node if you stop the loop after that.
 *          For destructive operations such as this, consider
 *          using EINA_LIST_REVERSE_FOREACH_SAFE().
 *
 * @param list The list to iterate over
 * @param l A list that is used as an iterator and points to the current node
 * @param data The current item's data
 *
 */
#define EINA_LIST_REVERSE_FOREACH(list, l, data) \
  for (l = eina_list_last(list),                 \
       data = eina_list_data_get(l);             \
       l;                                        \
       l = eina_list_prev(l),                    \
       data = eina_list_data_get(l))

/**
 * @def EINA_LIST_FOREACH_SAFE
 * @brief Definition of the macro to iterate over a list with support for node deletion.
 *
 * @details This macro iterates over @a list from the first element to
 *          the last. @a data is the data related to the current element.
 *          @a l is an #Eina_List used as the list iterator.
 *
 * @since_tizen 2.3
 *
 * @remarks Since this macro stores a pointer to the next list node in @a l_next,
 *          deleting the current node and continuing looping is safe.
 *
 * @remarks The following diagram ilustrates this macro iterating over a list of four
 *          elements("one", "two", "three" and "four"):
 *
 * @image html eina-list-foreach-safe.png
 * @htmlonly
 * <a href="eina-list-foreach-safe.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-foreach-safe.eps "eina list foreach safe" width=\textwidth
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
 *   if (strcmp(data, "key") == 0) {
 *      free(data);
 *      list = eina_list_remove_list(list, l);
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param list The list to iterate over
 * @param l A list that is used as an iterator and points to the current node
 * @param l_next A list that is used as an iterator and points to the next node
 * @param data The current item's data
 *
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
 * @brief Definition of the macro to iterate over a list in the reverse order with support
 *        for deletion.
 *
 * @details This macro works like EINA_LIST_FOREACH_SAFE, but iterates from the
 *          last element of a list to the first.
 *          @a data is the data related to the current element, while @a l
 *          is an #Eina_List that is used as the list iterator.
 *
 * @since_tizen 2.3
 *
 * @remarks Since this macro stores a pointer to the previous list node in @a l_prev,
 *          deleting the current node and continuing looping is safe.
 *
 * @remarks The following diagram ilustrates this macro iterating over a list of four
 *          elements("one", "two", "three" and "four"):
 *
 * @image html eina-list-reverse-foreach-safe.png
 * @htmlonly
 * <a href="eina-list-reverse-foreach-safe.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-reverse-foreach-safe.eps "eina list reverse foreach safe" width=\textwidth
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
 *   if (strcmp(data, "key") == 0) {
 *      free(data);
 *      list = eina_list_remove_list(list, l);
 *   }
 * @endcode
 *
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param list The list to iterate over
 * @param l A list that is used as an iterator and points to the current node
 * @param l_prev A list that is used as an iterator and points to the previous node
 * @param data The current item's data
 *
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
 * @brief Definition of the macro to remove each list node while having access to each node's data.
 *
 * @details This macro calls #eina_list_remove_list for each list node and stores
 *          the data contained in the current node in @a data.
 *
 * @since_tizen 2.3
 *
 * @remarks The following diagram ilustrates this macro iterating over a list of four
 *          elements("one", "two", "three" and "four"):
 *
 * @image html eina-list-free.png
 * @htmlonly
 * <a href="eina-list-free.png">Full-size</a>
 * @endhtmlonly
 * @image latex eina-list-free.eps "eina list free" width=\textwidth
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
 * @remarks @a list must be a pointer to the first element of the list.
 *
 * @param list The list that is cleared
 * @param data The current node's data
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

#endif /* EINA_LIST_H_ */
