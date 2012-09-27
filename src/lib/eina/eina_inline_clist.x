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

#ifndef __EINA_CLIST_INLINE_H__
#define __EINA_CLIST_INLINE_H__

#include <stddef.h>

static inline void eina_clist_add_after(Eina_Clist *elem, Eina_Clist *to_add)
{
   to_add->next = elem->next;
   to_add->prev = elem;
   elem->next->prev = to_add;
   elem->next = to_add;
}

static inline void eina_clist_add_before(Eina_Clist *elem, Eina_Clist *to_add)
{
   to_add->next = elem;
   to_add->prev = elem->prev;
   elem->prev->next = to_add;
   elem->prev = to_add;
}

static inline void eina_clist_add_head(Eina_Clist *list, Eina_Clist *elem)
{
   eina_clist_add_after(list, elem);
}

static inline void eina_clist_add_tail(Eina_Clist *list, Eina_Clist *elem)
{
   eina_clist_add_before(list, elem);
}

static inline void eina_clist_element_init(Eina_Clist *elem)
{
   elem->next = NULL;
   elem->prev = NULL;
}

static inline int eina_clist_element_is_linked(Eina_Clist *elem)
{
   return (elem->next != NULL && elem->prev != NULL);
}

static inline void eina_clist_remove(Eina_Clist *elem)
{
   elem->next->prev = elem->prev;
   elem->prev->next = elem->next;
   eina_clist_element_init(elem);
}

static inline Eina_Clist *eina_clist_next(const Eina_Clist *list, const Eina_Clist *elem)
{
   Eina_Clist *ret = elem->next;
   if (elem->next == list) ret = NULL;
   return ret;
}

static inline Eina_Clist *eina_clist_prev(const Eina_Clist *list, const Eina_Clist *elem)
{
   Eina_Clist *ret = elem->prev;
   if (elem->prev == list) ret = NULL;
   return ret;
}

static inline Eina_Clist *eina_clist_head(const Eina_Clist *list)
{
   return eina_clist_next(list, list);
}

static inline Eina_Clist *eina_clist_tail(const Eina_Clist *list)
{
   return eina_clist_prev(list, list);
}

static inline int eina_clist_empty(const Eina_Clist *list)
{
   return list->next == list;
}

static inline void eina_clist_init(Eina_Clist *list)
{
   list->next = list->prev = list;
}

static inline unsigned int eina_clist_count(const Eina_Clist *list)
{
   unsigned count = 0;
   const Eina_Clist *ptr;
   for (ptr = list->next; ptr != list; ptr = ptr->next) count++;
   return count;
}

static inline void eina_clist_move_tail(Eina_Clist *dst, Eina_Clist *src)
{
   if (eina_clist_empty(src)) return;

   dst->prev->next = src->next;
   src->next->prev = dst->prev;
   dst->prev = src->prev;
   src->prev->next = dst;
   eina_clist_init(src);
}

static inline void eina_clist_move_head(Eina_Clist *dst, Eina_Clist *src)
{
   if (eina_clist_empty(src)) return;

   dst->next->prev = src->prev;
   src->prev->next = dst->next;
   dst->next = src->next;
   src->next->prev = dst;
   eina_clist_init(src);
}

#endif
