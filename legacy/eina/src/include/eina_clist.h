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

/*
 * Eina_Clist is a compact inline list implementation
 *
 * Advantages over Eina_List and Eina_Inlist:
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
 * Things to note:
 *  - there's no NULL at the end of the list, the last item points to the head
 */

/* Define a list like so:
 *
 *   struct gadget
 *   {
 *       struct Eina_Clist  entry;   <-- doesn't have to be the first item in the struct
 *       int                a, b;
 *   };
 *
 *   static Eina_Clist global_gadgets = EINA_CLIST_INIT( global_gadgets );
 *
 * or
 *
 *   struct some_global_thing
 *   {
 *       Eina_Clist gadgets;
 *   };
 *
 *   eina_clist_init( &some_global_thing->gadgets );
 *
 * Manipulate it like this:
 *
 *   eina_clist_add_head( &global_gadgets, &new_gadget->entry );
 *   eina_clist_remove( &new_gadget->entry );
 *   eina_clist_add_after( &some_random_gadget->entry, &new_gadget->entry );
 *
 * And to iterate over it:
 *
 *   struct gadget *gadget;
 *   EINA_CLIST_FOR_EACH_ENTRY( gadget, &global_gadgets, struct gadget, entry )
 *   {
 *       ...
 *   }
 *
 */

typedef struct _Eina_Clist Eina_Clist;
struct _Eina_Clist
{
   Eina_Clist *next;
   Eina_Clist *prev;
};

/* add an element after the specified one */
static inline void eina_clist_add_after(Eina_Clist *elem, Eina_Clist *to_add)
{
   to_add->next = elem->next;
   to_add->prev = elem;
   elem->next->prev = to_add;
   elem->next = to_add;
}

/* add an element before the specified one */
static inline void eina_clist_add_before(Eina_Clist *elem, Eina_Clist *to_add)
{
   to_add->next = elem;
   to_add->prev = elem->prev;
   elem->prev->next = to_add;
   elem->prev = to_add;
}

/* add element at the head of the list */
static inline void eina_clist_add_head(Eina_Clist *list, Eina_Clist *elem)
{
   eina_clist_add_after(list, elem);
}

/* add element at the tail of the list */
static inline void eina_clist_add_tail(Eina_Clist *list, Eina_Clist *elem)
{
   eina_clist_add_before(list, elem);
}

/* init an (unlinked) element */
static inline void eina_clist_element_init(Eina_Clist *elem)
{
   elem->next = NULL;
   elem->next = NULL;
}

/* check if an element is in a list or not */
static inline int eina_clist_element_is_linked(Eina_Clist *elem)
{
   return (elem->next != NULL && elem->prev != NULL);
}

/* remove an element from its list */
static inline void eina_clist_remove(Eina_Clist *elem)
{
   elem->next->prev = elem->prev;
   elem->prev->next = elem->next;
   eina_clist_element_init(elem);
}

/* get the next element */
static inline Eina_Clist *eina_clist_next(const Eina_Clist *list, const Eina_Clist *elem)
{
   Eina_Clist *ret = elem->next;
   if (elem->next == list) ret = NULL;
   return ret;
}

/* get the previous element */
static inline Eina_Clist *eina_clist_prev(const Eina_Clist *list, const Eina_Clist *elem)
{
   Eina_Clist *ret = elem->prev;
   if (elem->prev == list) ret = NULL;
   return ret;
}

/* get the first element */
static inline Eina_Clist *eina_clist_head(const Eina_Clist *list)
{
   return eina_clist_next(list, list);
}

/* get the last element */
static inline Eina_Clist *eina_clist_tail(const Eina_Clist *list)
{
   return eina_clist_prev(list, list);
}

/* check if a list is empty */
static inline int eina_clist_empty(const Eina_Clist *list)
{
   return list->next == list;
}

/* initialize a list */
static inline void eina_clist_init(Eina_Clist *list)
{
   list->next = list->prev = list;
}

