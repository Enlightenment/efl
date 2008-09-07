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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <assert.h>

#include "eina_inlist.h"
#include "eina_error.h"
#include "eina_private.h"

/* FIXME: TODO please, refactor this :) */

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

typedef struct _Eina_Iterator_Inlist Eina_Iterator_Inlist;
typedef struct _Eina_Accessor_Inlist Eina_Accessor_Inlist;

struct _Eina_Iterator_Inlist
{
	Eina_Iterator iterator;
	const Eina_Inlist *head;
	const Eina_Inlist *current;
};

struct _Eina_Accessor_Inlist
{
	Eina_Accessor accessor;

	const Eina_Inlist *head;
	const Eina_Inlist *current;

	unsigned int index;
};

static Eina_Bool
eina_inlist_iterator_next(Eina_Iterator_Inlist *it, void **data) {
	if (it->current == NULL) return EINA_FALSE;
	if (data) *data = (void*) it->current;

	it->current = it->current->next;

	return EINA_TRUE;
}

static Eina_Inlist *
eina_inlist_iterator_get_container(Eina_Iterator_Inlist *it) {
	return (Eina_Inlist*) it->head;
}

static void
eina_inlist_iterator_free(Eina_Iterator_Inlist *it) {
	free(it);
}

static Eina_Bool
eina_inlist_accessor_get_at(Eina_Accessor_Inlist *it, unsigned int index, void **data) {
	const Eina_Inlist *over;
	unsigned int middle;
	unsigned int i;

	if (it->index == index) {
		over = it->current;
	} else if (index > it->index) {
		/* Looking after current. */
		for (i = it->index, over = it->current;
		     i < index && over != NULL;
		     ++i, over = over->next)
			;

	} else {
		middle = it->index >> 1;

		if (index > middle) {
			/* Looking backward from current. */
			for (i = it->index, over = it->current;
			     i > index && over != NULL;
			     --i, over = over->prev)
				;
		} else {
			/* Looking from the start. */
			for (i = 0, over = it->head;
			     i < index && over != NULL;
			     ++i, over = over->next)
				;
		}
	}

	if (over == NULL) return EINA_FALSE;

	it->current = over;
	it->index = index;

	if (data) *data = (void*) over;
	return EINA_TRUE;
}

static Eina_Inlist *
eina_inlist_accessor_get_container(Eina_Accessor_Inlist *it) {
	return (Eina_Inlist *) it->head;
}

static void
eina_inlist_accessor_free(Eina_Accessor_Inlist *it) {
	free(it);
}

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_inlist_append(void *in_list, void *in_item) {
	Eina_Inlist *l, *new_l;
	Eina_Inlist *list;

	list = in_list;
	new_l = in_item;
	new_l->next = NULL;
	if (!list) {
		new_l->prev = NULL;
		new_l->last = new_l;
		return new_l;
	}
	if (list->last)
		l = list->last;
	else
		for (l = list; (l) && (l->next); l = l->next)
			;
	l->next = new_l;
	new_l->prev = l;
	list->last = new_l;
	return list;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_inlist_prepend(void *in_list, void *in_item) {
	Eina_Inlist *new_l;
	Eina_Inlist *list;

	list = in_list;
	new_l = in_item;
	new_l->prev = NULL;
	if (!list) {
		new_l->next = NULL;
		new_l->last = new_l;
		return new_l;
	}
	new_l->next = list;
	list->prev = new_l;
	new_l->last = list->last;
	list->last = NULL;
	return new_l;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_inlist_append_relative(void *in_list, void *in_item,
		void *in_relative) {
	Eina_Inlist *list, *relative, *new_l;

	list = in_list;
	new_l = in_item;
	relative = in_relative;
	if (relative) {
		if (relative->next) {
			new_l->next = relative->next;
			relative->next->prev = new_l;
		} else
			new_l->next = NULL;
		relative->next = new_l;
		new_l->prev = relative;
		if (!new_l->next)
			list->last = new_l;
		return list;
	}
	return eina_inlist_append(list, new_l);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_inlist_prepend_relative(void *in_list, void *in_item,
		void *in_relative) {
	Eina_Inlist *list, *relative, *new_l;

	list = in_list;
	new_l = in_item;
	relative = in_relative;
	if (relative) {
		new_l->prev = relative->prev;
		new_l->next = relative;
		relative->prev = new_l;
		if (new_l->prev) {
			new_l->prev->next = new_l;
			/* new_l->next could not be NULL, as it was set to 'relative' */
			assert(new_l->next);
			return list;
		} else {
			/* new_l->next could not be NULL, as it was set to 'relative' */
			assert(new_l->next);

			new_l->last = list->last;
			list->last = NULL;
			return new_l;
		}
	}
	return eina_inlist_prepend(list, new_l);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_inlist_remove(void *in_list, void *in_item) {
	Eina_Inlist *return_l;
	Eina_Inlist *list, *item;

	/* checkme */
	if (!in_list)
		return in_list;

	list = in_list;
	item = in_item;
	if (!item)
		return list;
	if (item->next)
		item->next->prev = item->prev;
	if (item->prev) {
		item->prev->next = item->next;
		return_l = list;
	} else {
		return_l = item->next;
		if (return_l)
			return_l->last = list->last;
	}
	if (item == list->last)
		list->last = item->prev;
	item->next = NULL;
	item->prev = NULL;
	return return_l;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_inlist_find(void *in_list, void *in_item) {
	Eina_Inlist *l;
	Eina_Inlist *list, *item;

	list = in_list;
	item = in_item;
	for (l = list; l; l = l->next) {
		if (l == item)
			return item;
	}
	return NULL;
}

EAPI Eina_Iterator *eina_inlist_iterator_new(const void *in_list) {
	Eina_Iterator_Inlist *it;

	if (!in_list) return NULL;

	eina_error_set(0);
	it = calloc(1, sizeof (Eina_Iterator_Inlist));
	if (!it) {
		eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

	it->head = in_list;
	it->current = in_list;

	it->iterator.next = FUNC_ITERATOR_NEXT(eina_inlist_iterator_next);
	it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(eina_inlist_iterator_get_container);
	it->iterator.free = FUNC_ITERATOR_FREE(eina_inlist_iterator_free);

	return &it->iterator;
}

EAPI Eina_Accessor *eina_inlist_accessor_new(const void *in_list) {
	Eina_Accessor_Inlist *it;

	if (!in_list) return NULL;

	eina_error_set(0);
	it = calloc(1, sizeof (Eina_Accessor_Inlist));
	if (!it) {
		eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

	it->head = in_list;
	it->current = in_list;
	it->index = 0;

	it->accessor.get_at = FUNC_ACCESSOR_GET_AT(eina_inlist_accessor_get_at);
	it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(eina_inlist_accessor_get_container);
	it->accessor.free = FUNC_ACCESSOR_FREE(eina_inlist_accessor_free);

	return &it->accessor;
}
