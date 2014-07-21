/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Gustavo Sverzut Barbieri, Jorge Luis Zapata Muga
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

/* TODO
 * it is possible to have more than one tiler algorithm, but for now the
 * version Gustavo did is hardcoded here
 * http://blog.gustavobarbieri.com.br/2007/06/03/evas-now-using-rectangle-split-and-merge/
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_tiler.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/* The splitter data types */
typedef struct list_node list_node_t;
typedef struct list list_t;
typedef struct rect rect_t;
typedef struct rect_node rect_node_t;

struct list_node
{
   struct list_node *next;
};

struct list
{
   struct list_node *head;
   struct list_node *tail;
};

struct rect
{
   short right;
   short bottom;
   short left;
   short top;
   short width;
   short height;
   int area;
};

struct rect_node
{
   struct list_node _lst;
   struct rect rect;
};

typedef struct splitter
{
   Eina_Bool need_merge;
   list_t rects;
} splitter_t;

typedef struct list_node_pool
{
   list_node_t *node;
   int len;
   int max;
} list_node_pool_t;


static const list_node_t list_node_zeroed = { NULL };
static const list_t list_zeroed = { NULL, NULL };
static list_node_pool_t list_node_pool = { NULL, 0, 1024 };


typedef struct _Eina_Iterator_Tiler
{
   Eina_Iterator iterator;
   const Eina_Tiler *tiler;
   list_node_t *curr;
   Eina_Rectangle r;
   EINA_MAGIC
} Eina_Iterator_Tiler;

struct _Eina_Tiler
{
   struct
   {
      int w, h;
   } tile;

   struct
   {
      Eina_Rectangle add, del;
   } last;
   Eina_Rectangle area;
   EINA_MAGIC
   splitter_t splitter;

   Eina_Bool rounding : 1;
   Eina_Bool strict : 1;
};

#define EINA_MAGIC_CHECK_TILER(d, ...)                                  \
  do {                                                                  \
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_TILER))                        \
       {                                                                \
          EINA_MAGIC_FAIL(d, EINA_MAGIC_TILER);                         \
          return __VA_ARGS__;                                           \
       }                                                                \
  } while(0)


#define EINA_MAGIC_CHECK_TILER_ITERATOR(d, ...)                    \
  do {                                                             \
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_TILER_ITERATOR))          \
       {                                                           \
          EINA_MAGIC_FAIL(d, EINA_MAGIC_TILER_ITERATOR);           \
          return __VA_ARGS__;                                      \
       }                                                           \
  } while(0)

/* The Splitter algorithm */
static inline void rect_init(rect_t *r, int x, int y, int w, int h)
{
   r->area = w * h;

   r->left = x;
   r->top = y;

   r->right = x + w;
   r->bottom = y + h;

   r->width = w;
   r->height = h;
}

static inline list_node_t *
rect_list_node_pool_get(void)
{
   if (list_node_pool.node)
     {
        list_node_t *node;

        node = list_node_pool.node;
        list_node_pool.node = node->next;
        list_node_pool.len--;

        return node;
     }
   else
      return malloc(sizeof(rect_node_t));
}


static inline void rect_list_concat(list_t *rects, list_t *other)
{
   if (!other->head)
      return;

   if (rects->tail)
     {
        rects->tail->next = other->head;
        rects->tail = other->tail;
     }
   else
     {
        rects->head = other->head;
        rects->tail = other->tail;
     }

   *other = list_zeroed;
}

static inline void rect_list_append_node(list_t *rects, list_node_t *node)
{
   if (rects->tail)
     {
        rects->tail->next = node;
        rects->tail = node;
     }
   else
     {
        rects->head = node;
        rects->tail = node;
     }
}

static inline void rect_list_append(list_t *rects, const rect_t r)
{
   rect_node_t *rect_node;

   rect_node = (rect_node_t *)rect_list_node_pool_get();
   rect_node->rect = r;
   rect_node->_lst = list_node_zeroed;

   rect_list_append_node(rects, (list_node_t *)rect_node);
}

static inline void rect_list_append_xywh(list_t *rects,
                                         int x,
                                         int y,
                                         int w,
                                         int h)
{
   rect_t r;

   rect_init(&r, x, y, w, h);
   rect_list_append(rects, r);
}

static inline void _calc_intra_rect_area(const rect_t a, const rect_t b,
                                         int *width, int *height)
{
   int max_left, min_right, max_top, min_bottom;

   if (a.left < b.left)
      max_left = b.left;
   else
      max_left = a.left;

   if (a.right < b.right)
      min_right = a.right;
   else
      min_right = b.right;

   *width = min_right - max_left;

   if (a.top < b.top)
      max_top = b.top;
   else
      max_top = a.top;

   if (a.bottom < b.bottom)
      min_bottom = a.bottom;
   else
      min_bottom = b.bottom;

   *height = min_bottom - max_top;
}