/* count the elements of a list */
static inline unsigned int eina_clist_count(const Eina_Clist *list)
{
   unsigned count = 0;
   const Eina_Clist *ptr;
   for (ptr = list->next; ptr != list; ptr = ptr->next) count++;
   return count;
}

/* move all elements from src to the tail of dst */
static inline void eina_clist_move_tail(Eina_Clist *dst, Eina_Clist *src)
{
   if (eina_clist_empty(src)) return;

   dst->prev->next = src->next;
   src->next->prev = dst->prev;
   dst->prev = src->prev;
   src->prev->next = dst;
   eina_clist_init(src);
}

/* move all elements from src to the head of dst */
static inline void eina_clist_move_head(Eina_Clist *dst, Eina_Clist *src)
{
   if (eina_clist_empty(src)) return;

   dst->next->prev = src->prev;
   src->prev->next = dst->next;
   dst->next = src->next;
   src->next->prev = dst;
   eina_clist_init(src);
}

/* iterate through the list */
#define EINA_CLIST_FOR_EACH(cursor,list) \
    for ((cursor) = (list)->next; (cursor) != (list); (cursor) = (cursor)->next)

/* iterate through the list, with safety against removal */
#define EINA_CLIST_FOR_EACH_SAFE(cursor, cursor2, list) \
    for ((cursor) = (list)->next, (cursor2) = (cursor)->next; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->next)

/* iterate through the list using a list entry */
#define EINA_CLIST_FOR_EACH_ENTRY(elem, list, type, field) \
    for ((elem) = EINA_CLIST_ENTRY((list)->next, type, field); \
         &(elem)->field != (list); \
         (elem) = EINA_CLIST_ENTRY((elem)->field.next, type, field))

/* iterate through the list using a list entry, with safety against removal */
#define EINA_CLIST_FOR_EACH_ENTRY_SAFE(cursor, cursor2, list, type, field) \
    for ((cursor) = EINA_CLIST_ENTRY((list)->next, type, field), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.next, type, field); \
         &(cursor)->field != (list); \
         (cursor) = (cursor2), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.next, type, field))

/* iterate through the list in reverse order */
#define EINA_CLIST_FOR_EACH_REV(cursor,list) \
    for ((cursor) = (list)->prev; (cursor) != (list); (cursor) = (cursor)->prev)

/* iterate through the list in reverse order, with safety against removal */
#define EINA_CLIST_FOR_EACH_SAFE_REV(cursor, cursor2, list) \
    for ((cursor) = (list)->prev, (cursor2) = (cursor)->prev; \
         (cursor) != (list); \
         (cursor) = (cursor2), (cursor2) = (cursor)->prev)

/* iterate through the list in reverse order using a list entry */
#define EINA_CLIST_FOR_EACH_ENTRY_REV(elem, list, type, field) \
    for ((elem) = EINA_CLIST_ENTRY((list)->prev, type, field); \
         &(elem)->field != (list); \
         (elem) = EINA_CLIST_ENTRY((elem)->field.prev, type, field))

/* iterate through the list in reverse order using a list entry, with safety against removal */
#define EINA_CLIST_FOR_EACH_ENTRY_SAFE_REV(cursor, cursor2, list, type, field) \
    for ((cursor) = EINA_CLIST_ENTRY((list)->prev, type, field), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.prev, type, field); \
         &(cursor)->field != (list); \
         (cursor) = (cursor2), \
         (cursor2) = EINA_CLIST_ENTRY((cursor)->field.prev, type, field))

/* macros for statically initialized lists */
#undef EINA_CLIST_INIT
#define EINA_CLIST_INIT(list)  { &(list), &(list) }

/* get pointer to object containing list element */
#undef EINA_CLIST_ENTRY
#define EINA_CLIST_ENTRY(elem, type, field) \
    ((type *)((char *)(elem) - (unsigned long)(&((type *)0)->field)))

#endif /* __EINA_CLIST_H__ */
