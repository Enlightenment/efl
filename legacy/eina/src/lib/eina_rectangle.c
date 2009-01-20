/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Cedric BAIL, Carsten Haitzler
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

#include "eina_rectangle.h"
#include "eina_magic.h"
#include "eina_inlist.h"
#include "eina_private.h"
#include "eina_safety_checks.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_RECTANGLE_POOL_MAGIC 0x1578FCB0
#define EINA_RECTANGLE_ALLOC_MAGIC 0x1578FCB1

typedef struct _Eina_Rectangle_Alloc Eina_Rectangle_Alloc;

struct _Eina_Rectangle_Pool
{
   Eina_Inlist *head;
   void *data;

   unsigned int references;
   int w;
   int h;
   EINA_MAGIC
};

struct _Eina_Rectangle_Alloc
{
   EINA_INLIST;
   Eina_Rectangle_Pool *pool;
   EINA_MAGIC
};

#define EINA_MAGIC_CHECK_RECTANGLE_POOL(d)		       \
  do {							       \
     if (!EINA_MAGIC_CHECK((d), EINA_RECTANGLE_POOL_MAGIC))    \
       EINA_MAGIC_FAIL((d), EINA_RECTANGLE_POOL_MAGIC);	       \
  } while (0);

#define EINA_MAGIC_CHECK_RECTANGLE_ALLOC(d)		       \
  do {							       \
     if (!EINA_MAGIC_CHECK((d), EINA_RECTANGLE_ALLOC_MAGIC))   \
       EINA_MAGIC_FAIL((d), EINA_RECTANGLE_ALLOC_MAGIC);       \
  } while (0);

static inline Eina_Bool
_eina_rectangle_pool_collide(Eina_Rectangle_Alloc *head, Eina_Rectangle_Alloc *current, Eina_Rectangle *test)
{
   Eina_Rectangle_Alloc *collide;

   EINA_INLIST_FOREACH(head, collide)
     {
	Eina_Rectangle *colliding_rect = (Eina_Rectangle*) (collide + 1);

	if (collide == current) continue;
	if (eina_rectangles_intersect(colliding_rect, test))
	  return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
_eina_rectangle_pool_find(Eina_Rectangle_Alloc *head, int poolw, int poolh, int w, int h, int *x, int *y)
{
   Eina_Rectangle_Alloc *item;
   Eina_Rectangle tmp = { 0, 0, 0, 0 };

   if (head == NULL) goto on_intersect;

   EINA_INLIST_FOREACH(head, item)
     {
	Eina_Rectangle *rect = (Eina_Rectangle*) (item + 1);
	Eina_Bool t1 = EINA_TRUE;
	Eina_Bool t2 = EINA_TRUE;
	Eina_Bool t3 = EINA_TRUE;
	Eina_Bool t4 = EINA_TRUE;

	if ((rect->x + rect->w + w) > poolw) t1 = EINA_FALSE;
	if ((rect->y + h) > poolh) t1 = EINA_FALSE;
	if ((rect->y + rect->h + h) > poolh) t2 = EINA_FALSE;
	if ((rect->x + w) > poolw) t2 = EINA_FALSE;
	if ((rect->x - w) < 0) t3 = EINA_FALSE;
	if ((rect->y + h) > poolh) t3 = EINA_FALSE;
	if ((rect->x + w) > poolw) t4 = EINA_FALSE;
	if ((rect->y - h) < 0) t4 = EINA_FALSE;

	if (t1)
	  {
	     Eina_Bool intersects;
	     /* 1. try here:
	      * +----++--+
	      * |AAAA||??|
	      * |AAAA|+--+
	      * |AAAA|
	      * +----+
	      */
	     eina_rectangle_coords_from(&tmp, rect->x + rect->w, rect->y, w, h);
	     intersects = _eina_rectangle_pool_collide(head, item, &tmp);

	     if (!intersects) goto on_intersect;
	  }
	if (t2)
	  {
	     Eina_Bool intersects;
	     /* 2. try here:
	      * +----+
	      * |AAAA|
	      * |AAAA|
	      * |AAAA|
	      * +----+
	      * +--+
	      * |??|
	      * +--+
	      */
	     eina_rectangle_coords_from(&tmp, rect->x, rect->y + rect->h, w, h);
	     intersects = _eina_rectangle_pool_collide(head, item, &tmp);

	     if (!intersects) goto on_intersect;
	  }
	if (t3)
	  {
	     Eina_Bool intersects;
	     /* 3. try here:
	      * +--++----+
	      * |??||AAAA|
	      * +--+|AAAA|
	      *     |AAAA|
	      *     +----+
	      */
	     eina_rectangle_coords_from(&tmp, rect->x - w, rect->y, w, h);
	     intersects = _eina_rectangle_pool_collide(head, item, &tmp);

	     if (!intersects) goto on_intersect;
	  }
	if (t4)
	  {
	     Eina_Bool intersects;
	     /* 2. try here:
	      * +--+
	      * |??|
	      * +--+
	      * +----+
	      * |AAAA|
	      * |AAAA|
	      * |AAAA|
	      * +----+
	      */
	     eina_rectangle_coords_from(&tmp, rect->x, rect->y - h, w, h);
	     intersects = _eina_rectangle_pool_collide(head, item, &tmp);

	     if (!intersects) goto on_intersect;
	  }
     }

   return EINA_FALSE;

 on_intersect:
   *x = tmp.x;
   *y = tmp.y;
   return EINA_TRUE;
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eina_Rectangle_Pool *
eina_rectangle_pool_add(int w, int h)
{
   Eina_Rectangle_Pool *new;

   new = malloc(sizeof (Eina_Rectangle_Pool));
   if (!new) return NULL;

   new->head = NULL;
   new->references = 0;
   new->w = w;
   new->h = h;

   EINA_MAGIC_SET(new, EINA_RECTANGLE_POOL_MAGIC);

   return new;
}

EAPI void
eina_rectangle_pool_delete(Eina_Rectangle_Pool *pool)
{
   Eina_Rectangle_Alloc *del;

   EINA_SAFETY_ON_NULL_RETURN(pool);
   while (pool->head)
     {
	del = (Eina_Rectangle_Alloc*) pool->head;

	pool->head = (EINA_INLIST_GET(del))->next;

	MAGIC_FREE(del);
     }

   MAGIC_FREE(pool);
}

EAPI int
eina_rectangle_pool_count(Eina_Rectangle_Pool *pool)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pool, 0);
   return pool->references;
}

EAPI Eina_Rectangle *
eina_rectangle_pool_request(Eina_Rectangle_Pool *pool, int w, int h)
{
   Eina_Rectangle_Alloc *new;
   Eina_Rectangle *rect;
   Eina_Bool test;
   int x;
   int y;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pool, NULL);

   if (w > pool->w || h > pool->h) return NULL;

   test = _eina_rectangle_pool_find((Eina_Rectangle_Alloc*) pool->head, pool->w, pool->h, w, h, &x, &y);
   if (!test) return NULL;

   new = malloc(sizeof (Eina_Rectangle_Alloc) + sizeof (Eina_Rectangle));
   if (!new) return NULL;

   rect = (Eina_Rectangle*) (new + 1);
   eina_rectangle_coords_from(rect, x, y, w, h);

   pool->head = eina_inlist_prepend(pool->head, EINA_INLIST_GET(new));
   pool->references++;

   new->pool = pool;

   EINA_MAGIC_SET(new, EINA_RECTANGLE_ALLOC_MAGIC);

   return rect;
}