static inline void _split_strict(list_t *dirty, const rect_t current, rect_t r)
{
   int h_1, h_2, w_1, w_2;

   h_1 = current.top - r.top;
   h_2 = r.bottom - current.bottom;
   w_1 = current.left - r.left;
   w_2 = r.right - current.right;

   if (h_1 > 0)
     {
        /*    .--.r (b)                .---.r2
         *    |  |                     |   |
         *  .-------.cur (a) .---.r    '---'
         *  | |  |  |     -> |   |   +
         *  | `--'  |        `---'
         *  `-------'
         */
        rect_list_append_xywh(dirty, r.left, r.top, r.width, h_1);
        r.height -= h_1;
        r.top = current.top;
     }

   if (h_2 > 0)
     {
        /*  .-------.cur (a)
         *  | .---. |        .---.r
         *  | |   | |    ->  |   |
         *  `-------'        `---'   +  .---.r2
         *    |   |                     |   |
         *    `---'r (b)                `---'
         */
        rect_list_append_xywh(dirty, r.left, current.bottom, r.width,
                              h_2);
        r.height -= h_2;
     }

   if (w_1 > 0)
      /* (b) r  .----.cur (a)
       *     .--|-.  |      .--.r2   .-.r
       *     |  | |  |  ->  |  |   + | |
       *     `--|-'  |      `--'     `-'
       *        `----'
       */
        rect_list_append_xywh(dirty, r.left, r.top, w_1, r.height);  /* not necessary to keep these, r (b) will be destroyed */

   /* r.width -= w_1; */
   /* r.left = current.left; */

   if (w_2 > 0)
      /*  .----.cur (a)
       *  |    |
       *  |  .-|--.r (b)  .-.r   .--.r2
       *  |  | |  |    -> | |  + |  |
       *  |  `-|--'       `-'    `--'
       *  `----'
       */
        rect_list_append_xywh(dirty, current.right, r.top, w_2,
                            r.height);  /* not necessary to keep this, r (b) will be destroyed */

   /* r.width -= w_2; */
}

static inline void _calc_intra_outer_rect_area(const rect_t a, const rect_t b,
                                               rect_t *intra, rect_t *outer)
{
   int min_left, max_left, min_right, max_right;
   int min_top, max_top, min_bottom, max_bottom;

   if (a.left < b.left)
     {
        max_left = b.left;
        min_left = a.left;
     }
   else
     {
        max_left = a.left;
        min_left = b.left;
     }

   if (a.right < b.right)
     {
        min_right = a.right;
        max_right = b.right;
     }
   else
     {
        min_right = b.right;
        max_right = a.right;
     }

   intra->left = max_left;
   intra->right = min_right;
   intra->width = min_right - max_left;

   outer->left = min_left;
   outer->right = max_right;
   outer->width = max_right - min_left;

   if (a.top < b.top)
     {
        max_top = b.top;
        min_top = a.top;
     }
   else
     {
        max_top = a.top;
        min_top = b.top;
     }

   if (a.bottom < b.bottom)
     {
        min_bottom = a.bottom;
        max_bottom = b.bottom;
     }
   else
     {
        min_bottom = b.bottom;
        max_bottom = a.bottom;
     }

   intra->top = max_top;
   intra->bottom = min_bottom;
   intra->height = min_bottom - max_top;
   if ((intra->width > 0) && (intra->height > 0))
      intra->area = intra->width * intra->height;
   else
      intra->area = 0;

   outer->top = min_top;
   outer->bottom = max_bottom;
   outer->height = max_bottom - min_top;
   outer->area = outer->width * outer->height;
}

enum
{
   SPLIT_FUZZY_ACTION_NONE,
   SPLIT_FUZZY_ACTION_SPLIT,
   SPLIT_FUZZY_ACTION_MERGE
};

static inline int _split_fuzzy(list_t *dirty, const rect_t a, rect_t *b)
{
   int h_1, h_2, w_1, w_2, action;

   h_1 = a.top - b->top;
   h_2 = b->bottom - a.bottom;
   w_1 = a.left - b->left;
   w_2 = b->right - a.right;

   action = SPLIT_FUZZY_ACTION_NONE;

   if (h_1 > 0)
     {
        /*    .--.r (b)                .---.r2
         *    |  |                     |   |
         *  .-------.cur (a) .---.r    '---'
         *  | |  |  |     -> |   |   +
         *  | `--'  |        `---'
         *  `-------'
         */
        rect_list_append_xywh(dirty, b->left, b->top, b->width, h_1);
        b->height -= h_1;
        b->top = a.top;
        action = SPLIT_FUZZY_ACTION_SPLIT;
     }

   if (h_2 > 0)
     {
        /*  .-------.cur (a)
         *  | .---. |        .---.r
         *  | |   | |    ->  |   |
         *  `-------'        `---'   +  .---.r2
         *    |   |                     |   |
         *    `---'r (b)                `---'
         */
        rect_list_append_xywh(dirty, b->left, a.bottom, b->width, h_2);
        b->height -= h_2;
        action = SPLIT_FUZZY_ACTION_SPLIT;
     }

   if (((w_1 > 0) || (w_2 > 0)) && (a.height == b->height))
      return SPLIT_FUZZY_ACTION_MERGE;

   if (w_1 > 0)
     {
        /* (b)  r  .----.cur (a)
         *      .--|-.  |      .--.r2   .-.r
         *      |  | |  |  ->  |  |   + | |
         *      `--|-'  |      `--'     `-'
         *         `----'
         */
        rect_list_append_xywh(dirty, b->left, b->top, w_1, b->height);
        /* not necessary to keep these, r (b) will be destroyed */
        /* b->width -= w_1; */
        /* b->left = a.left; */
        action = SPLIT_FUZZY_ACTION_SPLIT;
     }

   if (w_2 > 0)
     {
        /* .----.cur (a)
         * |    |
         * |  .-|--.r (b)  .-.r   .--.r2
         * |  | |  |    -> | |  + |  |
         * |  `-|--'       `-'    `--'
         * `----'
         */
        rect_list_append_xywh(dirty, a.right, b->top, w_2, b->height);
        /* not necessary to keep these, r (b) will be destroyed */
        /* b->width -= w_2; */
        action = SPLIT_FUZZY_ACTION_SPLIT;
     }

   return action;
}

