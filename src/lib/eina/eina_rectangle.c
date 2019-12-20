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
#include "eina_private.h"
#include "eina_magic.h"
#include "eina_inlist.h"
#include "eina_mempool.h"
#include "eina_list.h"
#include "eina_trash.h"
#include "eina_log.h"
#include "eina_lock.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_rectangle.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_RECTANGLE_POOL_MAGIC 0x1578FCB0
#define EINA_RECTANGLE_ALLOC_MAGIC 0x1578FCB1

#define BUCKET_THRESHOLD 110

typedef struct _Eina_Rectangle_Alloc Eina_Rectangle_Alloc;

struct _Eina_Rectangle_Pool
{
   Eina_Inlist *head;
   Eina_List *empty;
   void *data;

   Eina_Compare_Cb eina_rectangle_compare_func;

   Eina_Trash *bucket;
   unsigned int bucket_count;

   Eina_Rectangle_Packing type;

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

#define EINA_MAGIC_CHECK_RECTANGLE_POOL(d)                     \
   do {                                                         \
        if (!EINA_MAGIC_CHECK((d), EINA_RECTANGLE_POOL_MAGIC)) {    \
             EINA_MAGIC_FAIL((d), EINA_RECTANGLE_POOL_MAGIC); }        \
     } while (0)

#define EINA_MAGIC_CHECK_RECTANGLE_ALLOC(d)                    \
   do {                                                         \
        if (!EINA_MAGIC_CHECK((d), EINA_RECTANGLE_ALLOC_MAGIC)) {   \
             EINA_MAGIC_FAIL((d), EINA_RECTANGLE_ALLOC_MAGIC); }       \
     } while (0)

static Eina_Mempool *_eina_rectangle_alloc_mp = NULL;
static Eina_Mempool *_eina_rectangle_mp = NULL;

static Eina_Spinlock _eina_spinlock;
static Eina_Trash *_eina_rectangles = NULL;
static unsigned int _eina_rectangles_count = 0;
static int _eina_rectangle_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_rectangle_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_rectangle_log_dom, __VA_ARGS__)

static int
_eina_rectangle_cmp(const void *data1, const void *data2)
{
   Eina_Rectangle *r1 = (Eina_Rectangle *) data1;
   Eina_Rectangle *r2 = (Eina_Rectangle *) data2;
   return (r2->w * r2->h) - (r1->w * r1->h);
}

static int
_eina_rectangle_cmp_asc(const void *data1, const void *data2)
{
   Eina_Rectangle *r1 = (Eina_Rectangle *) data1;
   Eina_Rectangle *r2 = (Eina_Rectangle *) data2;
   return (r1->w * r1->h) - (r2->w * r2->h);
}

static int
_eina_rectangle_cmp_bl(const void *data1, const void *data2)
{
   Eina_Rectangle *r1 = (Eina_Rectangle *) data1;
   Eina_Rectangle *r2 = (Eina_Rectangle *) data2;
   if (r1->y != r2->y)
     return (r1->y) - (r2->y);
   else
     return (r1->x) - (r2->x);
}

static Eina_List *
_eina_rectangle_merge_list(Eina_List *empty, Eina_Rectangle_Packing type, Eina_Rectangle *r)
{
   Eina_Rectangle *match, *r1;
   Eina_List *l;
   int xw;
   int yh;
   int x2 ,y2 ,w2 ,h2;

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
      else if (match->y > r->y && type == Eina_Packing_Bottom_Left_Skyline
               && (match->y + match->h == r->y + r->h) &&
              (match->x + match->w == r->x || r->x + r->w == match->x))
        {

           if (r->x < match->x)
              match->x = r->x;

           match->w += r->w;

           x2 = r->x;
           y2 = r->y;
           w2 = r->w;
           h2 = match->y - r->y;

           eina_rectangle_free(r);

           r1 = eina_rectangle_new(x2, y2, w2, h2);

           empty = eina_list_remove_list(empty, l);

           if (r1)
              empty = eina_list_append(empty, r1);

           r = match;

           goto start_again;
        }
   }

   return eina_list_append(empty, r);
}

