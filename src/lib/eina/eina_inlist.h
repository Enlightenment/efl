/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri
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

#ifndef EINA_INLIST_H_
#define EINA_INLIST_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"
#include <stddef.h>

/**
 * @defgroup Eina_Inline_List_Group Inline List
 * @ingroup Eina_Containers_Group
 *
 * @brief These functions provide inline list management.
 *
 * Inline lists mean its nodes pointers are part of the same memory as
 * the data. This has the benefit of fragmenting less memory and avoiding
 * @c node->data indirection, but has the drawback of higher cost for some
 * common operations like count and sort.
 *
 * It is possible to have inlist nodes to be part of regular lists, created with
 * @ref eina_list_append() or @ref eina_list_prepend(). It's also possible to
 * have a structure with two inlist pointers, thus being part of two different
 * inlists at the same time, but the current convenience macros that are provided won't
 * work for both of them. Consult @ref inlist_advanced for more info.
 *
 * Inline lists have their purposes, but if you don't know what those purposes are, go with
 * regular lists instead.
 *
 * Tip: When using inlists in more than one place (that is, passing them around
 * functions or keeping a pointer to them in a structure) it's is better
 * to keep a pointer to the first container, and not a pointer to the first
 * inlist item (mostly they are the same, but that's not always correct).
 * This lets the compiler do type checking and lets the programmer know
 * exactly what type this list is.
 *
 * @section inlist_algo Algorithm
 *
 * The basic structure can be represented by the following picture:
 *
 * @image html eina_inlist-node.png
 * @image rtf eina_inlist-node.png
 * @image latex eina_inlist-node.eps
 *
 * One data structure also has node information with three pointers:
 * @a prev, @a next, and @a last. The @a last pointer is just valid for the first
 * element (the list head), otherwise each insertion in the list has to
 * be done by updating every node with the correct pointer. This means that it's
 * very important to keep a pointer to the first element of the list,
 * since it is the only one that has the correct information to allow a proper
 * O(1) append to the list.
 *
 * @section inlist_perf Performance
 *
 * Due to the nature of the inlist, there's no accounting information, and no
 * easy access to the last element of each list node. This means that @ref
 * eina_inlist_count() is order-N, while @ref eina_list_count() is order-1 (constant
 * time).
 *
 * @section inlist_advanced Advanced Usage
 *
 * The basic usage considers a struct that has the user data, and also
 * has the inlist node information (prev, next, and last pointers) created with
 * @ref EINA_INLIST during struct declaration. This allows one to use the
 * convenience macros @ref EINA_INLIST_GET(), @ref EINA_INLIST_CONTAINER_GET(),
 * @ref EINA_INLIST_FOREACH(), and so on. This happens because the @ref EINA_INLIST
 * macro declares a struct member with the name @a __inlist, and all the other
 * macros assume that this struct member has this name.
 *
 * It may be the case that someone needs to have some inlist nodes added to a
 * @ref Eina_List too. If this happens, the inlist nodes can be added to the
 * @ref Eina_List without any problem.
 *
 * It's also possible to have data that is part of two different inlists.
 * If this is the case, then it won't be possible to use convenience macros
 * for both of the lists. It is necessary to create a new set of macros that
 * allow access to the second list node info.
 *
 * @{
 */

/**
 * @typedef Eina_Inlist
 * @brief The structure type for an inlined list type.
 */
typedef struct _Eina_Inlist Eina_Inlist;

/**
 * @typedef Eina_Inlist_Sorted_State
 * @brief The structure type for the state of a sorted Eina_Inlist.
 * @since 1.1.0
 */
typedef struct _Eina_Inlist_Sorted_State Eina_Inlist_Sorted_State;

/**
 * @struct _Eina_Inlist
 * @brief The structure type for an Inlined list type.
 */
