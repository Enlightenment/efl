/*
 * Linked lists support
 *
 * Copyright (C) 2002 Alexandre Julliard
 * Copyright (C) 2011 Mike McCormack (adapted for Eina)
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __EINA_CLIST_H__
#define __EINA_CLIST_H__

/**
 * @defgroup Eina_CList_Group Compact List
 * @ingroup Eina_Containers_Group
 *
 * @{
 *
 * @brief Eina_Clist is a compact (inline) list implementation.
 *
 * Elements of this list are members of the structs stored in the list.
 *
 * Advantages over @ref Eina_List and @ref Eina_Inlist :
 * - Uses less memory (two machine words per item).
 * - Allows removing items without knowing which list they're in while using O(1) time.
 * - Doesn't need to keep updating the head pointer as the list is changed.
 *
 * Disadvantages:
 * - O(N) time to calculate the list length.
 * - Requires one list entry in a struct per list (i.e. it's an inlist).
 * - Requires a head/tail pointer.
 * - Needs to know the list head when moving to the next or previous pointer.
 *
 * @note There's no @c NULL at the end of the list, the last item points to the head.
 *
 * @note List heads must be initialized with EINA_CLIST_INIT or by calling eina_clist_element_init.
 *
 * Define a list as follows:
 *
 * @code
 *   struct gadget
 *   {
 *       struct Eina_Clist  entry;   <-- doesn't have to be the first item in the struct
 *       int                a, b;
 *   };
 *
 *   static Eina_Clist global_gadgets = EINA_CLIST_INIT( global_gadgets );
 * @endcode
 *
 * or
 *
 * @code
 *   struct some_global_thing
 *   {
 *       Eina_Clist gadgets;
 *   };
 *
 *   eina_clist_init( &some_global_thing->gadgets );
 * @endcode
 *
 * Manipulate it like this:
 *
 * @code
 *   eina_clist_add_head( &global_gadgets, &new_gadget->entry );
 *   eina_clist_remove( &new_gadget->entry );
 *   eina_clist_add_after( &some_random_gadget->entry, &new_gadget->entry );
 * @endcode
 *
 * And to iterate over it:
 *
 * @code
 *   struct gadget *gadget;
 *   EINA_CLIST_FOR_EACH_ENTRY( gadget, &global_gadgets, struct gadget, entry )
 *   {
 *       ...
 *   }
 * @endcode
 *
 */

/**
 * @typedef Eina_Clist
 * @brief Type for #_Eina_Clist structure containing the list head and the list entry.
 * @since 1.1.0
 */
typedef struct _Eina_Clist Eina_Clist;

/**
 * @struct _Eina_Clist
 * @brief The structure type for a compact list type.
 *
 * @note This structure is used as both the list head and the list entry.
 *
 * @since 1.1.0
 */
struct _Eina_Clist
{
   Eina_Clist *next; /**< The next entry in the list */
   Eina_Clist *prev; /**< The previous entry in the list */
};

/**
 * @brief Adds an element after the specified one.
 *
 * @param[in,out] elem An element in the list
 * @param[in] to_add The element to add to the list
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There is no need to initialize an element before adding it to the list.
 *
 * @since 1.1.0
 */
static inline void eina_clist_add_after(Eina_Clist *elem, Eina_Clist *to_add);

/**
 * @brief Adds an element before the specified one.
 *
 * @param[in,out] elem An element in the list
 * @param[in] to_add The element to add to the list
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There is no need to initialize an element before adding it to the list.
 *
 * @since 1.1.0
 */
static inline void eina_clist_add_before(Eina_Clist *elem, Eina_Clist *to_add);

/**
 * @brief Adds an element to the head of the list.
 *
 * @param[in,out] list The list
 * @param[in] elem An element
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There is no need to initialize an element before adding it to the list.
 *
 * @since 1.1.0
 */
static inline void eina_clist_add_head(Eina_Clist *list, Eina_Clist *elem);

/**
 * @brief Adds an element at the tail of the list.
 *
 * @param[in,out] list The list
 * @param[in] elem An element
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There is no need to initialize an element before adding it to the list.
 *
 * @since 1.1.0
 */
static inline void eina_clist_add_tail(Eina_Clist *list, Eina_Clist *elem);