static Eina_List *
_eina_rectangle_empty_space_find(Eina_List *empty, Eina_Rectangle_Packing type, int w, int h, int *x, int *y)
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
                int rx1, ry1, rw1, rh1;
                int x2, y2, w2, h2;

                rx1 = r->x + w;
                ry1 = r->y;
                rw1 = r->w - w;
                /* h1 could be h or r->h */
                x2 = r->x;
                y2 = r->y + h;
                /* w2 could be w or r->w */
                h2 = r->h - h;

                if ((rw1 * r->h > h2 * r->w) || type == Eina_Packing_Bottom_Left || type == Eina_Packing_Bottom_Left_Skyline)
                  {
                     rh1 = r->h;
                     w2 = w;
                  }
                else
                  {
                     rh1 = h;
                     w2 = r->w;
                  }
                if (type == Eina_Packing_Bottom_Left_Skyline_Improved)
                  {
                     rh1 = r->h;
                     w2 = r->w;
                  }

                EINA_RECTANGLE_SET(r, rx1, ry1, rw1, rh1);
                empty = _eina_rectangle_merge_list(empty, type, r);

                r = eina_rectangle_new(x2, y2, w2, h2);
             }

           if (r)
             {
                empty = _eina_rectangle_merge_list(empty, type, r); /* Return empty */

             }

           return empty;
        }
   }

   *x = -1;
   *y = -1;
   return empty;
}

static Eina_List *
_eina_rectangle_skyline_merge_list(Eina_List *empty, Eina_Rectangle *r)
{
   Eina_Rectangle *match;
   Eina_List *l;

   EINA_LIST_FOREACH(empty, l, match)
     {
        if (match->x == r->x + r->w)
          {
             match->x = r->x;
             match->w = r->w + match->w;
          }
        else if (match->y == r->y + r->h)
          {
             match->y = r->y;
             match->h = r->h + match->h;
          }
        else if (match->x + match->w == r->x)
          {
            match->w = r->w + match->w;
          }
        else if (match->y + match->h == r->y )
          {
            match->h = r->h + match->h;
          }
     }
   return empty;
}

