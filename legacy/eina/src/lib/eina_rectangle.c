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

#include <stdio.h>
#include <stdlib.h>

#include "eina_config.h"
#include "eina_rectangle.h"
#include "eina_magic.h"
#include "eina_inlist.h"
#include "eina_private.h"
#include "eina_safety_checks.h"
#include "eina_mempool.h"
#include "eina_list.h"

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
   Eina_List *empty;
   void *data;

   unsigned int references;
   int w;
   int h;

   Eina_Bool sorted;
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

static int _eina_rectangle_init_count = 0;
static Eina_Mempool *_eina_rectangle_alloc_mp = NULL;
static Eina_Mempool *_eina_rectangle_mp = NULL;

static int
_eina_rectangle_cmp(const Eina_Rectangle *r1, const Eina_Rectangle *r2)
{
   return (r2->w * r2->h) - (r1->w * r1->h);
}

static Eina_List *
_eina_rectangle_merge_list(Eina_List *empty, Eina_Rectangle *r)
{
   Eina_Rectangle *match;
   Eina_List *l;
   int xw;
   int yh;

   if (r->w == 0 || r->h == 0)
     {
	eina_rectangle_free(r);
	return empty;
     }

 start_again:
   xw = r->x + r->w;
   yh = r->y + r->h;

   EINA_LIST_FOREACH(empty, l, match)
     {
	if (match->x == r->x && match->w == r->w
	    && (match->y == yh || r->y == match->y + match->h))
	  {
	     if (match->y > r->y)
	       match->y = r->y;
	     match->h += r->h;

	     eina_rectangle_free(r);

	     empty = eina_list_remove_list(empty, l);

	     r = match;

	     goto start_again;
	  }
	else if (match->y == r->y && match->h == r->h
		 && (match->x == xw || r->x == match->x + match->w))
	  {
	     if (match->x > r->x)
	       match->x = r->x;
	     match->w += r->w;

	     eina_rectangle_free(r);

	     empty = eina_list_remove_list(empty, l);

	     r = match;

	     goto start_again;
	  }
     }

   return eina_list_append(empty, r);
}

