#include "evas_common.h"

#ifdef EVAS_RECT_SPLIT

static const list_node_t list_node_zeroed = {.next = NULL};
static const list_t list_zeroed = {.head = NULL, .tail = NULL};


typedef struct list_node_pool
{
   list_node_t *node;
   int len;
   int max;
} list_node_pool_t;

static list_node_pool_t list_node_pool = {
   .node = NULL, .len = 0, .max = 1024
};

void
rect_list_node_pool_set_max(int max)
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

void
rect_list_node_pool_flush(void)
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

inline list_node_t *
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
   else return malloc(sizeof(rect_node_t));
}

inline void
rect_list_node_pool_put(list_node_t *node)
{
   if (list_node_pool.len < list_node_pool.max)
     {
        node->next = list_node_pool.node;
        list_node_pool.node = node;
        list_node_pool.len++;
     }
   else free(node);
}

inline void
rect_init(rect_t *r, int x, int y, int w, int h)
{
   r->area = w * h;
   
   r->left = x;
   r->top = y;
   
   r->right = x + w;
   r->bottom = y + h;
   
   r->width = w;
   r->height = h;
}

void
rect_print(const rect_t r)
{
   printf("<rect(%d, %d, %d, %d)>", r.left, r.top, r.width, r.height);
}

void
rect_list_print(const list_t rects)
{
   list_node_t *node;
   int len;
   
   len = 0;
   for (node = rects.head; node != NULL; node = node->next) len++;
   
   printf("[");
   for (node = rects.head; node != NULL; node = node->next)
     {
	rect_print(((rect_node_t *)node)->rect);
	if (node->next)
	  {
	     putchar(',');
	     if (len < 4) putchar(' ');
	     else
	       {
		  putchar('\n');
		  putchar(' ');
	       }
	  }
     }
   printf("]\n");
}

inline void
rect_list_append_node(list_t *rects, list_node_t *node)
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

inline void
rect_list_append(list_t *rects, const rect_t r)
{
   rect_node_t *rect_node;
   
   rect_node = (rect_node_t *)rect_list_node_pool_get();
   rect_node->rect = r;
   rect_node->_lst = list_node_zeroed;
   
   rect_list_append_node(rects, (list_node_t *)rect_node);
}

inline void
rect_list_append_xywh(list_t *rects, int x, int y, int w, int h)
{
   rect_t r;
   
   rect_init(&r, x, y, w, h);
   rect_list_append(rects, r);
}

inline void
rect_list_concat(list_t *rects, list_t *other)
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

inline list_node_t *
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
   
   if (rects->tail == node) rects->tail = parent_node;
   *node = list_node_zeroed;
   return node;
}

inline void
rect_list_del_next(list_t *rects, list_node_t *parent_node)
{
    list_node_t *node;

    node = rect_list_unlink_next(rects, parent_node);
    rect_list_node_pool_put(node);
}

void
rect_list_clear(list_t *rects)
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

static inline void
_calc_intra_rect_area(const rect_t a, const rect_t b, int *width, int *height)
{
   int max_left, min_right, max_top, min_bottom;
   
   if (a.left < b.left) max_left = b.left;
   else max_left = a.left;
   
   if (a.right < b.right) min_right = a.right;
   else min_right = b.right;
   
   *width = min_right - max_left;
   
   if (a.top < b.top) max_top = b.top;
   else max_top = a.top;
   
   if (a.bottom < b.bottom) min_bottom = a.bottom;
   else min_bottom = b.bottom;
   
   *height = min_bottom - max_top;
}