/**
 * @brief Inits an (unlinked) element.
 * @details This function is called on elements that have not been added to the list
 *          so that eina_clist_element_init() works correctly.
 *
 * @param[in,out] elem An element
 * @pre The element is not in any list.
 * @post The element is marked as not being in any list.
 *
 * @note It is not necessary to call this before adding an element to this list.
 *
 * @since 1.1.0
 */
static inline void eina_clist_element_init(Eina_Clist *elem);

/**
 * @brief Checks whether an element is in a list.
 *
 * @param[in] elem An element
 * @return TRUE if the element is in a list, else FALSE.
 * @pre Either eina_clist_element_init() has been called on @p elem,
 *      or it has been added to a list or removed from a list.
 *
 * @since 1.1.0
 */
static inline int eina_clist_element_is_linked(Eina_Clist *elem);

/**
 * @brief Removes an element from its list.
 *
 * @param[in,out] elem An element
 * @pre The element is already in a list.
 * @post The element is marked as not being in any list.
 *
 * @since 1.1.0
 */
static inline void eina_clist_remove(Eina_Clist *elem);

/**
 * @brief Gets the next element.
 *
 * @param[in] list The list
 * @param[in] elem An element
 * @return The element after @p elem in @p list, otherwise @c NULL if @p elem is last in @p list
 * @pre @p elem is in @p list.
 *
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_next(const Eina_Clist *list, const Eina_Clist *elem);

/**
 * @brief Gets the previous element.
 *
 * @param[in] list The list
 * @param[in] elem An element
 *
 * @return The element before @p elem, otherwise @c NULL if @p elem is first in @p list
 *
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_prev(const Eina_Clist *list, const Eina_Clist *elem);

/**
 * @brief Gets the first element.
 *
 * @param[in] list The list
 * @return The first element in @p list, otherwise @c NULL if @p list is empty
 *
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_head(const Eina_Clist *list);

/**
 * @brief Gets the last element.
 *
 * @param[in] list The list
 * @return The last element in @p list, otherwise @c NULL if @p list is empty
 *
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_tail(const Eina_Clist *list);

/**
 * @brief Checks whether a list is empty.
 *
 * @param[in] list The list
 * @return A non-zero value if @p list is empty, otherwise zero if it is not
 *
 * @since 1.1.0
 */
static inline int eina_clist_empty(const Eina_Clist *list);

/**
 * @brief Initializes a list.
 *
 * @param[in,out] list The list
 * @pre The list is uninitialized
 * @post The list contains no items
 *
 * @note Don't call this function on a list with items
 * @note This function must be called. Don't try to
 *       initialize the list by zeroing out the list head.
 *
 * @since 1.1.0
 */
static inline void eina_clist_init(Eina_Clist *list);

/**
 * @brief Counts the elements of a list.
 *
 * @param[in] list The list
 * @return The number of items in the list
 *
 * @since 1.1.0
 */
static inline unsigned int eina_clist_count(const Eina_Clist *list);

/**
 * @brief Moves all elements from @p src to the tail of @p dst.
 *
 * @param[in,out] dst The list to be appended to
 * @param[in] src The list to append
 *
 * @post @p src is initialized, but is empty after this operation.
 *
 * @since 1.1.0
 */
static inline void eina_clist_move_tail(Eina_Clist *dst, Eina_Clist *src);

/**
 * @brief Moves all elements from @p src to the head of @p dst.
 *
 * @param[in,out] dst The list to be prepended to
 * @param[in] src The list to prepend
 *
 * @post @p src is initialized, but is empty after this operation.
 *
 * @since 1.1.0
 */
static inline void eina_clist_move_head(Eina_Clist *dst, Eina_Clist *src);

/**
 * @def EINA_CLIST_FOR_EACH
 * @brief Iterates through the list.
 *
 * @param[out] cursor The pointer to be used during the interaction
 * @param[in] list The list to be interacted with
 */
#define EINA_CLIST_FOR_EACH(cursor,list) \
    for ((cursor) = (list)->next; (cursor) != (list); (cursor) = (cursor)->next)

/**
 * @def EINA_CLIST_FOR_EACH_SAFE
 * @brief Iterates through the list, with safety against removal.
 *
 * @param[out] cursor The pointer to be used during the interaction
 * @param[out] cursor2 The auxiliary pointer to be used during the interaction
 * @param[in] list The list to be interacted with
 */