#if 0
static void rect_list_node_pool_set_max(int max)
{
   int diff;

   diff = list_node_pool.len - max;
   for (; diff > 0 && list_node_pool.node != NULL; diff--)
     {
        list_node_t *node;

        node = list_node_pool.node;
        list_node_pool.node = node->next;
        list_node_pool.len--;

        free(node);
     }

   list_node_pool.max = max;
}
#endif

static void rect_list_node_pool_flush(void)
{
   while (list_node_pool.node)
     {
        list_node_t *node;

        node = list_node_pool.node;
        list_node_pool.node = node->next;
        list_node_pool.len--;

        free(node);
     }
}



static inline void rect_list_node_pool_put(list_node_t *node)
{
   if (list_node_pool.len < list_node_pool.max)
     {
        node->next = list_node_pool.node;
        list_node_pool.node = node;
        list_node_pool.len++;
     }
   else
        free(node);
}

#if 0
static void rect_print(const rect_t r)
{
        printf("<rect(%d, %d, %d, %d)>", r.left, r.top, r.width, r.height);
}

static void rect_list_print(const list_t rects)
{
   list_node_t *node;
   int len;

   len = 0;
   for (node = rects.head; node != NULL; node = node->next)
      len++;

        printf("[");
   for (node = rects.head; node != NULL; node = node->next)
     {
        rect_print(((rect_node_t *)node)->rect);
        if (node->next)
          {
                  putchar(',');
             if (len < 4)
                  putchar(' ');
             else
               {
                  putchar('\n');
                  putchar(' ');
               }
          }
     }
   printf("]\n");
}
#endif

static inline list_node_t *
rect_list_unlink_next(list_t *rects, list_node_t *parent_node)
{
   list_node_t *node;

   if (parent_node)
     {
        node = parent_node->next;
        parent_node->next = node->next;
     }
   else
     {
        node = rects->head;
        rects->head = node->next;
     }

   if (rects->tail == node)
      rects->tail = parent_node;

   *node = list_node_zeroed;
   return node;
}

static inline void rect_list_del_next(list_t *rects, list_node_t *parent_node)
{
   list_node_t *node;

   node = rect_list_unlink_next(rects, parent_node);
        rect_list_node_pool_put(node);
}

static void rect_list_clear(list_t *rects)
{
   list_node_t *node;

   node = rects->head;
   while (node)
     {
        list_node_t *aux;

        aux = node->next;
        rect_list_node_pool_put(node);
        node = aux;
     }
   *rects = list_zeroed;
}

static void rect_list_del_split_strict(list_t *rects, const rect_t del_r)
{
   list_t modified = list_zeroed;
   list_node_t *cur_node, *prev_node;

   prev_node = NULL;
   cur_node = rects->head;
   while (cur_node)
     {
        int intra_width, intra_height;
        rect_t current;

        current = ((rect_node_t *)cur_node)->rect;

        _calc_intra_rect_area(del_r, current, &intra_width,
                              &intra_height);
        if ((intra_width <= 0) || (intra_height <= 0))
          {
             /*  .---.current      .---.del_r
              *  |   |             |   |
              *  `---+---.del_r    `---+---.current
              *      |   |             |   |
              *      `---'             `---'
              * no intersection, nothing to do
              */
             prev_node = cur_node;
             cur_node = cur_node->next;
          }
        else if ((intra_width == current.width) && (intra_height
                                                    == current.height))
          {
             /*  .-------.del_r
              *  | .---. |
              *  | |   | |
              *  | `---'current
              *  `-------'
              * current is contained, remove from rects
              */
             cur_node = cur_node->next;
             rect_list_del_next(rects, prev_node);
          }
        else
          {
             _split_strict(&modified, del_r, current);
             cur_node = cur_node->next;
             rect_list_del_next(rects, prev_node);
          }
     }

   rect_list_concat(rects, &modified);
}