static inline void
_split_strict(list_t *dirty, const rect_t current, rect_t r)
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
        rect_list_append_xywh(dirty, r.left, current.bottom, r.width, h_2);
        r.height -= h_2;
     }
   
   if (w_1 > 0)
     {
        /* (b) r  .----.cur (a)
         *     .--|-.  |      .--.r2   .-.r
         *     |  | |  |  ->  |  |   + | |
         *     `--|-'  |      `--'     `-'
         *        `----'
         */
        rect_list_append_xywh(dirty, r.left, r.top, w_1, r.height);
        /* not necessary to keep these, r (b) will be destroyed */
        /* r.width -= w_1; */
        /* r.left = current.left; */
     }
   
   if (w_2 > 0)
     {
        /*  .----.cur (a)
         *  |    |
         *  |  .-|--.r (b)  .-.r   .--.r2
         *  |  | |  |    -> | |  + |  |
         *  |  `-|--'       `-'    `--'
         *  `----'
         */
        rect_list_append_xywh(dirty, current.right, r.top, w_2, r.height);
        /* not necessary to keep this, r (b) will be destroyed */
        /* r.width -= w_2; */
     }
}

void
rect_list_del_split_strict(list_t *rects, const rect_t del_r)
{
   list_t modified = list_zeroed;
   list_node_t *cur_node, *prev_node;
   
   prev_node = NULL;
   cur_node = rects->head;
   while (cur_node)
     {
        int intra_width, intra_height;
        rect_t current;
        
        current = ((rect_node_t*)cur_node)->rect;
	
        _calc_intra_rect_area(del_r, current, &intra_width, &intra_height);
        if ((intra_width <= 0) || (intra_height <= 0))
          {
             /*  .---.current      .---.del_r
              *  |   |             |   |
              *  `---+---.del_r    `---+---.current
              *      |   |             |   |
              *      `---'             `---'
              * no interception, nothing to do
              */
              prev_node = cur_node;
              cur_node = cur_node->next;
          }
        else if ((intra_width == current.width) &&
                 (intra_height == current.height))
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

void
rect_list_add_split_strict(list_t *rects, list_node_t *node)
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
	
        current = ((rect_node_t*)cur_node)->rect;
	
        while (dirty.head)
	  {
	     int intra_width, intra_height;
	     rect_t r;
	     
	     r = ((rect_node_t *)dirty.head)->rect;
	     _calc_intra_rect_area(r, current, &intra_width, &intra_height);
	     if ((intra_width == r.width) && (intra_height == r.height))
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

static inline void
_calc_intra_outer_rect_area(const rect_t a, const rect_t b,
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

static inline int
_split_fuzzy(list_t *dirty, const rect_t a, rect_t *b)
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

list_node_t *
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
	     
	     if ((intra.width == r.width) && (intra.height == r.height))
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
	     else if ((intra.width == current.width) &&
		      (intra.height == current.height))
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
		  
		  n = (rect_node_t *)rect_list_unlink_next(rects, prev_cur_node);
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
		       
		       n = (rect_node_t *)
			 rect_list_unlink_next(rects, prev_cur_node);
		       
		       n->rect.left = outer.left;
		       n->rect.width = outer.width;
		       n->rect.right = outer.right;
		       n->rect.area = outer.width * r.height;
		       rect_list_append_node(&dirty, (list_node_t *)n);
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
	
        if (UNLIKELY(keep_dirty)) rect_list_append_node(rects, d_node);
        else rect_list_node_pool_put(d_node);
    }

    return old_last;
}

static inline void
_calc_outer_rect_area(const rect_t a, const rect_t b, rect_t *outer)
{
   int min_left, max_right;
   int min_top, max_bottom;
   
   if (a.left < b.left) min_left = a.left;
   else min_left = b.left;
   
   if (a.right < b.right) max_right = b.right;
   else max_right = a.right;
   
   outer->left = min_left;
   outer->right = max_right;
   outer->width = max_right - min_left;
   
   if (a.top < b.top) min_top = a.top;
   else min_top = b.top;
   
   if (a.bottom < b.bottom) max_bottom = b.bottom;
   else max_bottom = a.bottom;
   
   outer->top = min_top;
   outer->bottom = max_bottom;
   outer->height = max_bottom - min_top;
   
   outer->area = outer->width * outer->height;
}

void
rect_list_merge_rects(list_t *rects, list_t *to_merge, int accepted_error)
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
        while (node != NULL)
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
		  
		  n = (rect_node_t *)rect_list_unlink_next(rects, parent_node);
		  n->rect = outer;
		  rect_list_append_node(to_merge, (list_node_t *)n);
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

void
rect_list_add_split_fuzzy_and_merge(list_t *rects,
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
#endif /* EVAS_RECT_SPLIT */

#define TILE(tb, x, y) ((tb)->tiles.tiles[((y) * (tb)->tiles.w) + (x)])

#ifdef RECTUPDATE
#elif defined(EVAS_RECT_SPLIT)
#else
static int  tilebuf_x_intersect(Tilebuf *tb, int x, int w, int *x1, int *x2, int *x1_fill, int *x2_fill);
static int  tilebuf_y_intersect(Tilebuf *tb, int y, int h, int *y1, int *y2, int *y1_fill, int *y2_fill);
static int  tilebuf_intersect(int tsize, int tlen, int tnum, int x, int w, int *x1, int *x2, int *x1_fill, int *x2_fill);
#endif
static void tilebuf_setup(Tilebuf *tb);

EAPI void
evas_common_tilebuf_init(void)
{
}

EAPI Tilebuf *
evas_common_tilebuf_new(int w, int h)
{
   Tilebuf *tb;

   tb = calloc(1, sizeof(Tilebuf));
   if (!tb) return NULL;

   tb->tile_size.w = 8;
   tb->tile_size.h = 8;
   tb->outbuf_w = w;
   tb->outbuf_h = h;

   return tb;
}

EAPI void
evas_common_tilebuf_free(Tilebuf *tb)
{
#ifdef RECTUPDATE
   evas_common_regionbuf_free(tb->rb);
#elif defined(EVAS_RECT_SPLIT)
   rect_list_clear(&tb->rects);
   rect_list_node_pool_flush();
#else
   if (tb->tiles.tiles) free(tb->tiles.tiles);
#endif
   free(tb);
}

EAPI void
evas_common_tilebuf_set_tile_size(Tilebuf *tb, int tw, int th)
{
   tb->tile_size.w = tw;
   tb->tile_size.h = th;
   tilebuf_setup(tb);
}

EAPI void
evas_common_tilebuf_get_tile_size(Tilebuf *tb, int *tw, int *th)
{
   if (tw) *tw = tb->tile_size.w;
   if (th) *th = tb->tile_size.h;
}

#ifdef EVAS_RECT_SPLIT
static inline int
_add_redraw(list_t *rects, int max_w, int max_h, int x, int y, int w, int h)
{
   rect_node_t *rn;

   if ((w <= 0) || (h <= 0)) return 0;
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, max_w, max_h);
   if ((w <= 0) || (h <= 0)) return 0;

   x >>= 1;
   y >>= 1;
   w += 2;
   w >>= 1;
   h += 2;
   h >>= 1;

   rn = (rect_node_t *)rect_list_node_pool_get();
   rn->_lst = list_node_zeroed;
   rect_init(&rn->rect, x, y, w, h);
   //fprintf(stderr, "ACCOUNTING: add_redraw: %4d,%4d %3dx%3d\n", x, y, w, h);
   //testing on my core2 duo desktop - fuzz of 32 or 48 is best.
#define FUZZ 32
   rect_list_add_split_fuzzy_and_merge(rects, (list_node_t *)rn,
                                       FUZZ * FUZZ, FUZZ * FUZZ);
   return 1;
}
#endif

EAPI int
evas_common_tilebuf_add_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
#ifdef RECTUPDATE
   int i;

   if ((w <= 0) || (h <= 0)) return 0;
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, tb->outbuf_w, tb->outbuf_h);
   if ((w <= 0) || (h <= 0)) return 0;
   for (i = 0; i < h; i++)
     evas_common_regionbuf_span_add(tb->rb, x, x + w - 1, y + i);
   return 1;
#elif defined(EVAS_RECT_SPLIT)
   return _add_redraw(&tb->rects, tb->outbuf_w, tb->outbuf_h, x, y, w, h);
#else
   int tx1, tx2, ty1, ty2, tfx1, tfx2, tfy1, tfy2, xx, yy;
   int num;

   if ((w <= 0) || (h <= 0)) return 0;
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, tb->outbuf_w, tb->outbuf_h);
   if ((w <= 0) || (h <= 0)) return 0;
   num = 0;
   /* wipes out any motion vectors in tiles it touches into redraws */
   if (tilebuf_x_intersect(tb, x, w, &tx1, &tx2, &tfx1, &tfx2) &&
       tilebuf_y_intersect(tb, y, h, &ty1, &ty2, &tfy1, &tfy2))
     {
        Tilebuf_Tile    *tbt;
        int             delta_x;
        int             delta_y;

        tbt = &(TILE(tb, tx1, ty1));
        delta_x = tx2 - tx1 + 1;
        delta_y = ty2 - ty1 + 1;
	for (yy = delta_y; yy > 0; yy--)
	  {
	     Tilebuf_Tile *tbti;

	     tbti = tbt;
	     for (xx = delta_x; xx > 0; xx--)
	       {
		  tbti->redraw = 1;
		  tbti++;
	       }
             tbt += tb->tiles.w;
	  }
	num = (tx2 - tx1 + 1) * (ty2 - ty1 + 1);
     }
   return num;