static Eina_List *
_eina_rectangle_skyline_list_update(Eina_List *empty, Eina_Rectangle *rect)
{
   Eina_Rectangle *r, *r1;
   Eina_List *l;
   int x2, y2, w2, h2;

start_again :
   EINA_LIST_FOREACH(empty, l, r)
     {
        if (eina_rectangles_intersect(rect, r))
          {
             /* Remove l from empty */
             empty = eina_list_remove_list(empty, l);

             if (r->y > rect->y)
               {
                  if (r->y + r->h > rect->y + rect->h)
                    {
                       w2 = r->w;
                       h2 = (r->y +r->h) - (rect->y + rect->h);
                       x2 = r->x;
                       y2 = (r->y +r->h) - h2;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
                  if ( r->x + r->w > rect->x + rect->w )
                    {
                       w2 = (r->x +r->w) - (rect->x + rect->w);
                       h2 = r->h;
                       x2 = rect->x + rect->w;
                       y2 = r->y;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
                  if ( rect->x - r->x)
                    {
                       w2 = rect->x - r->x;
                       h2 = r->h;
                       x2 = r->x;
                       y2 = r->y;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
               }
             else if (r->x > rect->x)
               {
                  if (r->x + r->w > rect->x + rect->w)
                    {
                       w2 = (r->x + r->w) - (rect->x + rect->w);
                       h2 = r->h;
                       x2 = (r->x +r->w) - w2;
                       y2 = r->y;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
                  if ( r->y + r->h > rect->y + rect->y )
                    {
                       w2 = r->w;
                       h2 = (r->y +r->h) - (rect->y + rect->h);
                       x2 = r->x;
                       y2 = rect->y + rect->h;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
                  if ( rect->y > r->y)
                    {
                       w2 = r->w;;
                       h2 = rect->y - r->y;
                       x2 = r->x;
                       y2 = r->y;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
               }
             else if (r->x == rect->x && r->y < rect->y)
               {
                  if (rect->y + rect->h < r->y + r->h)
                    {
                       w2 = r->w;
                       h2 = (r->y +r->h) - (rect->y + rect->h);
                       x2 = r->x;
                       y2 = rect->y + rect->h;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
                  w2 = r->w;
                  h2 = rect->y - r->y;
                  x2 = r->x;
                  y2 = r->y;
                  r1 = eina_rectangle_new(x2, y2, w2, h2);
                  empty = eina_list_prepend(empty, r1);
               }
             else if (r->y == rect->y && r->x < rect->x)
               {
                  if (rect->w + rect->x < r->w + r->x)
                    {
                       w2 = (r->x + r->w) - (rect->x + rect->w);
                       h2 = r->h;
                       x2 = rect->x + rect->w;
                       y2 = r->y;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
                  w2 = rect->x - r->x;
                  h2 = r->h;;
                  x2 = r->x;
                  y2 = r->y;
                  r1 = eina_rectangle_new(x2, y2, w2, h2);
                  empty = eina_list_prepend(empty, r1);
               }
             else if (r->x < rect->x && r->y < rect->y)
               {
                  w2 = rect->x - r->x;
                  h2 = r->h;
                  x2 = r->x;
                  y2 = r->y;
                  r1 = eina_rectangle_new(x2, y2, w2, h2);
                  empty = eina_list_prepend(empty, r1);
                  w2 = r->w;
                  h2 = rect->y - r->y;
                  x2 = r->x;
                  y2 = r->y;
                  r1 = eina_rectangle_new(x2, y2, w2, h2);
                  empty = eina_list_prepend(empty, r1);
               }
             else if (r->x == rect->x && r->y == rect->y)
               {
                  if (r->w > rect->w )
                    {
                       w2 = r->w - rect->w;
                       h2 = r->h;
                       x2 = rect->x + rect->w;
                       y2 = r->y;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
                  if (r->h > rect->h )
                    {
                       w2 = r->w;
                       h2 = r->h - rect->h;
                       x2 = r->x;
                       y2 = rect->y + rect->h;
                       r1 = eina_rectangle_new(x2, y2, w2, h2);
                       empty = eina_list_prepend(empty, r1);
                    }
               }
             goto start_again;
          }
   }
   return empty;
}

static Eina_List *
_eina_rectangle_skyline_list_update_duplicate(Eina_List *empty)
{
   Eina_Rectangle *r, *r1;
   Eina_List *l, *l1, *l2;

start_again:
   EINA_LIST_FOREACH(empty, l, r)
     {
        l1 = eina_list_next(l);
        EINA_LIST_FOREACH(l1, l2, r1)
          {
             if ((r->x <= r1->x) && (r->y <= r1->y) && (r->x + r->w >= r1->x + r1->w) && (r->y + r->h >= r1->y + r1->h))
               {
                  empty = eina_list_remove_list(empty, l2);
                  goto start_again;
               }
             else if ((r->x >= r1->x) && (r->y >= r1->y) && (r->x + r->w <= r1->x + r1->w) && (r->y + r->h <= r1->y + r1->h))
               {
                  empty = eina_list_remove_list(empty, l);
                  goto start_again;
               }
          }
     }
   return empty;
}

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

Eina_Bool
eina_rectangle_init(void)
{
   const char *choice, *tmp;

   _eina_rectangle_log_dom = eina_log_domain_register("eina_rectangle",
                                                      EINA_LOG_COLOR_DEFAULT);
   if (_eina_rectangle_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_rectangle");
        return EINA_FALSE;
     }

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   choice = "chained_mempool";
#endif
   tmp = getenv("EINA_MEMPOOL");
   if (tmp && tmp[0])
      choice = tmp;

   _eina_rectangle_alloc_mp = eina_mempool_add
         (choice, "rectangle-alloc", NULL,
         sizeof(Eina_Rectangle_Alloc) + sizeof(Eina_Rectangle), 64);
   if (!_eina_rectangle_alloc_mp)
     {
        ERR("Mempool for rectangle cannot be allocated in rectangle init.");
        goto init_error;
     }

   _eina_rectangle_mp = eina_mempool_add
         (choice, "rectangle", NULL, sizeof(Eina_Rectangle), 32);
   if (!_eina_rectangle_mp)
     {
        ERR("Mempool for rectangle cannot be allocated in rectangle init.");
        goto init_error;
     }

   eina_spinlock_new(&_eina_spinlock);

   return EINA_TRUE;

init_error:
   eina_log_domain_unregister(_eina_rectangle_log_dom);
   _eina_rectangle_log_dom = -1;

   return EINA_FALSE;
}

Eina_Bool
eina_rectangle_shutdown(void)
{
   Eina_Rectangle *del;

   eina_spinlock_free(&_eina_spinlock);
   while ((del = eina_trash_pop(&_eina_rectangles)))
      eina_mempool_free(_eina_rectangle_mp, del);
   _eina_rectangles_count = 0;

   eina_mempool_del(_eina_rectangle_alloc_mp);
   eina_mempool_del(_eina_rectangle_mp);

   eina_log_domain_unregister(_eina_rectangle_log_dom);
   _eina_rectangle_log_dom = -1;

   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI Eina_Rectangle *
eina_rectangle_new(int x, int y, int w, int h)
{
   Eina_Rectangle *rect = NULL;

   if (_eina_rectangles)
     {
        eina_spinlock_take(&_eina_spinlock);
        rect = eina_trash_pop(&_eina_rectangles);
        _eina_rectangles_count--;
        eina_spinlock_release(&_eina_spinlock);
     }

   if (!rect)
     rect = eina_mempool_malloc(_eina_rectangle_mp, sizeof (Eina_Rectangle));

   if (!rect)
     return NULL;

   EINA_RECTANGLE_SET(rect, x, y, w, h);

   return rect;
}

EAPI void
eina_rectangle_free(Eina_Rectangle *rect)
{
   EINA_SAFETY_ON_NULL_RETURN(rect);

   if (_eina_rectangles_count > BUCKET_THRESHOLD)
      eina_mempool_free(_eina_rectangle_mp, rect);
   else
     {
        eina_spinlock_take(&_eina_spinlock);
        eina_trash_push(&_eina_rectangles, rect);
        _eina_rectangles_count++;
        eina_spinlock_release(&_eina_spinlock);
     }
}

EAPI Eina_Rectangle_Pool *
eina_rectangle_pool_new(int w, int h)
{
   Eina_Rectangle_Pool *new;

   if ((w <= 0) || (h <= 0))
      return NULL;

   new = malloc(sizeof (Eina_Rectangle_Pool));
   if (!new)
      return NULL;

   new->head = NULL;
   new->empty = eina_list_append(NULL, eina_rectangle_new(0, 0, w, h));
   new->references = 0;
   new->sorted = EINA_FALSE;
   new->w = w;
   new->h = h;
   new->bucket = NULL;
   new->bucket_count = 0;
   new->eina_rectangle_compare_func = _eina_rectangle_cmp;
   new->type = Eina_Packing_Ascending;

   EINA_MAGIC_SET(new, EINA_RECTANGLE_POOL_MAGIC);
   DBG("pool=%p, size=(%d, %d)", new, w, h);

   return new;
}

EAPI void
eina_rectangle_pool_free(Eina_Rectangle_Pool *pool)
{
   Eina_Rectangle_Alloc *del;
   Eina_Rectangle *r;

   EINA_SAFETY_ON_NULL_RETURN(pool);
   DBG("pool=%p, size=(%d, %d), references=%u",
       pool, pool->w, pool->h, pool->references);
   EINA_LIST_FREE(pool->empty, r)
     eina_rectangle_free(r);
   while (pool->head)
     {
        del = (Eina_Rectangle_Alloc *)pool->head;

        pool->head = (EINA_INLIST_GET(del))->next;

        EINA_MAGIC_SET(del, EINA_MAGIC_NONE);
        eina_mempool_free(_eina_rectangle_alloc_mp, del);
     }

   while (pool->bucket)
     {
        del = eina_trash_pop(&pool->bucket);
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

   DBG("pool=%p, size=(%d, %d), references=%u",
       pool, pool->w, pool->h, pool->references);

   if (w <= 0 || h <= 0)
      return NULL;

   if (w > pool->w || h > pool->h)
      return NULL;

   /* Sort empty if dirty */
   if (!pool->sorted)
     {
        pool->empty =
           eina_list_sort(pool->empty, 0, pool->eina_rectangle_compare_func);
        pool->sorted = EINA_TRUE;
     }

   if (pool->type ==   Eina_Packing_Bottom_Left_Skyline_Improved)
     pool->empty = _eina_rectangle_skyline_list_update_duplicate(pool->empty);
   pool->empty = _eina_rectangle_empty_space_find(pool->empty, pool->type, w, h, &x, &y);
   if (x == -1)
      return NULL;

   pool->sorted = EINA_FALSE;

   if (pool->bucket_count > 0)
     {
        new = eina_trash_pop(&pool->bucket);
        pool->bucket_count--;
     }
   else
      new = eina_mempool_malloc(_eina_rectangle_alloc_mp,
                                sizeof (Eina_Rectangle_Alloc) +
                                sizeof (Eina_Rectangle));

   if (!new)
      return NULL;

   rect = (Eina_Rectangle *)(new + 1);
   eina_rectangle_coords_from(rect, x, y, w, h);

   if (pool->type == Eina_Packing_Bottom_Left_Skyline_Improved)
     pool->empty = _eina_rectangle_skyline_list_update(pool->empty, rect);

   pool->head = eina_inlist_prepend(pool->head, EINA_INLIST_GET(new));
   pool->references++;

   new->pool = pool;

   EINA_MAGIC_SET(new, EINA_RECTANGLE_ALLOC_MAGIC);
   DBG("rect=%p pool=%p, size=(%d, %d), references=%u",
       rect, pool, pool->w, pool->h, pool->references);

   return rect;
}

EAPI void
eina_rectangle_pool_release(Eina_Rectangle *rect)
{
   Eina_Rectangle_Alloc *era = ((Eina_Rectangle_Alloc *)rect) - 1;
   Eina_Rectangle_Alloc *new;
   Eina_Rectangle *r;
   Eina_Rectangle *match;

   EINA_SAFETY_ON_NULL_RETURN(rect);

   EINA_MAGIC_CHECK_RECTANGLE_ALLOC(era);
   EINA_MAGIC_CHECK_RECTANGLE_POOL(era->pool);

   DBG("rect=%p pool=%p, size=(%d, %d), references=%u",
       rect, era->pool, era->pool->w, era->pool->h, era->pool->references);

   era->pool->references--;
   era->pool->head = eina_inlist_remove(era->pool->head, EINA_INLIST_GET(era));

   r = eina_rectangle_new(rect->x, rect->y, rect->w, rect->h);
   if (r)
     {
        if (era->pool->type == Eina_Packing_Bottom_Left_Skyline_Improved)
          {
             era->pool->empty = _eina_rectangle_skyline_merge_list(era->pool->empty, r);
             era->pool->empty = _eina_rectangle_skyline_list_update_duplicate(era->pool->empty);
             EINA_INLIST_FOREACH(era->pool->head, new)
             {
                match =(Eina_Rectangle *) (new + 1);
                if (match)
                  era->pool->empty = _eina_rectangle_skyline_list_update(era->pool->empty, match);
             }
          }
        else
          era->pool->empty = _eina_rectangle_merge_list(era->pool->empty, era->pool->type, r);
       era->pool->sorted = EINA_FALSE;
     }

   if (era->pool->bucket_count < BUCKET_THRESHOLD)
     {
        Eina_Rectangle_Pool *pool;

        pool = era->pool;

        pool->bucket_count++;
        eina_trash_push(&pool->bucket, era);
     }
   else
     {
        EINA_MAGIC_SET(era, EINA_MAGIC_NONE);
        eina_mempool_free(_eina_rectangle_alloc_mp, era);
     }
}

EAPI Eina_Rectangle_Pool *
eina_rectangle_pool_get(Eina_Rectangle *rect)
{
   Eina_Rectangle_Alloc *era = ((Eina_Rectangle_Alloc *)rect) - 1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rect, NULL);

   EINA_MAGIC_CHECK_RECTANGLE_ALLOC(era);
   EINA_MAGIC_CHECK_RECTANGLE_POOL(era->pool);

   return era->pool;
}

EAPI void
eina_rectangle_pool_packing_set(Eina_Rectangle_Pool *pool, Eina_Rectangle_Packing type)
{
   EINA_MAGIC_CHECK_RECTANGLE_POOL(pool);
   EINA_SAFETY_ON_NULL_RETURN(pool);

   DBG("type=%d  pool=%p, size=(%d, %d), references=%u",
       type, pool, pool->w, pool->h, pool->references);
   pool->type =type;

   switch (type)
     {
      case Eina_Packing_Ascending:
         pool->eina_rectangle_compare_func = _eina_rectangle_cmp_asc;
         break;
      case Eina_Packing_Descending:
         pool->eina_rectangle_compare_func = _eina_rectangle_cmp;
         break;
      default:
         pool->eina_rectangle_compare_func = _eina_rectangle_cmp_bl;
     }
}

EAPI void
eina_rectangle_pool_data_set(Eina_Rectangle_Pool *pool, const void *data)
{
   EINA_MAGIC_CHECK_RECTANGLE_POOL(pool);
   EINA_SAFETY_ON_NULL_RETURN(pool);

   DBG("data=%p pool=%p, size=(%d, %d), references=%u",
       data, pool, pool->w, pool->h, pool->references);

   pool->data = (void *)data;
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
   if (!pool)
      return EINA_FALSE;

   EINA_MAGIC_CHECK_RECTANGLE_POOL(pool);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pool, EINA_FALSE);

   if (w)
      *w = pool->w;

   if (h)
      *h = pool->h;

   return EINA_TRUE;
}

EAPI Eina_Rectangle_Outside
eina_rectangle_outside_position(Eina_Rectangle *rect1, Eina_Rectangle *rect2)
{
   Eina_Rectangle_Outside ret = 0;

   if (rect1->y > rect2->y)
     ret |= EINA_RECTANGLE_OUTSIDE_TOP;
   if (rect1->x > rect2->x)
     ret |= EINA_RECTANGLE_OUTSIDE_LEFT;
   if (rect1->y + rect1->h < rect2->y + rect2->h)
     ret |= EINA_RECTANGLE_OUTSIDE_BOTTOM;
   if (rect1->x + rect1->w < rect2->x + rect2->w)
     ret |= EINA_RECTANGLE_OUTSIDE_RIGHT;

   return ret;
}