#if 0
static void rect_list_add_split_strict(list_t *rects, list_node_t *node)
{
   list_t dirty = list_zeroed;
   list_t new_dirty = list_zeroed;
   list_node_t *cur_node;

   if (!rects->head)
     {
        rect_list_append_node(rects, node);
        return;
     }

        rect_list_append_node(&dirty, node);

   cur_node = rects->head;
   while (dirty.head)
     {
        rect_t current;

        if (!cur_node)
          {
             rect_list_concat(rects, &dirty);
             break;
          }

        current = ((rect_node_t *)cur_node)->rect;

        while (dirty.head)
          {
             int intra_width, intra_height;
             rect_t r;

             r = ((rect_node_t *)dirty.head)->rect;
             _calc_intra_rect_area(r, current, &intra_width,
                                   &intra_height);
             if ((intra_width == r.width) && (intra_height
                                              == r.height))
                /*  .-------.cur
                 *  | .---.r|
                 *  | |   | |
                 *  | `---' |
                 *  `-------'
                 */
                rect_list_del_next(&dirty, NULL);
             else if ((intra_width <= 0) || (intra_height <= 0))
               {
                  /*  .---.cur     .---.r
                   *  |   |        |   |
                   *  `---+---.r   `---+---.cur
                   *      |   |        |   |
                   *      `---'        `---'
                   */
                  list_node_t *tmp;
                  tmp = rect_list_unlink_next(&dirty, NULL);
                  rect_list_append_node(&new_dirty, tmp);
               }
             else
               {
                  _split_strict(&new_dirty, current, r);
                  rect_list_del_next(&dirty, NULL);
               }
          }
        dirty = new_dirty;
        new_dirty = list_zeroed;

        cur_node = cur_node->next;
     }
}
#endif

static list_node_t *
rect_list_add_split_fuzzy(list_t *rects, list_node_t *node, int accepted_error)
{
   list_t dirty = list_zeroed;
   list_node_t *old_last;

   old_last = rects->tail;

   if (!rects->head)
     {
        rect_list_append_node(rects, node);
        return old_last;
     }

        rect_list_append_node(&dirty, node);
   while (dirty.head)
     {
        list_node_t *d_node, *cur_node, *prev_cur_node;
        int keep_dirty;
        rect_t r;

        d_node = rect_list_unlink_next(&dirty, NULL);
        r = ((rect_node_t *)d_node)->rect;

        prev_cur_node = NULL;
        cur_node = rects->head;
        keep_dirty = 1;
        while (cur_node)
          {
             int area, action;
             rect_t current, intra, outer;

             current = ((rect_node_t *)cur_node)->rect;

             _calc_intra_outer_rect_area(r, current, &intra, &outer);
             area = current.area + r.area - intra.area;

             if ((intra.width == r.width) && (intra.height
                                              == r.height))
               {
                  /*  .-------.cur
                   *  | .---.r|
                   *  | |   | |
                   *  | `---' |
                   *  `-------'
                   */
                  keep_dirty = 0;
                  break;
               }
             else if ((intra.width == current.width)
                      && (intra.height == current.height))
               {
                  /* .-------.r
                   * | .---.cur
                   * | |   | |
                   * | `---' |
                   * `-------'
                   */
                  if (old_last == cur_node)
                     old_last = prev_cur_node;

                  cur_node = cur_node->next;
                  rect_list_del_next(rects, prev_cur_node);
               }
             else if ((outer.area - area) <= accepted_error)
               {
                  /* .-----------. bounding box (outer)
                   * |.---. .---.|
                   * ||cur| |r  ||
                   * ||   | |   ||
                   * |`---' `---'|
                   * `-----------'
                   * merge them, remove both and add merged
                   */
                  rect_node_t *n;

                  if (old_last == cur_node)
                     old_last = prev_cur_node;

                  n = (rect_node_t *)rect_list_unlink_next(
                        rects, prev_cur_node);
                  n->rect = outer;
                       rect_list_append_node(&dirty, (list_node_t *)n);

                  keep_dirty = 0;
                  break;
               }
             else if (intra.area <= accepted_error)
               {
                  /*  .---.cur     .---.r
                   *  |   |        |   |
                   *  `---+---.r   `---+---.cur
                   *      |   |        |   |
                   *      `---'        `---'
                   *  no split, no merge
                   */
                  prev_cur_node = cur_node;
                  cur_node = cur_node->next;
               }
             else
               {
                  /* split is required */
                  action = _split_fuzzy(&dirty, current, &r);
                  if (action == SPLIT_FUZZY_ACTION_MERGE)
                    {
/* horizontal merge is possible: remove both, add merged */
                       rect_node_t *n;

                       if (old_last == cur_node)
                          old_last = prev_cur_node;

                       n
                          = (rect_node_t *)rect_list_unlink_next(
                                rects,
                                prev_cur_node);

                       n->rect.left = outer.left;
                       n->rect.width = outer.width;
                       n->rect.right = outer.right;
                       n->rect.area = outer.width * r.height;
                       rect_list_append_node(&dirty,
                                             (list_node_t *)n);
                    }
                  else if (action == SPLIT_FUZZY_ACTION_NONE)
                    {
/*
 * this rect check was totally useless,
 * should never happen
 */
/* prev_cur_node = cur_node; */
/* cur_node = cur_node->next; */
                       printf("Should not get here!\n");
                       abort();
                    }

                  keep_dirty = 0;
                  break;
               }
          }
        if (EINA_UNLIKELY(keep_dirty))
           rect_list_append_node(rects, d_node);
        else
           rect_list_node_pool_put(d_node);
     }

   return old_last;
}