#define EINA_CLIST_FOR_EACH_SAFE(cursor, cursor2, list) \
    for ((cursor) = (list)->next, (cursor2) = (cursor)->next; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->next)

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY
 * @brief Iterates through the list using a list entry.
 *
 * @param[out] elem The element to be used
 * @param[in] list The list to be iterated
 * @param[in] type The type of the list
 * @param[in] field The field of the element
 */
#define EINA_CLIST_FOR_EACH_ENTRY(elem, list, type, field) \
    for ((elem) = EINA_CLIST_ENTRY((list)->next, type, field); \
         &(elem)->field != (list); \
         (elem) = EINA_CLIST_ENTRY((elem)->field.next, type, field))

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY_SAFE
 * @brief Iterates through the list using a list entry, with safety against removal.
 *
 * @param[out] cursor The pointer to be used during the interaction
 * @param[out] cursor2 The auxiliary pointer to be used during the interaction
 * @param[in] list The list to be interacted with
 * @param[in] type The type of the list
 * @param[in] field The field of the element
*/
#define EINA_CLIST_FOR_EACH_ENTRY_SAFE(cursor, cursor2, list, type, field) \
    for ((cursor) = EINA_CLIST_ENTRY((list)->next, type, field), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.next, type, field); \
         &(cursor)->field != (list); \
         (cursor) = (cursor2), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.next, type, field))

/**
 * @def EINA_CLIST_FOR_EACH_REV
 * @brief Iterates through the list in the reverse order.
 *
 * @param[out] cursor The pointer to be used during the interaction
 * @param[in] list The list to be interacted with
 */
#define EINA_CLIST_FOR_EACH_REV(cursor,list) \
    for ((cursor) = (list)->prev; (cursor) != (list); (cursor) = (cursor)->prev)

/**
 * @def EINA_CLIST_FOR_EACH_SAFE_REV
 * @brief Iterates through the list in the reverse order, with safety against removal.
 *
 * @param[out] cursor The pointer to be used during the interaction
 * @param[out] cursor2 The auxiliary pointer to be used during the interaction
 * @param[in] list The list to be interacted with
 */
#define EINA_CLIST_FOR_EACH_SAFE_REV(cursor, cursor2, list) \
    for ((cursor) = (list)->prev, (cursor2) = (cursor)->prev; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->prev)

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY_REV
 * @brief Iterates through the list in the reverse order using a list entry.
 *
 * @param[out] elem The element to be used
 * @param[in] list The list to be iterated
 * @param[in] type The type of the list
 * @param[in] field The field of the element
 */
#define EINA_CLIST_FOR_EACH_ENTRY_REV(elem, list, type, field) \
    for ((elem) = EINA_CLIST_ENTRY((list)->prev, type, field); \
         &(elem)->field != (list); \
         (elem) = EINA_CLIST_ENTRY((elem)->field.prev, type, field))

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY_SAFE_REV
 * @brief Iterates through the list in the reverse order using a list entry, with safety against
 *        removal.
 *
 * @param[out] cursor The pointer to be used during the interaction
 * @param[out] cursor2 The auxiliary pointer to be used during the interaction
 * @param[in] list The list to be interacted with
 * @param[in] type The type of the list
 * @param[in] field The field of the element
 */
#define EINA_CLIST_FOR_EACH_ENTRY_SAFE_REV(cursor, cursor2, list, type, field) \
    for ((cursor) = EINA_CLIST_ENTRY((list)->prev, type, field), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.prev, type, field); \
         &(cursor)->field != (list); \
         (cursor) = (cursor2), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.prev, type, field))

/**
 * @def EINA_CLIST_INIT
 * @brief Macros for statically initialized lists.
 *
 * @param[in,out] list The list to be used
 */
#undef EINA_CLIST_INIT
#define EINA_CLIST_INIT(list)  { &(list), &(list) }

/**
 * @def EINA_CLIST_ENTRY
 * @brief Gets a pointer to the object containing the list element.
 *
 * @param[out] elem The element to be used
 * @param[in] type The type of the element
 * @param[in] field The field of the element
 */
#undef EINA_CLIST_ENTRY
#define EINA_CLIST_ENTRY(elem, type, field) \
    ((type *)((char *)(elem) - (uintptr_t)(&((type *)0)->field)))

#include "eina_inline_clist.x"

/**
 * @}
 */

#endif /* __EINA_CLIST_H__ */