#endif
}

EAPI int
evas_common_tilebuf_del_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
#ifdef RECTUPDATE
   int i;

   for (i = 0; i < h; i++)
     evas_common_regionbuf_span_del(tb->rb, x, x + w - 1, y + i);
#elif defined(EVAS_RECT_SPLIT)
   rect_t r;
   
   if (!tb->rects.head) return 0;
   if ((w <= 0) || (h <= 0)) return 0;
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, tb->outbuf_w, tb->outbuf_h);
   if ((w <= 0) || (h <= 0)) return 0;

   x += 1;
   y += 1;
   x >>= 1;
   y >>= 1;
   w -= 1;
   w >>= 1;
   h -= 1;
   h >>= 1;
   
   if ((w <= 0) || (h <= 0)) return 0;

   rect_init(&r, x, y, w, h);
   //fprintf(stderr, "ACCOUNTING: del_redraw: %4d,%4d %3dx%3d\n", x, y, w, h);

   rect_list_del_split_strict(&tb->rects, r);
   tb->need_merge = 1;
   return 0;
#else
   int tx1, tx2, ty1, ty2, tfx1, tfx2, tfy1, tfy2, xx, yy;
   int num;

   num = 0;
   /* wipes out any motion vectors in tiles it touches into redraws */
   if (tilebuf_x_intersect(tb, x, w, &tx1, &tx2, &tfx1, &tfx2) &&
       tilebuf_y_intersect(tb, y, h, &ty1, &ty2, &tfy1, &tfy2))
     {
        Tilebuf_Tile    *tbt;
        int             delta_y;
        int             delta_x;

        if (!tfx1) tx1++;
	if (!tfx2) tx2--;
	if (!tfy1) ty1++;
	if (!tfy2) ty2--;

        tbt = &(TILE(tb, tx1, ty1));
        delta_x = tx2 - tx1 + 1;
        delta_y = ty2 - ty1 + 1;
	for (yy = delta_y; yy > 0; yy--)
	  {
	     Tilebuf_Tile       *tbti;

	     tbti = tbt;
	     for (xx = delta_x; xx > 0; xx--)
	       {
		  tbti->redraw = 0;
		  tbti++;
	       }
             tbt += tb->tiles.w;
	  }
	num = (tx2 - tx1 + 1) * (ty2 - ty1 + 1);
     }
   return num;