static inline void _calc_outer_rect_area(const rect_t a, const rect_t b,
                                         rect_t *outer)
{
   int min_left, max_right;
   int min_top, max_bottom;

   if (a.left < b.left)
      min_left = a.left;
   else
      min_left = b.left;

   if (a.right < b.right)
      max_right = b.right;
   else
      max_right = a.right;

   outer->left = min_left;
   outer->right = max_right;
   outer->width = max_right - min_left;

   if (a.top < b.top)
      min_top = a.top;
   else
      min_top = b.top;

   if (a.bottom < b.bottom)
      max_bottom = b.bottom;
   else
      max_bottom = a.bottom;

   outer->top = min_top;
   outer->bottom = max_bottom;
   outer->height = max_bottom - min_top;

   outer->area = outer->width * outer->height;
}

static void rect_list_merge_rects(list_t *rects,
                                  list_t *to_merge,
                                  int accepted_error)
{
   while (to_merge->head)
     {
        list_node_t *node, *parent_node;
        rect_t r1;
        int merged;

        r1 = ((rect_node_t *)to_merge->head)->rect;

        merged = 0;
        parent_node = NULL;
        node = rects->head;
        while (node)
          {
             rect_t r2, outer;
             int area;

             r2 = ((rect_node_t *)node)->rect;

             _calc_outer_rect_area(r1, r2, &outer);
             area = r1.area + r2.area; /* intra area is taken as 0 */
             if (outer.area - area <= accepted_error)
               {
                  /*
                   * remove both r1 and r2, create r3
                   * actually r3 uses r2 instance, saves memory
                   */
                  rect_node_t *n;

                  n = (rect_node_t *)rect_list_unlink_next(
                        rects, parent_node);
                  n->rect = outer;
                  rect_list_append_node(to_merge,
                                        (list_node_t *)n);
                  merged = 1;
                  break;
               }

             parent_node = node;
             node = node->next;
          }

        if (!merged)
          {
             list_node_t *n;
             n = rect_list_unlink_next(to_merge, NULL);
                  rect_list_append_node(rects, n);
          }
        else
                  rect_list_del_next(to_merge, NULL);
     }
}

static void rect_list_add_split_fuzzy_and_merge(list_t *rects,
                                                list_node_t *node,
                                                int split_accepted_error,
                                                int merge_accepted_error)
{
   list_node_t *n;

   n = rect_list_add_split_fuzzy(rects, node, split_accepted_error);
   if (n && n->next)
     {
        list_t to_merge;

        /* split list into 2 segments, already merged and to merge */
        to_merge.head = n->next;
        to_merge.tail = rects->tail;
        rects->tail = n;
        n->next = NULL;

        rect_list_merge_rects(rects, &to_merge, merge_accepted_error);
     }
}

static inline void _splitter_new(Eina_Tiler *t)
{
   t->splitter.rects = list_zeroed;
   t->splitter.need_merge = EINA_FALSE;
}

static inline void _splitter_del(Eina_Tiler *t)
{
   rect_list_clear(&t->splitter.rects);
   rect_list_node_pool_flush();
}

static inline void _splitter_tile_size_set(Eina_Tiler *t,
                                           int w EINA_UNUSED,
                                           int h EINA_UNUSED)
{
   /* TODO are w and h used for something? */
   t->splitter.rects = list_zeroed;
}

static inline Eina_Bool _splitter_rect_add(Eina_Tiler *t, Eina_Rectangle *rect)
{
   rect_node_t *rn;

   //printf("ACCOUNTING[1]: add_redraw: %4d,%4d %3dx%3d\n", x, y, w, h);
   if (t->rounding)
     {
        // FIXME: Seems that this trigger overdraw bug in Evas implementation and
        // was disable. Need to investigate.
        rect->x >>= 1;
        rect->y >>= 1;
        rect->w += 2;
        rect->w >>= 1;
        rect->h += 2;
        rect->h >>= 1;
     }

   rn = (rect_node_t *)rect_list_node_pool_get();
   rn->_lst = list_node_zeroed;
   rect_init(&rn->rect, rect->x, rect->y, rect->w, rect->h);
   //printf("ACCOUNTING[2]: add_redraw: %4d,%4d %3dx%3d\n", x, y, w, h);
   //testing on my core2 duo desktop - fuzz of 32 or 48 is best.
   rect_list_add_split_fuzzy_and_merge(&t->splitter.rects,
                                       (list_node_t *)rn,
                                       t->tile.w * t->tile.h,
                                       t->tile.w * t->tile.h);
   return EINA_TRUE;
}

static inline void _splitter_rect_del(Eina_Tiler *t, Eina_Rectangle *rect)
{
   rect_t r;

   if (!t->splitter.rects.head)
      return;

   if (t->rounding)
     {
        rect->x += 1;
        rect->y += 1;
        rect->x >>= 1;
        rect->y >>= 1;
        rect->w -= 1;
        rect->w >>= 1;
        rect->h -= 1;
        rect->h >>= 1;
     }

   if ((rect->w <= 0) || (rect->h <= 0))
      return;

   rect_init(&r, rect->x, rect->y, rect->w, rect->h);
   //fprintf(stderr, "ACCOUNTING: del_redraw: %4d,%4d %3dx%3d\n", x, y, w, h);

   rect_list_del_split_strict(&t->splitter.rects, r);
   t->splitter.need_merge = EINA_TRUE;
   return;
}

