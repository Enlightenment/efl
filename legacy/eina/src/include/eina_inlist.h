/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri
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

#ifndef EINA_INLIST_H_
#define EINA_INLIST_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"
#include <stddef.h>

/**
 * @addtogroup Eina_Inline_List_Group Inline List
 *
 * @brief These functions provide inline list management.
 *
 * Inline lists mean its nodes pointers are part of same memory as
 * data. This has the benefit of fragmenting memory less and avoiding
 * @c node->data indirection, but has the drawback of elements only
 * being able to be part of one single inlist at same time. But it is
 * possible to have inlist nodes to be part of regular lists created
 * with eina_list_append() or eina_list_prepend().
 *
 * Inline lists have their purposes, but if you don't know what those purposes are, go with
 * regular lists instead.
 *
 * Tip: When using inlists in more than one place (that is, passing them around
 * functions or keeping a pointer to them in a structure) it's more correct
 * to keep a pointer to the first container, and not a pointer to the first
 * inlist item (mostly they are the same, but that's not always correct).
 * This lets the compiler to do type checking and let the programmer know
 * exactly what type this list is.
 *
 * @code
 * #include <Eina.h>
 * #include <stdio.h>
 *
 * int
 * main(void)
 * {
 *    struct my_struct {
 *       EINA_INLIST;
 *       int a, b;
 *    } *d, *cur;
 *    Eina_Inlist *list, *itr;
 *
 *    eina_init();
 *
 *    d = malloc(sizeof(*d));
 *    d->a = 1;
 *    d->b = 10;
 *    list = eina_inlist_append(NULL, EINA_INLIST_GET(d));
 *
 *    d = malloc(sizeof(*d));
 *    d->a = 2;
 *    d->b = 20;
 *    list = eina_inlist_append(list, EINA_INLIST_GET(d));
 *
 *    d = malloc(sizeof(*d));
 *    d->a = 3;
 *    d->b = 30;
 *    list = eina_inlist_prepend(list, EINA_INLIST_GET(d));
 *
 *    printf("list=%p\n", list);
 *    EINA_INLIST_FOREACH(list, cur)
 *      printf("\ta=%d, b=%d\n", cur->a, cur->b);
 *
 *    list = eina_inlist_remove(list, EINA_INLIST_GET(d));
 *    free(d);
 *    printf("list=%p\n", list);
 *    for (itr = list; itr != NULL; itr = itr->next)
 *      {
 *         cur = EINA_INLIST_CONTAINER_GET(itr, struct my_struct);
 *         printf("\ta=%d, b=%d\n", cur->a, cur->b);
 *      }
 *
 *    while (list)
 *      {
 *         Eina_Inlist *aux = list;
 *         list = eina_inlist_remove(list, list);
 *         free(aux);
 *      }
 *
 *    eina_shutdown();
 *
 *    return 0;
 * }
 * @endcode
 *
 * @{
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
 * @defgroup Eina_Inline_List_Group Inline List
 *
 * @{
 */

/**
 * @typedef Eina_Inlist
 * Inlined list type.
 */
typedef struct _Eina_Inlist Eina_Inlist;

/**
 * @struct _Eina_Inlist
 * Inlined list type.
 */
struct _Eina_Inlist
{
   Eina_Inlist *next; /**< next node */
   Eina_Inlist *prev; /**< previous node */
   Eina_Inlist *last; /**< last node */
};
/** Used for declaring an inlist member in a struct */
#define EINA_INLIST Eina_Inlist __in_list
/** Utility macro to get the inlist object of a struct */
#define EINA_INLIST_GET(Inlist)         (& ((Inlist)->__in_list))
/** Utility macro to get the container object of an inlist */
#define EINA_INLIST_CONTAINER_GET(ptr,                          \
                                  type) ((type *)((char *)ptr - \
                                                  offsetof(type, __in_list)))


/**
 * Add a new node to end of a list.
 *
 * @note this code is meant to be fast: appends are O(1) and do not
 *       walk @a list.
 *
 * @note @a new_l is considered to be in no list. If it was in another
 *       list before, eina_inlist_remove() it before adding. No
 *       check of @a new_l prev and next pointers is done, so it's safe
 *       to have them uninitialized.
 *
 * @param list existing list head or NULL to create a new list.
 * @param new_l new list node, must not be NULL.
 *
 * @return the new list head. Use it and not @a list anymore.
 */
