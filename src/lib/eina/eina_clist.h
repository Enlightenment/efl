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
 * @defgroup Eina_CList_Group Compact List
 *
 * @{
 *
 * @brief Eina_Clist is a compact (inline) list implementation
 *
 * Elements of this list are members of the structs stored in the list
 *
 * Advantages over @ref Eina_List and @ref Eina_Inlist :
 *  - uses less memory (two machine words per item)
 *  - allows removing items without knowing which list they're in using O(1) time
 *  - no need to keep updating the head pointer as the list is changed
 *
 * Disadvantages:
 *  - O(N) time to calculate list length
 *  - requires one list entry in a struct per list (i.e. it's an inlist)
 *  - requires a head/tail pointer
 *  - need to know the list head when moving to next or previous pointer
 *
 * @note There's no NULL at the end of the list, the last item points to the head.
 *
 * @note List heads must be initialized with EINA_CLIST_INIT or by calling eina_clist_element_init
 *
 * Define a list like so:
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
 * This is the list head and the list entry.
 * @since 1.1.0
 */
typedef struct _Eina_Clist Eina_Clist;

/**
 * @struct _Eina_Clist
 * Compact list type
 * @note This structure is used as both the list head and the list entry.
 * @since 1.1.0
 */
struct _Eina_Clist
{
   Eina_Clist *next;
   Eina_Clist *prev;
};

/**
 * Add an element after the specified one.
 *
 * @param elem An element in the list
 * @param to_add The element to add to the list
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There's no need to initialize an element before adding it to the list.
 * @since 1.1.0
 */
static inline void eina_clist_add_after(Eina_Clist *elem, Eina_Clist *to_add);

/**
 * Add an element before the specified one.
 *
 * @param elem An element in the list
 * @param to_add The element to add to the list
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There's no need to initialize an element before adding it to the list.
 * @since 1.1.0
 */
static inline void eina_clist_add_before(Eina_Clist *elem, Eina_Clist *to_add);

/**
 * Add element at the head of the list.
 *
 * @param list The list
 * @param elem An element
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There's no need to initialize an element before adding it to the list.
 * @since 1.1.0
 */
static inline void eina_clist_add_head(Eina_Clist *list, Eina_Clist *elem);

/**
 * Add element at the tail of the list.
 *
 * @param list The list
 * @param elem An element
 * @pre The list head must be initialized once before adding anything.
 * @pre The element is not in any list.
 *
 * @note There's no need to initialize an element before adding it to the list.
 * @since 1.1.0
 */
static inline void eina_clist_add_tail(Eina_Clist *list, Eina_Clist *elem);

/**
 * Init an (unlinked) element.
 *
 * Call this function on elements that have not been added to the list
 * if you want eina_clist_element_init() to work correctly
 *
 * @param elem An element
 * @pre The element is not in any list.
 * @post The element is marked as not being in any list
 *
 * @note It is not necessary to call this before adding an element to this list.
 * @since 1.1.0
 */
static inline void eina_clist_element_init(Eina_Clist *elem);

/**
 * Check if an element is in a list or not.
 *
 * @param elem An element
 *
 * @pre Either eina_clist_element_init() has been called on @a elem,
 *      it has been added to a list or remove from a list.
 * @since 1.1.0
 */
static inline int eina_clist_element_is_linked(Eina_Clist *elem);

/**
 * Remove an element from its list.
 *
 * @param elem An element
 * @pre The element is in a list already
 * @post The element is marked as not being in any list
 * @since 1.1.0
 */
static inline void eina_clist_remove(Eina_Clist *elem);

/**
 * Get the next element.
 *
 * @param list The list
 * @param elem An element
 * @pre @a elem is in @a list
 * @return The element after @a elem in @a list or @c NULL if @a elem is last in @a list.
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_next(const Eina_Clist *list, const Eina_Clist *elem);

/**
 * Get the previous element.
 *
 * @param list The list
 * @param elem An element
 *
 * @return The element before @a elem or @c NULL if @a elem is the first in the list.
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_prev(const Eina_Clist *list, const Eina_Clist *elem);

/**
 * Get the first element.
 *
 * @param list The list
 * @returns The first element in @a list or @c NULL if @a list is empty.
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_head(const Eina_Clist *list);

/**
 * Get the last element.
 *
 * @param list The list
 * @returns The last element in @a list or @c NULL if @a list is empty.
 * @since 1.1.0
 */
static inline Eina_Clist *eina_clist_tail(const Eina_Clist *list);

/**
 * Check if a list is empty.
 *
 * @param list The list
 * @returns non-zero if @a list is empty, zero if it is not
 * @since 1.1.0
 */
static inline int eina_clist_empty(const Eina_Clist *list);

/**
 * Initialize a list
 *
 * @param list The list
 * @pre The list is uninitialized
 * @post The list contains no items
 *
 * @note Don't call this function on a list with items
 * @note This function must be called.  Don't try do
 * initialize the list by zero'ing out the list head.
 * @since 1.1.0
 */
static inline void eina_clist_init(Eina_Clist *list);

/**
 * Count the elements of a list
 *
 * @param list The list
 * @returns The number of items in the list
 * @since 1.1.0
 */
static inline unsigned int eina_clist_count(const Eina_Clist *list);

/**
 * Move all elements from src to the tail of dst
 *
 * @param dst List to be appended to
 * @param src List to append
 *
 * @post @a src is initialized but empty after this operation
 * @since 1.1.0
 */
static inline void eina_clist_move_tail(Eina_Clist *dst, Eina_Clist *src);

/**
 * move all elements from src to the head of dst
 *
 * @param dst List to be prepended to
 * @param src List to prepend
 *
 * @post @a src is initialized but empty after this operation
 * @since 1.1.0
 */
static inline void eina_clist_move_head(Eina_Clist *dst, Eina_Clist *src);

/**
 * @def EINA_CLIST_FOR_EACH
 * @brief Iterate through the list.
 * @param cursor The pointer to be used during the interation.
 * @param list The list to be interated.
 */
#define EINA_CLIST_FOR_EACH(cursor,list) \
    for ((cursor) = (list)->next; (cursor) != (list); (cursor) = (cursor)->next)

/**
 * @def EINA_CLIST_FOR_EACH_SAFE
 * @brief Iterate through the list, with safety against removal.
 * @param cursor The pointer to be used during the interation.
 * @param cursor2 The auxiliar pointer to be used during the interation.
 * @param list The list to be interated.
 */
#define EINA_CLIST_FOR_EACH_SAFE(cursor, cursor2, list) \
    for ((cursor) = (list)->next, (cursor2) = (cursor)->next; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->next)

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY
 * @brief Iterate through the list using a list entry.
 * @param elem The element to be used.
 * @param list The list to be iterated.
 * @param type The type of the list.
 * @param field The field of the element.
 */
#define EINA_CLIST_FOR_EACH_ENTRY(elem, list, type, field) \
    for ((elem) = EINA_CLIST_ENTRY((list)->next, type, field); \
         &(elem)->field != (list); \
         (elem) = EINA_CLIST_ENTRY((elem)->field.next, type, field))

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY_SAFE
 * @brief Iterate through the list using a list entry, with safety against removal.
 * @param cursor The pointer to be used during the interation.
 * @param cursor2 The auxiliar pointer to be used during the interation.
 * @param list The list to be interated.
 * @param type The type of the list.
 * @param field The field of the element.
*/
#define EINA_CLIST_FOR_EACH_ENTRY_SAFE(cursor, cursor2, list, type, field) \
    for ((cursor) = EINA_CLIST_ENTRY((list)->next, type, field), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.next, type, field); \
         &(cursor)->field != (list); \
         (cursor) = (cursor2), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.next, type, field))