static inline void _splitter_clear(Eina_Tiler *t)
{
   rect_list_clear(&t->splitter.rects);
   t->splitter.need_merge = EINA_FALSE;
}
/* end of splitter algorithm */

static Eina_Bool _iterator_next(Eina_Iterator_Tiler *it, void **data)
{
   list_node_t *n;

   for (n = it->curr; n; n = n->next)
     {
        rect_t cur;

        cur = ((rect_node_t *)n)->rect;

        if (it->tiler->rounding)
          {
             it->r.x = cur.left << 1;
             it->r.y = cur.top << 1;
             it->r.w = cur.width << 1;
             it->r.h = cur.height << 1;
          }
        else
          {
             it->r.x = cur.left;
             it->r.y = cur.top;
             it->r.w = cur.width;
             it->r.h = cur.height;             
          }

        if (eina_rectangle_intersection(&it->r, &it->tiler->area) == EINA_FALSE)
           continue;

        if ((it->r.w <= 0) || (it->r.h <= 0))
           continue;

        it->curr = n->next;
        *(Eina_Rectangle **)data = &it->r;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void *_iterator_get_container(Eina_Iterator_Tiler *it)
{
   EINA_MAGIC_CHECK_TILER_ITERATOR(it, NULL);
   return (void *)it->tiler;
}

static void _iterator_free(Eina_Iterator_Tiler *it)
{
   EINA_MAGIC_CHECK_TILER_ITERATOR(it);
   free(it);
}

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI Eina_Tiler *eina_tiler_new(int w, int h)
{
   Eina_Tiler *t;

   if ((w <= 0) || (h <= 0))
     return NULL;

   t = calloc(1, sizeof(Eina_Tiler));
   t->last.add.w = -1;
   t->last.add.h = -1;
   t->last.del.w = -1;
   t->last.del.h = -1;
   t->area.w = w;
   t->area.h = h;
   t->tile.w = 32;
   t->tile.h = 32;
   t->rounding = EINA_TRUE;
   EINA_MAGIC_SET(t, EINA_MAGIC_TILER);
   _splitter_new(t);
   return t;
}

EAPI void eina_tiler_free(Eina_Tiler *t)
{
   if (!t)
     return;

   EINA_MAGIC_CHECK_TILER(t);
   _splitter_del(t);
   free(t);
}

EAPI void eina_tiler_area_size_set(Eina_Tiler *t, int w, int h)
{
   EINA_MAGIC_CHECK_TILER(t);
   if ((w <= 0) || (h <= 0))
     return;

   t->area.w = w;
   t->area.h = h;
}

EAPI void eina_tiler_area_size_get(const Eina_Tiler *t, int *w, int *h)
{
   EINA_MAGIC_CHECK_TILER(t);
   if (w) *w = t->area.w;
   if (h) *h = t->area.h;
}

EAPI void eina_tiler_tile_size_set(Eina_Tiler *t, int w, int h)
{
   EINA_MAGIC_CHECK_TILER(t);
   if ((w <= 0) || (h <= 0))
      return;

   if (w == 1 || h == 1) t->rounding = EINA_FALSE;
   t->tile.w = w;
   t->tile.h = h;
   _splitter_tile_size_set(t, w, h);
}

EAPI Eina_Bool
eina_tiler_empty(Eina_Tiler *t)
{
   EINA_MAGIC_CHECK_TILER(t, EINA_TRUE);

   return !memcmp(&t->splitter.rects, &list_zeroed, sizeof (list_t));
}

EAPI Eina_Bool eina_tiler_rect_add(Eina_Tiler *t, const Eina_Rectangle *r)
{
   Eina_Rectangle tmp;

   EINA_MAGIC_CHECK_TILER(t, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(r, EINA_FALSE);

   if ((r->w <= 0) || (r->h <= 0))
      return EINA_FALSE;

   tmp = *r;
   if (eina_rectangle_intersection(&tmp, &t->area) == EINA_FALSE)
      return EINA_FALSE;

   if ((tmp.w <= 0) || (tmp.h <= 0))
      return EINA_FALSE;

   if (!memcmp(&tmp, &t->last.add, sizeof (Eina_Rectangle)))
      return EINA_FALSE;

   t->last.add = tmp;

   return _splitter_rect_add(t, &tmp);
}

EAPI void eina_tiler_rect_del(Eina_Tiler *t, const Eina_Rectangle *r)
{
   Eina_Rectangle tmp;

   EINA_MAGIC_CHECK_TILER(t);
   EINA_SAFETY_ON_NULL_RETURN(r);

   if ((r->w <= 0) || (r->h <= 0))
      return;

   tmp = *r;
   if (eina_rectangle_intersection(&tmp, &t->area) == EINA_FALSE)
      return;

   if ((tmp.w <= 0) || (tmp.h <= 0))
      return;

   if (!memcmp(&tmp, &t->last.del, sizeof (Eina_Rectangle)))
      return;

   t->last.del = tmp;

   _splitter_rect_del(t, &tmp);
}

EAPI void eina_tiler_clear(Eina_Tiler *t)
{
   EINA_MAGIC_CHECK_TILER(t);
   _splitter_clear(t);
   t->last.add.w = -1;
   t->last.add.h = -1;
   t->last.del.w = -1;
   t->last.del.h = -1;
}

EAPI void
eina_tiler_strict_set(Eina_Tiler *t, Eina_Bool strict)
{
   EINA_MAGIC_CHECK_TILER(t);
   t->strict = strict;
}

EAPI Eina_Iterator *eina_tiler_iterator_new(const Eina_Tiler *t)
{
   Eina_Iterator_Tiler *it;

   if (!t) return NULL;

   EINA_MAGIC_CHECK_TILER(t, NULL);

   it = calloc(1, sizeof (Eina_Iterator_Tiler));
   if (!it)
      return NULL;

   it->tiler = t;

   if (t->splitter.need_merge == EINA_TRUE)
     {
        splitter_t *sp;
        list_t to_merge;

        to_merge = t->splitter.rects;
        if (t->strict)
          {
             rect_node_t *rn;
             list_node_t *n;

             // round up rects to tb->tile_size.w and tb->tile_size.h
             to_merge = list_zeroed;
             for (n = t->splitter.rects.head; n; n = n->next)
               {
                  int x1, x2, y1, y2;

                  x1 = ((rect_node_t *)n)->rect.left;
                  x2 = x1 + ((rect_node_t *)n)->rect.width;
                  y1 = ((rect_node_t *)n)->rect.top;
                  y2 = y1 + ((rect_node_t *)n)->rect.height;
                  x1 = t->tile.w * (x1 / t->tile.w);
                  y1 = t->tile.h * (y1 / t->tile.h);
                  x2 = t->tile.w * ((x2 + t->tile.w - 1) / t->tile.w);
                  y2 = t->tile.h * ((y2 + t->tile.h - 1) / t->tile.h);

                  rn = (rect_node_t *)rect_list_node_pool_get();
                  rn->_lst = list_node_zeroed;
                  rect_init(&rn->rect, x1, y1, x2 - x1, y2 - y1);
                  rect_list_add_split_fuzzy_and_merge(&to_merge,
                                                      (list_node_t *)rn,
                                                      t->tile.w * t->tile.h,
                                                      t->tile.w * t->tile.h);
               }
          }

        sp = (splitter_t *)&(t->splitter);
        sp->rects = list_zeroed;
        rect_list_merge_rects(&sp->rects, &to_merge, t->tile.w * t->tile.h);
        sp->need_merge = 0;
     }

   it->curr = it->tiler->splitter.rects.head;
   if (!it->tiler->splitter.rects.head)
     {
        free(it);
        return NULL;
     }

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(it,            EINA_MAGIC_TILER_ITERATOR);

   return &it->iterator;
}

EAPI Eina_Bool
eina_tiler_union(Eina_Tiler *dst,
                 Eina_Tiler *src)
{
   Eina_Iterator *itr;
   Eina_Rectangle *rect, _rect;

   if ((!src) || (!dst))
     return EINA_FALSE;

   EINA_MAGIC_CHECK_TILER(src, EINA_FALSE);
   EINA_MAGIC_CHECK_TILER(dst, EINA_FALSE);

   if ((src->area.w <= 0) || (src->area.h <=0))
     return EINA_FALSE;

   dst->area.w = MAX(src->area.w, dst->area.w);
   dst->area.h = MAX(src->area.h, dst->area.h);

   itr = eina_tiler_iterator_new(src);
   if (!itr)
     return EINA_FALSE;

   EINA_ITERATOR_FOREACH(itr, rect)
     {
        _rect = *rect;
        if (src->rounding)
          {
             _rect.w -= 1;
             _rect.h -= 1;
          }
        _splitter_rect_add(dst, &_rect);
     }

   if (rect)
     dst->last.add = *rect;

   eina_iterator_free(itr);

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_tiler_subtract(Eina_Tiler *dst,
                    Eina_Tiler *src)
{
   Eina_Iterator *itr;
   Eina_Rectangle *rect, _rect;

   if ((!src) || (!dst))
     return EINA_FALSE;

   EINA_MAGIC_CHECK_TILER(src, EINA_FALSE);
   EINA_MAGIC_CHECK_TILER(dst, EINA_FALSE);

   if ((src->area.w <= 0) || (src->area.h <= 0))
     return EINA_FALSE;

   itr = eina_tiler_iterator_new(src);
   if (!itr)
     return EINA_FALSE;

   EINA_ITERATOR_FOREACH(itr, rect)
     {
        _rect = *rect;
        if (src->rounding)
          {
             _rect.w -= 1;
             _rect.h -= 1;
          }
        _splitter_rect_del(dst, &_rect);
     }

   if (rect)
     dst->last.del = *rect;

   eina_iterator_free(itr);

   return EINA_TRUE;
}

EAPI Eina_Tiler *
eina_tiler_intersection(Eina_Tiler *t1,
                        Eina_Tiler *t2)
{
   Eina_Tiler *t = NULL;
   int w, h;

   Eina_Iterator *itr1 = NULL, *itr2 = NULL;
   Eina_Rectangle rect, *rect1 = NULL, *rect2 = NULL;

   EINA_MAGIC_CHECK_TILER(t1, NULL);
   EINA_MAGIC_CHECK_TILER(t2, NULL);

   if (!(eina_rectangles_intersect(&t1->area, &t2->area)))
     return NULL;

   itr1 = eina_tiler_iterator_new(t1);
   itr2 = eina_tiler_iterator_new(t2);

   if (!((itr1) && (itr2)))
     goto cleanup;

   if ((!eina_iterator_next(itr1, (void**)(void*)(&rect1))) && (!rect1))
     goto cleanup;
   if ((!eina_iterator_next(itr2, (void**)(void*)(&rect2))) && (!rect2))
     goto cleanup;

   w = MIN(t1->area.w, t2->area.w);
   h = MIN(t1->area.h, t2->area.h);
   t = eina_tiler_new(w, h);

   while((rect1) && (rect2))
     {
        if (eina_rectangles_intersect(rect1, rect2))
          {
             rect.x = MAX(rect1->x, rect2->x);
             rect.y = MAX(rect1->y, rect2->y);
             rect.w = MIN(rect1->x + rect1->w, rect2->x + rect2->w) - rect.x;
             rect.h = MIN(rect1->y + rect1->h, rect2->y + rect2->h) - rect.y;

             if ((t1->rounding) || (t2->rounding))
               {
                  rect.w -= 1;
                  rect.h -= 1;
               }

             _splitter_rect_add(t, &rect);

             t->last.add = rect;
          }

        if (rect1->x + rect1->w  < rect2->x + rect2->w)
          {
             if(eina_iterator_next(itr1, (void**)&rect1))
               continue;
          }
        else if (rect1->x + rect1->w > rect2->x + rect2->w)
          {
             if (eina_iterator_next(itr2, (void**)&rect2))
               continue;
          }

        if (eina_iterator_next(itr1, (void**)&rect1))
          continue;
        if (eina_iterator_next(itr2, (void**)&rect2))
          continue;

        break;
     }

   if (eina_tiler_empty(t))
     {
        eina_tiler_free(t);
        t = NULL;
     }

cleanup:
   eina_iterator_free(itr1);
   eina_iterator_free(itr2);

   return t;
}

EAPI Eina_Bool
eina_tiler_equal(Eina_Tiler *t1,
                 Eina_Tiler *t2)
{
   Eina_Iterator  *itr1 = NULL, *itr2 = NULL;
   Eina_Rectangle *rect1 = NULL, *rect2 = NULL;
   Eina_Bool      next_t1 = EINA_FALSE, next_t2 = EINA_FALSE;
   Eina_Bool r = EINA_FALSE;

   EINA_MAGIC_CHECK_TILER(t1, EINA_FALSE);
   EINA_MAGIC_CHECK_TILER(t2, EINA_FALSE);

   if (!(eina_rectangles_intersect(&t1->area, &t2->area)))
     return EINA_FALSE;

   itr1 = eina_tiler_iterator_new(t1);
   itr2 = eina_tiler_iterator_new(t2);

   if (!((itr1) && (itr2)))
     goto cleanup;

   if ((!eina_iterator_next(itr1, (void**)(void*)(&rect1))) && (!rect1))
     goto cleanup;
   if ((!eina_iterator_next(itr2, (void**)(void*)(&rect2))) && (!rect2))
     goto cleanup;

   while((rect1) && (rect2))
     {
        if (!eina_rectangles_intersect(rect1, rect2))
          break;

        if ((rect1->x != rect2->x) || (rect1->y != rect2->y) ||
            (rect1->w != rect2->w) || (rect1->h != rect2->h))
          break;

        next_t1 = eina_iterator_next(itr1, (void**)&rect1);
        next_t2 = eina_iterator_next(itr2, (void**)&rect2);

        if (next_t1 != next_t2)
          break;

        if (!next_t1 && !next_t2)
          break;
     }

   r = EINA_TRUE;

cleanup:
   eina_iterator_free(itr1);
   eina_iterator_free(itr2);

   return r;
}

struct _Eina_Tile_Grid_Slicer_Iterator
{
   Eina_Iterator iterator;
   Eina_Tile_Grid_Slicer priv;
};

typedef struct _Eina_Tile_Grid_Slicer_Iterator Eina_Tile_Grid_Slicer_Iterator;

static void
eina_tile_grid_slicer_iterator_free(Eina_Tile_Grid_Slicer_Iterator *it)
{
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_NONE);
   free(it);
}

static Eina_Bool
eina_tile_grid_slicer_iterator_next(Eina_Tile_Grid_Slicer_Iterator *it,
                                    void **data)
{
   return eina_tile_grid_slicer_next
             (&it->priv, (const Eina_Tile_Grid_Info **)data);
}

EAPI Eina_Iterator *
eina_tile_grid_slicer_iterator_new(int x,
                                   int y,
                                   int w,
                                   int h,
                                   int tile_w,
                                   int tile_h)
{
   Eina_Tile_Grid_Slicer_Iterator *it;

   if ((x < 0) || (y < 0) || (w <= 0) || (h <= 0) || 
       (tile_w <= 0) || (tile_h <= 0))
     return NULL;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(eina_tile_grid_slicer_iterator_next);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_tile_grid_slicer_iterator_free);

   eina_tile_grid_slicer_setup(&it->priv, x, y, w, h, tile_w, tile_h);

   return &it->iterator;
}