struct _Eina_Inlist
{
   Eina_Inlist *next; /**< Next node */
   Eina_Inlist *prev; /**< Previous node */
   Eina_Inlist *last; /**< Last node */
};
/** Definition used for declaring an inlist member in a struct */
#define EINA_INLIST Eina_Inlist __in_list
/** Definition of the utility macro to get the inlist object of a struct */
#define EINA_INLIST_GET(Inlist)         (& ((Inlist)->__in_list))
/** Definition of the utility macro to get the container object of an inlist */
#define EINA_INLIST_CONTAINER_GET(ptr,                          \
                                  type) ((type *)((char *)ptr - \
                                                  offsetof(type, __in_list)))


/**
 * @brief Adds a new node to the end of a list.
 *
 * @details  This code is meant to be fast: appends are O(1) and do not
 *           walk through @a in_list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a in_item is considered to be in no list. If it has been in another
 *          list before, eina_inlist_remove() it before adding. No
 *          check of @a new_l prev and next pointers is done, so it's safe
 *          to have them uninitialized.
 *
 * @param[in] in_list The existing list head, otherwise @c NULL to create a new list
 * @param[in] in_item new The list node, must not be @c NULL
 *
 * @return The new list head \n
 *         Use it and not @a in_list anymore.
 */
EAPI Eina_Inlist *eina_inlist_append(Eina_Inlist *in_list,
                                     Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Adds a new node to the beginning of the list.
 *
 * @details This code is meant to be fast: appends are O(1) and do not
 *          walk through @a in_list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a new_l is considered to be in no list. If it has been in another
 *          list before, eina_inlist_remove() it before adding. No
 *          check of @a new_l prev and next pointers is done, so it's safe
 *          to have them uninitialized.
 *
 * @param[in] in_list The existing list head, otherwise @c NULL to create a new list
 * @param[in] in_item The new list node, must not be @c NULL
 *
 * @return The new list head \n
 *         Use it and not @a in_list anymore.
 */
EAPI Eina_Inlist *eina_inlist_prepend(Eina_Inlist *in_list,
                                      Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Adds a new node after the given relative item in the list.
 *
 * @details This code is meant to be fast: appends are O(1) and do not
 *          walk through @a in_list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a in_item_l is considered to be in no list. If it has been in another
 *          list before, eina_inlist_remove() it before adding. No
 *          check of @a in_item prev and next pointers is done, so it's safe
 *          to have them uninitialized.
 *
 * @remarks @a in_relative is considered to be inside @a in_list, no checks are
 *          done to confirm that and giving nodes from different lists
 *          leads to problems. Setting @a in_relative to @c NULL is the same as
 *          eina_list_append().
 *
 * @param[in] in_list The existing list head, otherwise @c NULL to create a new list
 * @param[in] in_item new The list node, must not be @c NULL
 * @param[in] in_relative The reference node, @a in_item is added after it
 *
 * @return The new list head \n
 *         Use it and not @a list anymore.
 */
EAPI Eina_Inlist *eina_inlist_append_relative(Eina_Inlist *in_list,
                                              Eina_Inlist *in_item,
                                              Eina_Inlist *in_relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Adds a new node before the given relative item in the list.
 *
 * @details This code is meant to be fast: appends are O(1) and do not
 *          walk through @a in_list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a in_item is considered to be in no list. If it has been in another
 *          list before, eina_inlist_remove() it before adding. No
 *          check of @a in_item prev and next pointers is done, so it's safe
 *          to have them uninitialized.
 *
 * @remarks @a in_relative is considered to be inside @a in_list, no checks are
 *          done to confirm that and giving nodes from different lists
 *          leads to problems. Setting @a in_relative to @c NULL is the same as
 *          eina_list_prepend().
 *
 * @param[in] in_list The existing list head, otherwise @c NULL to create a new list
 * @param[in] in_item new The list node, must not be @c NULL
 * @param[in] in_relative The reference node, @a in_item is added before it
 *
 * @return The new list head \n
 *         Use it and not @a in_list anymore.
 */
EAPI Eina_Inlist *eina_inlist_prepend_relative(Eina_Inlist *in_list,
                                               Eina_Inlist *in_item,
                                               Eina_Inlist *in_relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Removes a node from the list.
 *
 * @details This code is meant to be fast: appends are O(1) and do not
 *          walk through @a list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a in_item is considered to be inside @a in_list, no checks are
 *          done to confirm that and giving nodes from different lists
 *          lead to problems, especially if @a in_item is the head since
 *          it is different from @a list and the wrong new head is
 *          returned.
 *
 * @param[in] in_list The existing list head, must not be @c NULL
 * @param[in] in_item The existing list node, must not be @c NULL
 *
 * @return The new list head \n
 *         Use it and not @a list anymore.
 */
EAPI Eina_Inlist   *eina_inlist_remove(Eina_Inlist *in_list,
                                       Eina_Inlist *in_item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Finds a given node in the list, returns itself if found, @c NULL if not.
 *
 * @details This is an expensive call and has O(n) cost, possibly
 *          walking the whole list.
 *
 * @since_tizen 2.3
 *
 * @param[in] in_list The existing list to search @a in_item in, must not be @c NULL
 * @param[in] in_item The item to search for, must not be @c NULL
 *
 * @return @a in_item if found, @c NULL if not
 */
EAPI Eina_Inlist   *eina_inlist_find(Eina_Inlist *in_list,
                                     Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Moves an existing node to the beginning of the list.
 *
 * @details This code is meant to be fast: appends are O(1) and do not
 *          walk through @a list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a item is considered to be inside @a list. No checks are
 *          done to confirm this, and giving nodes from different lists
 *          leads to problems.
 *
 * @param[in] list The existing list head, otherwise @c NULL to create a new list
 * @param[in] item The list node to move to the beginning (head), must not be @c NULL
 *
 * @return The new list head \n
 *         Use it and not @a list anymore.
 */
EAPI Eina_Inlist   *eina_inlist_promote(Eina_Inlist *list,
                                        Eina_Inlist *item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Moves an existing node to the end of the list.
 *
 * @details This code is meant to be fast: appends are O(1) and do not
 *          walk through @a list.
 *
 * @since_tizen 2.3
 *
 * @remarks @a item is considered to be inside @a list. No checks are
 *          done to confirm this, and giving nodes from different lists
 *          leads to problems.
 *
 * @param[in] list The existing list head, otherwise @c NULL to create a new list
 * @param[in] item The list node to move to the end (tail), must not be @c NULL
 *
 * @return The new list head \n
 *         Use it and not @a list anymore.
 */
EAPI Eina_Inlist   *eina_inlist_demote(Eina_Inlist *list,
                                       Eina_Inlist *item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets the count of the number of items in a list.
 *
 * @details This function returns the number of members that @a list contains. If the
 *          @a list is @c NULL, @c 0 is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks This is an order-N operation and so the time depends
 *          on the number of elements in the list, so it might become
 *          slow for big lists.
 *
 * @param[in] list The list whose count to return
 * @return The number of members in the list
 *
 */
EAPI unsigned int   eina_inlist_count(const Eina_Inlist *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returns a new iterator associated to @a list.
 *
 * @details This function returns a newly allocated iterator associated to @a
 *          in_list. If @a in_list is @c NULL or the count member of @a in_list is less than
 *          or equal to @c 0, this function still returns a valid iterator that
 *          always returns @c false on a call to eina_iterator_next(), thus keeping the API
 *          sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator
 *          is returned.
 *
 * @remarks If the list structure changes then the iterator becomes
 *          invalid, and if you add or remove nodes the iterator's
 *          behavior is undefined, and your program may crash.
 *
 * @param[in] in_list The list
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_inlist_iterator_new(const Eina_Inlist *in_list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new accessor associated to the list.
 *
 * @details This function returns a newly allocated accessor associated to
 *          @a in_list. If @a in_list is @c NULL or the count member of @a in_list is
 *          less than or equal to @c 0, this function returns @c NULL. If the memory cannot
 *          be allocated, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 *          set. Otherwise, a valid accessor is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] in_list The list
 * @return A new accessor
 *
 */
EAPI Eina_Accessor *eina_inlist_accessor_new(const Eina_Inlist *in_list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Inserts a new node into a sorted list.
 *
 * @details This function inserts items into a linked list assuming it is
 *          sorted and the result is sorted. If @a list is @c NULLL, @a item
 *          is returned. On success, a new list pointer that should be
 *          used in place of the one given to this function is
 *          returned. Otherwise, the old pointer is returned.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @remarks Average/worst case performance is O(log2(n)) comparisons (calls to @a func). 
 *          As said in eina_list_search_sorted_near_list(),
 *          lists do not have O(1) access time, so walking to the correct node
 *          can be costly, consider worst case to be almost O(n) pointer
 *          dereference (list walk).
 *
 * @param[in] list The given linked list, @b must be sorted
 * @param[in] item The list node to insert, must not be @c NULL
 * @param[in] func The function called for the sort
 * @return A list pointer
 *
 * @see eina_error_get()
 */
EAPI Eina_Inlist *eina_inlist_sorted_insert(Eina_Inlist *list, Eina_Inlist *item, Eina_Compare_Cb func) EINA_ARG_NONNULL(2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a state with the valid data in it.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @return A valid Eina_Inlist_Sorted_State
 *
 * @see eina_inlist_sorted_state_insert()
 */
EAPI Eina_Inlist_Sorted_State *eina_inlist_sorted_state_new(void);

/**
 * @brief Forces an Eina_Inlist_Sorted_State to match the content of a list.
 *
 * @details This function is useful if you didn't use eina_inlist_sorted_state_insert() at any point,
 *          but still think you have a sorted list.
 *          It only works correctly on a sorted list.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] state The state to update
 * @param[in] list The list to match
 * @return The number of items that are actually in the list
 *
 * @see eina_inlist_sorted_state_insert()
 */
EAPI int eina_inlist_sorted_state_init(Eina_Inlist_Sorted_State *state, Eina_Inlist *list);

/**
 * @brief Frees an Eina_Inlist_Sorted_State.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] state The state to destroy
 *
 * @see eina_inlist_sorted_state_insert()
 */
EAPI void eina_inlist_sorted_state_free(Eina_Inlist_Sorted_State *state);

/**
 * @brief Inserts a new node into a sorted list.
 *
 * @details This function inserts an item into a linked list assuming @a state matches
 *          the exact content order of the list. It uses @a state to do a fast
 *          first step dichotomic search before starting to walk through the inlist itself.
 *          This makes this code much faster than eina_inlist_sorted_insert() as it
 *          doesn't need to walk through the list at all. The result is of course a sorted
 *          list with an updated state.. If @a list is @c NULLL, @a item
 *          is returned. On success, a new list pointer that should be
 *          used in place of the one given to this function is
 *          returned. Otherwise, the old pointer is returned.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @remarks Average/worst case performance is O(log2(n)) comparisons (calls to @a func)
 *          As said in eina_list_search_sorted_near_list(),
 *          lists do not have O(1) access time, so walking to the correct node
 *          can be costly, but this version tries to minimize that by making it
 *          O(log2(n)) for a small number. After n == 256, it starts to add
 *          linear cost again. Consider worst case to be almost O(n) pointer
 *          dereference (list walk).
 *
 * @param[in] list The given linked list, @b must be sorted
 * @param[in] item The list node to insert, must not be @c NULL
 * @param[in] func The function called for the sort
 * @param[in] state The current array for an initial dichotomic search
 * @return A list pointer
 *
 *
 * @see eina_error_get()
 */
EAPI Eina_Inlist *eina_inlist_sorted_state_insert(Eina_Inlist *list,
						  Eina_Inlist *item,
						  Eina_Compare_Cb func,
						  Eina_Inlist_Sorted_State *state);
/**
 * @brief Sorts a list according to the ordering that @a func returns.
 *
 * @details This function sorts all the elements of @a head. @a func is used to
 *          compare two elements of @a head. If @a head or @a func is @c NULL,
 *          this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @remarks @b in-place: This changes the given list, so you should
 *          now point to the new list head that is returned by this function.
 *
 * @remarks Worst case is O(n * log2(n)) comparisons (calls to func()).
 *          That means, for 1,000,000 list  elements, sort does 20,000,000
 *          comparisons.
 *
 * Example:
 * @code
 * typedef struct _Sort_Ex Sort_Ex;
 * struct _Sort_Ex
 * {
 *   INLIST;
 *   const char *text;
 * };
 *
 * int
 * sort_cb(const Inlist *l1, const Inlist *l2)
 * {
 *    const Sort_Ex *x1;
 *    const Sort_Ex *x2;
 *
 *    x1 = EINA_INLIST_CONTAINER_GET(l1, Sort_Ex);
 *    x2 = EINA_INLIST_CONTAINER_GET(l2, Sort_Ex);
 *
 *    return(strcmp(x1->text, x2->text));
 * }
 * extern Eina_Inlist *list;
 *
 * list = eina_inlist_sort(list, sort_cb);
 * @endcode
 *
 * @param[in] head The list handle to sort
 * @param[in] func A function pointer that can handle comparing the list data nodes
 *
 * @return The new head of the list
 *
 */
EAPI Eina_Inlist *eina_inlist_sort(Eina_Inlist *head, Eina_Compare_Cb func);

/* These two macros are helpers for the _FOREACH ones, don't use them */
/**
 * @def _EINA_INLIST_OFFSET
 *
 * @since_tizen 2.3
 *
 * @param ref The reference to be used
 */
#define _EINA_INLIST_OFFSET(ref)         ((char *)&(ref)->__in_list - (char *)(ref))

#if !defined(__cplusplus)
/**
 * @def _EINA_INLIST_CONTAINER
 *
 * @since_tizen 2.3
 *
 * @param ref The reference to be used
 * @param ptr The pointer to be used
 */
#define _EINA_INLIST_CONTAINER(ref, ptr) (void *)((char *)(ptr) - \
                                                  _EINA_INLIST_OFFSET(ref))
#else
/*
 * In C++ we can't assign a "type*" pointer to void* so we rely on GCC's typeof
 * operator.
 */
#define _EINA_INLIST_CONTAINER(ref, ptr) (typeof(ref))((char *)(ptr) - \
                                                  _EINA_INLIST_OFFSET(ref))
#endif

/**
 * @def EINA_INLIST_FOREACH
 *
 * @since_tizen 2.3
 *
 * @param list The list to iterate on
 * @param it A pointer to the list item, i.e. a pointer to each item
 *           that is a part of the list.
 */
#define EINA_INLIST_FOREACH(list, it)                                     \
  for (it = NULL, it = (list ? _EINA_INLIST_CONTAINER(it, list) : NULL); it; \
       it = (EINA_INLIST_GET(it)->next ? _EINA_INLIST_CONTAINER(it, EINA_INLIST_GET(it)->next) : NULL))

/**
 * @def EINA_INLIST_FOREACH_SAFE
 *
 * @since_tizen 2.3
 *
 * @param list The list to iterate on
 * @param list2 The auxiliar Eina_Inlist variable so we can save the pointer to the
 *              next element, allowing us to free/remove the current one \n
 *              Note that this macro is safe only if the next element is not removed \n
 *              Only the current one is allowed to be removed.
 *
 * @param it A pointer to the list item, i.e. a pointer to each item
 *           that is a part of the list
 */
#define EINA_INLIST_FOREACH_SAFE(list, list2, it) \
   for (it = NULL, it = (list ? _EINA_INLIST_CONTAINER(it, list) : NULL), list2 = it ? ((EINA_INLIST_GET(it) ? EINA_INLIST_GET(it)->next : NULL)) : NULL; \
        it; \
        it = NULL, it = list2 ? _EINA_INLIST_CONTAINER(it, list2) : NULL, list2 = list2 ? list2->next : NULL)

/**
 * @def EINA_INLIST_REVERSE_FOREACH
 *
 * @since_tizen 2.3
 *
 * @param list The list to traverse in the reverse order
 * @param it A pointer to the list item, i.e. a pointer to each item
 *           that is a part of the list
 */
#define EINA_INLIST_REVERSE_FOREACH(list, it)                                \
  for (it = NULL, it = (list ? _EINA_INLIST_CONTAINER(it, list->last) : NULL); \
       it; it = (EINA_INLIST_GET(it)->prev ? _EINA_INLIST_CONTAINER(it, EINA_INLIST_GET(it)->prev) : NULL))

/**
 * @}
 */

#endif /*EINA_INLIST_H_*/