/**
 * @def EINA_CLIST_FOR_EACH_REV
 * @brief Iterate through the list in reverse order.
 * @param cursor The pointer to be used during the interation.
 * @param list The list to be interated.
 */
#define EINA_CLIST_FOR_EACH_REV(cursor,list) \
    for ((cursor) = (list)->prev; (cursor) != (list); (cursor) = (cursor)->prev)

/**
 * @def EINA_CLIST_FOR_EACH_SAFE_REV
 * @brief Iterate through the list in reverse order, with safety against removal.
 * @param cursor The pointer to be used during the interation.
 * @param cursor2 The auxiliar pointer to be used during the interation.
 * @param list The list to be interated.
 */
#define EINA_CLIST_FOR_EACH_SAFE_REV(cursor, cursor2, list) \
    for ((cursor) = (list)->prev, (cursor2) = (cursor)->prev; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->prev)

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY_REV
 * @brief Iterate through the list in reverse order using a list entry.
 * @param elem The element to be used.
 * @param list The list to be iterated.
 * @param type The type of the list.
 * @param field The field of the element.
 */
#define EINA_CLIST_FOR_EACH_ENTRY_REV(elem, list, type, field) \
    for ((elem) = EINA_CLIST_ENTRY((list)->prev, type, field); \
         &(elem)->field != (list); \
         (elem) = EINA_CLIST_ENTRY((elem)->field.prev, type, field))

/**
 * @def EINA_CLIST_FOR_EACH_ENTRY_SAFE_REV
 * @brief Iterate through the list in reverse order using a list entry, with safety against
 * removal.
 * @param cursor The pointer to be used during the interation.
 * @param cursor2 The auxiliar pointer to be used during the interation.
 * @param list The list to be interated.
 * @param type The type of the list.
 * @param field The field of the element.
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
 * @param list The list to be used.
 */
#undef EINA_CLIST_INIT
#define EINA_CLIST_INIT(list)  { &(list), &(list) }

/**
 * @def EINA_CLIST_ENTRY
 * @brief Get pointer to object containing list element.
 * @param elem The element to be used.
 * @param type The type of the element.
 * @param field The field of the element.
 */
#undef EINA_CLIST_ENTRY
#define EINA_CLIST_ENTRY(elem, type, field) \
    ((type *)((char *)(elem) - (uintptr_t)(&((type *)0)->field)))

#include "eina_inline_clist.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* __EINA_CLIST_H__ */