static Eina_List *
_eina_rectangle_empty_space_find(Eina_List *empty, int w, int h, int *x, int *y)
{
   Eina_Rectangle *r;
   Eina_List *l;

   EINA_LIST_FOREACH(empty, l, r)
     {
	if (r->w >= w && r->h >= h)
	  {
	     /* Remove l from empty */
	     empty = eina_list_remove_list(empty, l);
	     /* Remember x and y */
	     *x = r->x;
	     *y = r->y;
	     /* Split r in 2 rectangle if needed (only the empty one) and insert them */
	     if (r->w == w)
	       {
		  r->y += h;
		  r->h -= h;
	       }
	     else if (r->h == h)
	       {
		  r->x += w;
		  r->w -= w;
	       }
	     else
	       {
		  int x1, y1, w1, h1;
		  int x2, y2, w2, h2;

		  x1 = r->x + w;
		  y1 = r->y;
		  w1 = r->w - w;
		  /* h1 could be h or r->h */
		  x2 = r->x;
		  y2 = r->y + h;
		  /* w2 could be w or r->w */
		  h2 = r->h - h;

		  if (w1 * r->h > h2 * r->w)
		    {
		       h1 = r->h;
		       w2 = w;
		    }
		  else
		    {
		       h1 = h;
		       w2 = r->w;
		    }

		  EINA_RECTANGLE_SET(r, x1, y1, w1, h1);
		  empty = _eina_rectangle_merge_list(empty, r);

		  r = eina_rectangle_new(x2, y2, w2, h2);
	       }
	     if (r) empty = _eina_rectangle_merge_list(empty, r);
	     /* Return empty */
	     return empty;
	  }
     }

   *x = -1;
   *y = -1;
   return empty;
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

EAPI int
eina_rectangle_init(void)
{
   const char *choice;

   _eina_rectangle_init_count++;

   if (_eina_rectangle_init_count > 1) return _eina_rectangle_init_count;

   if (!eina_error_init())
     {
        EINA_ERROR_PERR("Could not initialize eina error module.\n");
        return 0;
     }
   if (!eina_mempool_init())
     {
        EINA_ERROR_PERR("Could not initialize eina mempool module.\n");
        goto mempool_init_error;
     }

   if (!eina_list_init())
     {
	EINA_ERROR_PERR("Could not initialize eina list module.\n");
	goto list_init_error;
     }

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   if (!(choice = getenv("EINA_MEMPOOL")))
     choice = "chained_mempool";
#endif

   _eina_rectangle_alloc_mp = eina_mempool_add(choice, "rectangle-alloc", NULL,
                                         sizeof (Eina_Rectangle_Alloc) + sizeof (Eina_Rectangle), 1024);
   if (!_eina_rectangle_alloc_mp)
     {
        EINA_ERROR_PERR("ERROR: Mempool for rectangle cannot be allocated in list init.\n");
        goto init_error;
     }

   _eina_rectangle_mp = eina_mempool_add(choice, "rectangle", NULL, sizeof (Eina_Rectangle), 256);
   if (!_eina_rectangle_mp)
     {
        EINA_ERROR_PERR("ERROR: Mempool for rectangle cannot be allocated in list init.\n");
        goto init_error;
     }

   return _eina_rectangle_init_count;

 init_error:
   eina_list_shutdown();
 list_init_error:
   eina_mempool_shutdown();
 mempool_init_error:
   eina_error_shutdown();

   return 0;
}

EAPI int
eina_rectangle_shutdown(void)
{
   --_eina_rectangle_init_count;

   if (_eina_rectangle_init_count) return _eina_rectangle_init_count;

   eina_list_shutdown();

   eina_mempool_del(_eina_rectangle_alloc_mp);
   eina_mempool_del(_eina_rectangle_mp);

   eina_mempool_shutdown();
   eina_error_shutdown();

   return 0;
}

EAPI Eina_Rectangle *
eina_rectangle_new(int x, int y, int w, int h)
{
   Eina_Rectangle *rect;

   rect = eina_mempool_malloc(_eina_rectangle_mp, sizeof (Eina_Rectangle));
   if (!rect) return NULL;

   EINA_RECTANGLE_SET(rect, x, y, w, h);

   return rect;
}

EAPI void
eina_rectangle_free(Eina_Rectangle *rect)
{
   EINA_SAFETY_ON_NULL_RETURN(rect);
   eina_mempool_free(_eina_rectangle_mp, rect);
}

EAPI Eina_Rectangle_Pool *
eina_rectangle_pool_new(int w, int h)
{
   Eina_Rectangle_Pool *new;

   new = malloc(sizeof (Eina_Rectangle_Pool));
   if (!new) return NULL;

   new->head = NULL;
   new->empty = eina_list_append(NULL, eina_rectangle_new(0, 0, w, h));
   new->references = 0;
   new->sorted = EINA_FALSE;
   new->w = w;
   new->h = h;

   EINA_MAGIC_SET(new, EINA_RECTANGLE_POOL_MAGIC);

   return new;
}

EAPI void
eina_rectangle_pool_free(Eina_Rectangle_Pool *pool)
{
   Eina_Rectangle_Alloc *del;

   EINA_SAFETY_ON_NULL_RETURN(pool);
   while (pool->head)
     {
	del = (Eina_Rectangle_Alloc*) pool->head;

	pool->head = (EINA_INLIST_GET(del))->next;

	EINA_MAGIC_SET(del, EINA_MAGIC_NONE);
	eina_mempool_free(_eina_rectangle_alloc_mp, del);
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
   int x;
   int y;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pool, NULL);

   if (w <= 0 || h <= 0) return NULL;
   if (w > pool->w || h > pool->h) return NULL;

   /* Sort empty if dirty */
   if (pool->sorted)
     {
	pool->empty = eina_list_sort(pool->empty, 0, EINA_COMPARE_CB(_eina_rectangle_cmp));
	pool->sorted = EINA_TRUE;
     }

   pool->empty = _eina_rectangle_empty_space_find(pool->empty, w, h, &x, &y);
   if (x == -1) return NULL;
   pool->sorted = EINA_FALSE;

   new = eina_mempool_malloc(_eina_rectangle_alloc_mp,
			     sizeof (Eina_Rectangle_Alloc) + sizeof (Eina_Rectangle));
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
   Eina_Rectangle *r;

   EINA_SAFETY_ON_NULL_RETURN(rect);

   EINA_MAGIC_CHECK_RECTANGLE_ALLOC(era);
   EINA_MAGIC_CHECK_RECTANGLE_POOL(era->pool);

   era->pool->references--;
   era->pool->head = eina_inlist_remove(era->pool->head, EINA_INLIST_GET(era));

   r = eina_rectangle_new(rect->x, rect->y, rect->w, rect->h);
   if (r)
     {
	era->pool->empty = _eina_rectangle_merge_list(era->pool->empty, r);
	era->pool->sorted = EINA_FALSE;
     }

   EINA_MAGIC_SET(era, EINA_MAGIC_NONE);
   eina_mempool_free(_eina_rectangle_alloc_mp, era);
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