EAPI Eina_Inlist *eina_inlist_append(Eina_Inlist *in_list,
                                     Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * Add a new node to beginning of list.
 *
 * @note this code is meant to be fast: appends are O(1) and do not
 *       walk @a list.
 *
 * @note @a new_l is considered to be in no list. If it was in another
 *       list before, eina_inlist_remove() it before adding. No
 *       check of @a new_l prev and next pointers is done, so it's safe
 *       to have them uninitialized.
 *
 * @param list existing list head or NULL to create a new list.
 * @param new_l new list node, must not be NULL.
 *
 * @return the new list head. Use it and not @a list anymore.
 */
EAPI Eina_Inlist *eina_inlist_prepend(Eina_Inlist *in_list,
                                      Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * Add a new node after the given relative item in list.
 *
 * @note this code is meant to be fast: appends are O(1) and do not
 *       walk @a list.
 *
 * @note @a new_l is considered to be in no list. If it was in another
 *       list before, eina_inlist_remove() it before adding. No
 *       check of @a new_l prev and next pointers is done, so it's safe
 *       to have them uninitialized.
 *
 * @note @a relative is considered to be inside @a list, no checks are
 *       done to confirm that and giving nodes from different lists
 *       will lead to problems. Giving NULL @a relative is the same as
 *       eina_list_append().
 *
 * @param list existing list head or NULL to create a new list.
 * @param new_l new list node, must not be NULL.
 * @param relative reference node, @a new_l will be added after it.
 *
 * @return the new list head. Use it and not @a list anymore.
 */
EAPI Eina_Inlist *eina_inlist_append_relative(Eina_Inlist *in_list,
                                              Eina_Inlist *in_item,
                                              Eina_Inlist *in_relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * Add a new node before the given relative item in list.
 *
 * @note this code is meant to be fast: appends are O(1) and do not
 *       walk @a list.
 *
 * @note @a new_l is considered to be in no list. If it was in another
 *       list before, eina_inlist_remove() it before adding. No
 *       check of @a new_l prev and next pointers is done, so it's safe
 *       to have them uninitialized.
 *
 * @note @a relative is considered to be inside @a list, no checks are
 *       done to confirm that and giving nodes from different lists
 *       will lead to problems. Giving NULL @a relative is the same as
 *       eina_list_prepend().
 *
 * @param list existing list head or NULL to create a new list.
 * @param new_l new list node, must not be NULL.
 * @param relative reference node, @a new_l will be added before it.
 *
 * @return the new list head. Use it and not @a list anymore.
 */
EAPI Eina_Inlist *eina_inlist_prepend_relative(Eina_Inlist *in_list,
                                               Eina_Inlist *in_item,
                                               Eina_Inlist *in_relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * Remove node from list.
 *
 * @note this code is meant to be fast: appends are O(1) and do not
 *       walk @a list.
 *
 * @note @a item is considered to be inside @a list, no checks are
 *       done to confirm that and giving nodes from different lists
 *       will lead to problems, especially if @a item is the head since
 *       it will be different from @a list and the wrong new head will
 *       be returned.
 *
 * @param list existing list head, must not be NULL.
 * @param item existing list node, must not be NULL.
 *
 * @return the new list head. Use it and not @a list anymore.
 */
EAPI Eina_Inlist   *eina_inlist_remove(Eina_Inlist *in_list,
                                       Eina_Inlist *in_item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * Find given node in list, returns itself if found, NULL if not.
 *
 * @warning this is an expensive call and has O(n) cost, possibly
 *    walking the whole list.
 *
 * @param list existing list to search @a item in, must not be NULL.
 * @param item what to search for, must not be NULL.
 *
 * @return @a item if found, NULL if not.
 */
EAPI Eina_Inlist   *eina_inlist_find(Eina_Inlist *in_list,
                                     Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;

/**
 * Move existing node to beginning of list.
 *
 * @note this code is meant to be fast: appends are O(1) and do not
 *       walk @a list.
 *
 * @note @a item is considered to be inside @a list. No checks are
 *       done to confirm this, and giving nodes from different lists
 *       will lead to problems.
 *
 * @param list existing list head or NULL to create a new list.
 * @param item list node to move to beginning (head), must not be NULL.
 *
 * @return the new list head. Use it and not @a list anymore.
 */
EAPI Eina_Inlist   *eina_inlist_promote(Eina_Inlist *list,
                                        Eina_Inlist *item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * Move existing node to end of list.
 *
 * @note this code is meant to be fast: appends are O(1) and do not
 *       walk @a list.
 *
 * @note @a item is considered to be inside @a list. No checks are
 *       done to confirm this, and giving nodes from different lists
 *       will lead to problems.
 *
 * @param list existing list head or NULL to create a new list.
 * @param item list node to move to end (tail), must not be NULL.
 *
 * @return the new list head. Use it and not @a list anymore.
 */
EAPI Eina_Inlist   *eina_inlist_demote(Eina_Inlist *list,
                                       Eina_Inlist *item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the count of the number of items in a list.
 *
 * @param list The list whose count to return.
 * @return The number of members in the list.
 *
 * This function returns how many members @p list contains. If the
 * list is @c NULL, 0 is returned.
 *
 * @warning This is an order-N operation and so the time will depend
 *    on the number of elements on the list, so, it might become
 *    slow for big lists!
 */
EAPI unsigned int   eina_inlist_count(const Eina_Inlist *list) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returns a new iterator associated to @a list.
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
 * If the memory can not be allocated, NULL is returned and
 * #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 * returned.
 *
 * @warning if the list structure changes then the iterator becomes
 *    invalid, and if you add or remove nodes iterator
 *    behavior is undefined, and your program may crash!
 */
EAPI Eina_Iterator *eina_inlist_iterator_new(const Eina_Inlist *in_list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new accessor associated to a list.
 *
 * @param list The list.
 * @return A new accessor.
 *
 * This function returns a newly allocated accessor associated to
 * @p list. If @p list is @c NULL or the count member of @p list is
 * less or equal than 0, this function returns NULL. If the memory can
 * not be allocated, NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 * set. Otherwise, a valid accessor is returned.
 */
EAPI Eina_Accessor *eina_inlist_accessor_new(const Eina_Inlist *in_list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Insert a new node into a sorted list.
 *
 * @param list The given linked list, @b must be sorted.
 * @param item list node to insert, must not be NULL.
 * @param func The function called for the sort.
 * @return A list pointer.
 *
 * This function inserts item into a linked list assuming it was
 * sorted and the result will be sorted. If @p list is @c NULLL, item
 * is returned. On success, a new list pointer that should be
 * used in place of the one given to this function is
 * returned. Otherwise, the old pointer is returned. See eina_error_get().
 *
 * @note O(log2(n)) comparisons (calls to @p func) average/worst case
 * performance as it uses eina_list_search_sorted_near_list() and thus
 * is bounded to that. As said in eina_list_search_sorted_near_list(),
 * lists do not have O(1) access time, so walking to the correct node
 * can be costly, consider worst case to be almost O(n) pointer
 * dereference (list walk).
 */
EAPI Eina_Inlist *eina_inlist_sorted_insert(Eina_Inlist *list, Eina_Inlist *item, Eina_Compare_Cb func) EINA_ARG_NONNULL(2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Sort a list according to the ordering func will return.
 *
 * @param list The list handle to sort.
 * @param func A function pointer that can handle comparing the list data
 * nodes.
 * @return the new head of list.
 *
 * This function sorts all the elements of @p list. @p func is used to
 * compare two elements of @p list. If @p list or @p func are @c NULL,
 * this function returns @c NULL.
 *
 * @note @b in-place: this will change the given list, so you should
 * now point to the new list head that is returned by this function.
 *
 * @note worst case is O(n * log2(n)) comparisons (calls to func()),
 * O(n) comparisons average case. That means that for 1,000,000 list
 * elements, sort will usually do 1,000,000 comparisons, but may do up
 * to 20,000,000.
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
 */
EAPI Eina_Inlist *eina_inlist_sort(Eina_Inlist *head, Eina_Compare_Cb func);

/* This two macros are helpers for the _FOREACH ones, don't use them */
#define _EINA_INLIST_OFFSET(ref)         ((char *)&(ref)->__in_list - (char *)(ref))
#define _EINA_INLIST_CONTAINER(ref, ptr) (void *)((char *)(ptr) - \
                                                  _EINA_INLIST_OFFSET(ref))

#define EINA_INLIST_FOREACH(list, l)                                     \
  for (l = NULL, l = (list ? _EINA_INLIST_CONTAINER(l, list) : NULL); l; \
       l = (EINA_INLIST_GET(l)->next ? _EINA_INLIST_CONTAINER(l, EINA_INLIST_GET(l)->next) : NULL))
#define EINA_INLIST_FOREACH_SAFE(list, list2, l) \
   for (l = (list ? _EINA_INLIST_CONTAINER(l, list) : NULL), list2 = l ? ((EINA_INLIST_GET(l) ? EINA_INLIST_GET(l)->next : NULL)) : NULL; \
        l; \
        l = _EINA_INLIST_CONTAINER(l, list2), list2 = list2 ? list2->next : NULL)
#define EINA_INLIST_REVERSE_FOREACH(list, l)                                \
  for (l = NULL, l = (list ? _EINA_INLIST_CONTAINER(l, list->last) : NULL); \
       l; l = (EINA_INLIST_GET(l)->prev ? _EINA_INLIST_CONTAINER(l, EINA_INLIST_GET(l)->prev) : NULL))

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /*EINA_INLIST_H_*/