#endif
}

EAPI int
evas_common_tilebuf_add_motion_vector(Tilebuf *tb, int x, int y, int w, int h, int dx, int dy, int alpha)
{
#ifdef EVAS_RECT_SPLIT
   list_t lr = list_zeroed;
   int num;

   num = _add_redraw(&lr, tb->outbuf_w, tb->outbuf_h, x, y, w, h);
   num += _add_redraw(&lr, tb->outbuf_w, tb->outbuf_h, x + dx, y + dy, w, h);
   while (lr.head != NULL)
     {
        list_node_t *node = rect_list_unlink_next(&lr, NULL);
        rect_list_add_split_fuzzy_and_merge(&tb->rects, node,
                                            FUZZ * FUZZ, FUZZ * FUZZ);
     }
   return num;
#else
   /* FIXME: need to actually impliment motion vectors. for now it just */
   /*        implements redraws */
   int num;

   num = evas_common_tilebuf_add_redraw(tb, x, y, w, h);
   num += evas_common_tilebuf_add_redraw(tb, x + dx, y + dy, w, h);
   return num;
#endif
}

EAPI void
evas_common_tilebuf_clear(Tilebuf *tb)
{
#ifdef RECTUPDATE
   evas_common_regionbuf_clear(tb->rb);
#elif defined(EVAS_RECT_SPLIT)
   rect_list_clear(&tb->rects);
   tb->need_merge = 0;
#else
   if (!tb->tiles.tiles) return;
   memset(tb->tiles.tiles, 0, tb->tiles.w * tb->tiles.h * sizeof(Tilebuf_Tile));
#endif
}