EAPI void
eina_rectangle_pool_release(Eina_Rectangle *rect)
{
   Eina_Rectangle_Alloc *era = ((Eina_Rectangle_Alloc *) rect) - 1;

   EINA_SAFETY_ON_NULL_RETURN(rect);

   EINA_MAGIC_CHECK_RECTANGLE_ALLOC(era);
   EINA_MAGIC_CHECK_RECTANGLE_POOL(era->pool);

   era->pool->references--;
   era->pool->head = eina_inlist_remove(era->pool->head, EINA_INLIST_GET(era));

   MAGIC_FREE(era);
}

EAPI Eina_Rectangle_Pool *
eina_rectangle_pool_get(Eina_Rectangle *rect)
{
   Eina_Rectangle_Alloc *era = ((Eina_Rectangle_Alloc *) rect) - 1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rect, NULL);

   EINA_MAGIC_CHECK_RECTANGLE_ALLOC(era);
   EINA_MAGIC_CHECK_RECTANGLE_POOL(era->pool);

   return era->pool;
}

EAPI void
eina_rectangle_pool_data_set(Eina_Rectangle_Pool *pool, const void *data)
{
   EINA_MAGIC_CHECK_RECTANGLE_POOL(pool);
   EINA_SAFETY_ON_NULL_RETURN(pool);

   pool->data = (void*) data;
}

EAPI void *
eina_rectangle_pool_data_get(Eina_Rectangle_Pool *pool)
{
   EINA_MAGIC_CHECK_RECTANGLE_POOL(pool);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pool, NULL);

   return pool->data;
}

EAPI Eina_Bool
eina_rectangle_pool_geometry_get(Eina_Rectangle_Pool *pool, int *w, int *h)
{
   if (!pool) return EINA_FALSE;

   EINA_MAGIC_CHECK_RECTANGLE_POOL(pool);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pool, EINA_FALSE);

   if (w) *w = pool->w;
   if (h) *h = pool->h;

   return EINA_TRUE;
}