EAPI Tilebuf_Rect *
evas_common_tilebuf_get_render_rects(Tilebuf *tb)
{
#ifdef RECTUPDATE
   return evas_common_regionbuf_rects_get(tb->rb);
#elif defined(EVAS_RECT_SPLIT)
   list_node_t *n;
   Tilebuf_Rect *rects = NULL;

   if (tb->need_merge) {
       list_t to_merge;
       to_merge = tb->rects;
       tb->rects = list_zeroed;
       rect_list_merge_rects(&tb->rects, &to_merge, FUZZ * FUZZ);
       tb->need_merge = 0;
   }

   for (n = tb->rects.head; n != NULL; n = n->next) {
       rect_t cur;

       cur = ((rect_node_t *)n)->rect;

       cur.left <<= 1;
       cur.top <<= 1;
       cur.width <<= 1;
       cur.height <<= 1;

       RECTS_CLIP_TO_RECT(cur.left, cur.top, cur.width, cur.height,
			  0, 0, tb->outbuf_w, tb->outbuf_h);
       if ((cur.width > 0) && (cur.height > 0))
	 {
	    Tilebuf_Rect *r;

	    r = malloc(sizeof(Tilebuf_Rect));
	    r->_list_data.next = NULL;
	    r->_list_data.prev = NULL;
	    r->_list_data.last = NULL;
	    r->x = cur.left;
	    r->y = cur.top;
	    r->w = cur.width;
	    r->h = cur.height;

	    rects = evas_object_list_append(rects, r);
	 }
   }
   return rects;

#else
   Tilebuf_Rect *rects = NULL;
   Tilebuf_Tile *tbt;
   int x, y;

   tbt = &(TILE(tb, 0, 0));
   for (y = 0; y < tb->tiles.h; y++)
     {
	for (x = 0; x < tb->tiles.w; x++, tbt++)
	  {
	     if (tbt->redraw)
	       {
                  Tilebuf_Tile *tbti;
		  int can_expand_x = 1, can_expand_y = 1;
		  Tilebuf_Rect *r = NULL;
		  int xx = 0, yy = 0;
		  r = malloc(sizeof(Tilebuf_Rect));
                  r->_list_data.next = NULL;
                  r->_list_data.prev = NULL;
                  r->_list_data.last = NULL;

/* amalgamate tiles */
#if 1
                  tbti = tbt;
		  while (can_expand_x)
		    {
                       tbti++;
		       xx++;
		       if ((x + xx) >= tb->tiles.w)
			 can_expand_x = 0;
		       else if (!(tbti->redraw))
			 can_expand_x = 0;
		       if (can_expand_x)
			 tbti->redraw = 0;
		    }
                  tbti = tbt;
		  while (can_expand_y)
		    {
		       int i;

                       tbti += tb->tiles.w;
		       yy++;
		       if ((y + yy) >= tb->tiles.h)
			 can_expand_y = 0;
		       if (can_expand_y)
			 {
                            Tilebuf_Tile *tbtj;

                            tbtj = tbti;
			    for (i = x; i < x + xx; i++, tbtj++)
			      {
				 if (!(tbtj->redraw))
				   {
				      can_expand_y = 0;
				      break;
				   }
			      }
			 }
		       if (can_expand_y)
			 {
                            Tilebuf_Tile *tbtj;

                            tbtj = tbti;
			    for (i = x; i < x + xx; i++, tbtj++)
			      tbtj->redraw = 0;
			 }
		    }
		  tbt->redraw = 0;
#else
		  xx = 1;
		  yy = 1;
#endif
		  r->x = x * tb->tile_size.w;
		  r->y = y * tb->tile_size.h;
		  r->w = (xx) * tb->tile_size.w;
		  r->h = (yy) * tb->tile_size.h;
		  rects = evas_object_list_append(rects, r);
		  x = x + (xx - 1);
                  tbt += xx - 1;
	       }
	  }
     }
   return rects;
#endif
}

EAPI void
evas_common_tilebuf_free_render_rects(Tilebuf_Rect *rects)
{
   while (rects)
     {
	Tilebuf_Rect *r;

	r = rects;
	rects = evas_object_list_remove(rects, r);
	free(r);
     }
}

/* need a way of getting rectangles to: blit, re-render */





/* internal usage */

static void
tilebuf_setup(Tilebuf *tb)
{
   if ((tb->outbuf_w <= 0) || (tb->outbuf_h <= 0)) return;
#ifdef RECTUPDATE
   tb->rb = evas_common_regionbuf_new(tb->outbuf_w, tb->outbuf_h);
#elif defined(EVAS_RECT_SPLIT)
   tb->rects = list_zeroed;
#else
   if (tb->tiles.tiles) free(tb->tiles.tiles);
   tb->tiles.tiles = NULL;

   tb->tiles.w = (tb->outbuf_w + (tb->tile_size.w - 1)) / tb->tile_size.w;
   tb->tiles.h = (tb->outbuf_h + (tb->tile_size.h - 1)) / tb->tile_size.h;

   tb->tiles.tiles = malloc(tb->tiles.w * tb->tiles.h * sizeof(Tilebuf_Tile));

   if (!tb->tiles.tiles)
     {
	tb->tiles.w = 0;
	tb->tiles.h = 0;
	return;
     }
   memset(tb->tiles.tiles, 0, tb->tiles.w * tb->tiles.h * sizeof(Tilebuf_Tile));
#endif
}

#ifdef RECTUPDATE
#elif defined(EVAS_RECT_SPLIT)
#else
static int
tilebuf_x_intersect(Tilebuf *tb, int x, int w, int *x1, int *x2, int *x1_fill, int *x2_fill)
{
   return tilebuf_intersect(tb->tile_size.w, tb->outbuf_w, tb->tiles.w,
			    x, w, x1, x2, x1_fill, x2_fill);
}

static int
tilebuf_y_intersect(Tilebuf *tb, int y, int h, int *y1, int *y2, int *y1_fill, int *y2_fill)
{
   return tilebuf_intersect(tb->tile_size.h, tb->outbuf_h, tb->tiles.h,
			    y, h, y1, y2, y1_fill, y2_fill);
}

static int
tilebuf_intersect(int tsize, int tlen, int tnum, int x, int w, int *x1, int *x2, int *x1_fill, int *x2_fill)
{
   int p1, p2;

   /* initial clip out of region */
   if ((x + w) <= 0) return 0;
   if (x >= tlen) return 0;

   /* adjust x & w so it all fits in region */
   if (x < 0)
     {
	w += x;
	x = 0;
     }
   if (w < 0) return 0;
   if ((x + w) > tlen) w = tlen - x;

   /* now figure if the first edge is fully filling its tile */
   p1 = (x) / tsize;
   if ((p1 * tsize) == (x)) *x1_fill = 1;
   else                     *x1_fill = 0;
   *x1 = p1;

   /* now figure if the last edge is fully filling its tile */
   p2 = (x + w - 1) / tsize;
   if (((p2 + 1) * tsize) == (x + w)) *x2_fill = 1;
   else                               *x2_fill = 0;
   *x2 = p2;

   return 1;
   tnum = 0;
}
#endif
