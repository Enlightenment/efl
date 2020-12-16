#include "evas_common_private.h"
#include "region.h"


#ifdef NEWTILER
#define MAXREG 24

EVAS_API void
evas_common_tilebuf_init(void)
{
}

EVAS_API Tilebuf *
evas_common_tilebuf_new(int w, int h)
{
   Tilebuf *tb = malloc(sizeof(Tilebuf));
   tb->outbuf_w = w;
   tb->outbuf_h = h;
   tb->region = region_new(tb->outbuf_w, tb->outbuf_h);
   return tb;
}

EVAS_API void
evas_common_tilebuf_free(Tilebuf *tb)
{
   region_free(tb->region);
   free(tb);
}

EVAS_API void
evas_common_tilebuf_set_tile_size(Tilebuf *tb EINA_UNUSED, int tw EINA_UNUSED, int th EINA_UNUSED)
{
}

EVAS_API void
evas_common_tilebuf_get_tile_size(Tilebuf *tb EINA_UNUSED, int *tw, int *th)
{
   if (tw) *tw = 1;
   if (th) *th = 1;
}

EVAS_API void
evas_common_tilebuf_tile_strict_set(Tilebuf *tb EINA_UNUSED, Eina_Bool strict EINA_UNUSED)
{
}

EVAS_API int
evas_common_tilebuf_add_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
   region_rect_add(tb->region, x, y, w, h);
   return 1;
}

EVAS_API int
evas_common_tilebuf_del_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
   region_rect_del(tb->region, x, y, w, h);
   return 1;
}

EVAS_API int
evas_common_tilebuf_add_motion_vector(Tilebuf *tb EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, int dx EINA_UNUSED, int dy EINA_UNUSED, int alpha EINA_UNUSED)
{
   return 0;
}

EVAS_API void
evas_common_tilebuf_clear(Tilebuf *tb)
{
   region_free(tb->region);
   tb->region = region_new(tb->outbuf_w, tb->outbuf_h);
}

static Region *
_region_round(Region *region, int tsize)
{
   Region *region2;
   Box *rects;
   int num, i, w, h;

   region_size_get(region, &w, &h);
   region2 = region_new(w, h);
   rects = region_rects(region);
   num = region_rects_num(region);
   for (i = 0; i < num; i++)
     {
        int x1, y1, x2, y2;

        x1 = (rects[i].x1 / tsize) * tsize;
        y1 = (rects[i].y1 / tsize) * tsize;
        x2 = ((rects[i].x2 + tsize - 1) / tsize) * tsize;
        y2 = ((rects[i].y2 + tsize - 1) / tsize) * tsize;
        region_rect_add(region2, x1, y1, x2 - x1, y2 - y1);
     }
   return region2;
}

EVAS_API Tilebuf_Rect *
evas_common_tilebuf_get_render_rects(Tilebuf *tb)
{
   Tilebuf_Rect *rects = NULL, *r, *rend, *rbuf;
   Region *region2;
   Box *rects2, *rs;
   int n, num, minx, miny, maxx, maxy;

   region2 = _region_round(tb->region, 16);
   if (!region2) return NULL;

   rects2 = region_rects(region2);
   if (!rects2)
     {
        region_free(region2);
        return NULL;
     }
   n = region_rects_num(region2);
   if (n <= 0)
     {
        region_free(region2);
        return NULL;
     }

   rbuf = malloc(n * sizeof(Tilebuf_Rect));
   if (!rbuf)
     {
        region_free(region2);
        return NULL;
     }

   rend = rbuf + n;
   rs = rects2;
   num = 0;

   minx = rs->x1;
   miny = rs->y1;
   maxx = rs->x2;
   maxy = rs->y2;
   for (r = rbuf; r < rend; r++)
     {
        if (rs->x1 < minx) minx = rs->x1;
        if (rs->y1 < miny) miny = rs->y1;
        if (rs->x2 > maxx) maxx = rs->x2;
        if (rs->y2 > maxy) maxy = rs->y2;
        EINA_INLIST_GET(r)->next = NULL;
        EINA_INLIST_GET(r)->prev = NULL;
        EINA_INLIST_GET(r)->last = NULL;
        r->x = rs->x1;
        r->y = rs->y1;
        r->w = rs->x2 - rs->x1;
        r->h = rs->y2 - rs->y1;
        rs++;
        rects = (Tilebuf_Rect *)
          eina_inlist_append(EINA_INLIST_GET(rects),
                             EINA_INLIST_GET(r));
        num++;
     }
   // if > max, then bounding box
   if (num > MAXREG)
     {
        r = rects;
        EINA_INLIST_GET(r)->next = NULL;
        EINA_INLIST_GET(r)->prev = NULL;
        EINA_INLIST_GET(r)->last = NULL;
        r->x = minx;
        r->y = miny;
        r->w = maxx - minx;
        r->h = maxy - miny;
     }
   region_free(region2);
   return rects;
}

EVAS_API void
evas_common_tilebuf_free_render_rects(Tilebuf_Rect *rects)
{
   free(rects);
}

#else

#define FUZZ 32
#define MAXREG 24
#define MAX_NODES 1024

static inline void rect_list_node_pool_flush(void);
static inline list_node_t *rect_list_node_pool_get(void);
static inline void rect_list_node_pool_put(list_node_t *node);
static inline void rect_init(rect_t *r, int x, int y, int w, int h);
static inline void rect_list_append_node(list_t *rects, list_node_t *node);
static inline void rect_list_append(list_t *rects, const rect_t r);
static inline void rect_list_append_xywh(list_t *rects, int x, int y, int w, int h);
static inline void rect_list_concat(list_t *rects, list_t *other);
static inline list_node_t *rect_list_unlink_next(list_t *rects, list_node_t *parent_node);
static inline void rect_list_del_next(list_t *rects, list_node_t *parent_node);
static inline void rect_list_clear(list_t *rects);
static inline void rect_list_del_split_strict(list_t *rects, const rect_t del_r);
static inline list_node_t *rect_list_add_split_fuzzy(list_t *rects, list_node_t *node, int accepted_error);
static inline void rect_list_merge_rects(list_t *rects, list_t *to_merge, int accepted_error);
static inline void rect_list_add_split_fuzzy_and_merge(list_t *rects, list_node_t *node, int split_accepted_error, int merge_accepted_error);

static const list_node_t list_node_zeroed = { NULL };
static const list_t list_zeroed = { NULL, NULL };

typedef struct list_node_pool
{
   list_node_t *node;
   int len;
   int max;
} list_node_pool_t;

static list_node_pool_t list_node_pool = { NULL, 0, MAX_NODES };

static inline void
rect_list_node_pool_flush(void)
{
   while (list_node_pool.node)
     {
        list_node_t *node = list_node_pool.node;
        list_node_pool.node = node->next;
        list_node_pool.len--;
        free(node);
     }
}

static inline list_node_t *
rect_list_node_pool_get(void)
{
   if (list_node_pool.node)
     {
        list_node_t *node = list_node_pool.node;
        list_node_pool.node = node->next;
        list_node_pool.len--;
        return node;
     }
   else return (list_node_t *)malloc(sizeof(rect_node_t));
}

static inline void
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

static inline void
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

static inline void
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

static inline void
rect_list_append(list_t *rects, const rect_t r)
{
   rect_node_t *rect_node = (rect_node_t *)rect_list_node_pool_get();
   rect_node->rect = r;
   rect_node->_lst = list_node_zeroed;
   rect_list_append_node(rects, (list_node_t *)rect_node);
}

static inline void
rect_list_append_xywh(list_t *rects, int x, int y, int w, int h)
{
   rect_t r;
   rect_init(&r, x, y, w, h);
   rect_list_append(rects, r);
}

static inline void
rect_list_concat(list_t *rects, list_t *other)
{
   if (!other->head) return;
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
   if (rects->tail == node) rects->tail = parent_node;
   *node = list_node_zeroed;
   return node;
}

static inline void
rect_list_del_next(list_t *rects, list_node_t *parent_node)
{
    list_node_t *node = rect_list_unlink_next(rects, parent_node);
    rect_list_node_pool_put(node);
}

static inline void
rect_list_clear(list_t *rects)
{
   list_node_t *node = rects->head;
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

static inline void
rect_list_del_split_strict(list_t *rects, const rect_t del_r)
{
   list_t modified = list_zeroed;
   list_node_t *cur_node, *prev_node;
   int intra_width, intra_height;
   rect_t current;

   prev_node = NULL;
   cur_node = rects->head;
   while (cur_node)
     {
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

static inline list_node_t *
rect_list_add_split_fuzzy(list_t *rects, list_node_t *node, int accepted_error)
{
   list_t dirty = list_zeroed;
   list_node_t *old_last = rects->tail;

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
	     else if ((intra.area - area) <= accepted_error)
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
		       /* this rect check was totally useless,
			* should never happen */
		       /* prev_cur_node = cur_node; */
		       /* cur_node = cur_node->next; */
		       WRN("Should not get here!");
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

static inline void
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
        while (node)
	  {
	     rect_t r2, outer;
	     int area;

	     r2 = ((rect_node_t *)node)->rect;
	     _calc_outer_rect_area(r1, r2, &outer);
	     area = r1.area + r2.area; /* intra area is taken as 0 */
	     if (outer.area - area <= accepted_error)
	       {
		  /* remove both r1 and r2, create r3
		   * actually r3 uses r2 instance, saves memory */
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

static inline void
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

static inline int
_add_redraw(list_t *rects, int x, int y, int w, int h, int fuzz)
{
   rect_node_t *rn;
   rn = (rect_node_t *)rect_list_node_pool_get();
   rn->_lst = list_node_zeroed;
   rect_init(&rn->rect, x, y, w, h);
   rect_list_add_split_fuzzy_and_merge(rects, (list_node_t *)rn, fuzz, fuzz);
   return 1;
}

/////////////////////////////////////////////////////////////////

EVAS_API void
evas_common_tilebuf_init(void)
{
}

EVAS_API Tilebuf *
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

EVAS_API void
evas_common_tilebuf_free(Tilebuf *tb)
{
   rect_list_clear(&tb->rects);
   rect_list_node_pool_flush();
   free(tb);
}

EVAS_API void
evas_common_tilebuf_set_tile_size(Tilebuf *tb, int tw, int th)
{
   tb->tile_size.w = tw;
   tb->tile_size.h = th;
}

EVAS_API void
evas_common_tilebuf_get_tile_size(Tilebuf *tb, int *tw, int *th)
{
   if (tw) *tw = tb->tile_size.w;
   if (th) *th = tb->tile_size.h;
}

EVAS_API void
evas_common_tilebuf_tile_strict_set(Tilebuf *tb, Eina_Bool strict)
{
   tb->strict_tiles = strict;
}

EVAS_API int
evas_common_tilebuf_add_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
   if ((w <= 0) || (h <= 0)) return 0;
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, tb->outbuf_w, tb->outbuf_h);
   if ((w <= 0) || (h <= 0)) return 0;
   // optimize a common case -> adding the exact same rect 2x in a row
   if ((tb->prev_add.x == x) && (tb->prev_add.y == y) &&
       (tb->prev_add.w == w) && (tb->prev_add.h == h)) return 1;
   tb->prev_add.x = x; tb->prev_add.y = y;
   tb->prev_add.w = w; tb->prev_add.h = h;
   tb->prev_del.w = 0; tb->prev_del.h = 0;
   return _add_redraw(&tb->rects, x, y, w, h, FUZZ * FUZZ);
}

EVAS_API int
evas_common_tilebuf_del_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
   rect_t r;

   if (!tb->rects.head) return 0;
   if ((w <= 0) || (h <= 0)) return 0;
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, tb->outbuf_w, tb->outbuf_h);
   if ((w <= 0) || (h <= 0)) return 0;
   // optimize a common case -> deleting the exact same rect 2x in a row
   if ((tb->prev_del.x == x) && (tb->prev_del.y == y) &&
       (tb->prev_del.w == w) && (tb->prev_del.h == h)) return 1;
   tb->prev_del.x = x; tb->prev_del.y = y;
   tb->prev_del.w = w; tb->prev_del.h = h;
   tb->prev_add.w = 0; tb->prev_add.h = 0;
   rect_init(&r, x, y, w, h);
   rect_list_del_split_strict(&tb->rects, r);
   tb->need_merge = 1;
   return 0;
}

EVAS_API int
evas_common_tilebuf_add_motion_vector(Tilebuf *tb EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, int dx EINA_UNUSED, int dy EINA_UNUSED, int alpha EINA_UNUSED)
{
   return 0;
}

EVAS_API void
evas_common_tilebuf_clear(Tilebuf *tb)
{
   tb->prev_add.x = tb->prev_add.y = tb->prev_add.w = tb->prev_add.h = 0;
   tb->prev_del.x = tb->prev_del.y = tb->prev_del.w = tb->prev_del.h = 0;
   rect_list_clear(&tb->rects);
   tb->need_merge = 0;
}

EVAS_API Tilebuf_Rect *
evas_common_tilebuf_get_render_rects(Tilebuf *tb)
{
   list_node_t *n;
   list_t to_merge;
   Tilebuf_Rect *rects = NULL, *rbuf, *r;
   int bx1 = 0, bx2 = 0, by1 = 0, by2 = 0, num = 0, x1, x2, y1, y2, i;

/* don't need this since the below is now always on
   if (tb->need_merge)
     {
        to_merge = tb->rects;
        tb->rects = list_zeroed;
        rect_list_merge_rects(&tb->rects, &to_merge, 0);
        tb->need_merge = 0;
     }
 */
   if (1)
// always fuzz merge for optimal perf
//   if (!tb->strict_tiles)
     {
        // round up rects to tb->tile_size.w and tb->tile_size.h
        to_merge = list_zeroed;
        for (n = tb->rects.head; n; n = n->next)
          {
             x1 = ((rect_node_t *)n)->rect.left;
             x2 = x1 + ((rect_node_t *)n)->rect.width;
             y1 = ((rect_node_t *)n)->rect.top;
             y2 = y1 + ((rect_node_t *)n)->rect.height;
             x1 = tb->tile_size.w * (x1 / tb->tile_size.w);
             y1 = tb->tile_size.h * (y1 / tb->tile_size.h);
             x2 = tb->tile_size.w * ((x2 + tb->tile_size.w - 1) / tb->tile_size.w);
             y2 = tb->tile_size.h * ((y2 + tb->tile_size.h - 1) / tb->tile_size.h);
             _add_redraw(&to_merge, x1, y1, x2 - x1, y2 - y1, 0);
          }
        rect_list_clear(&tb->rects);
        rect_list_merge_rects(&tb->rects, &to_merge, 0);
     }
   n = tb->rects.head;
   if (n)
     {
        RECTS_CLIP_TO_RECT(((rect_node_t *)n)->rect.left,
                           ((rect_node_t *)n)->rect.top,
                           ((rect_node_t *)n)->rect.width,
                           ((rect_node_t *)n)->rect.height,
                           0, 0, tb->outbuf_w, tb->outbuf_h);
        num = 1;
        bx1 = ((rect_node_t *)n)->rect.left;
        bx2 = bx1 + ((rect_node_t *)n)->rect.width;
        by1 = ((rect_node_t *)n)->rect.top;
        by2 = by1 + ((rect_node_t *)n)->rect.height;
        n = n->next;
        for (; n; n = n->next)
          {
             RECTS_CLIP_TO_RECT(((rect_node_t *)n)->rect.left,
                                ((rect_node_t *)n)->rect.top,
                                ((rect_node_t *)n)->rect.width,
                                ((rect_node_t *)n)->rect.height,
                                0, 0, tb->outbuf_w, tb->outbuf_h);
             x1 = ((rect_node_t *)n)->rect.left;
             if (x1 < bx1) bx1 = x1;
             x2 = x1 + ((rect_node_t *)n)->rect.width;
             if (x2 > bx2) bx2 = x2;

             y1 = ((rect_node_t *)n)->rect.top;
             if (y1 < by1) by1 = y1;
             y2 = y1 + ((rect_node_t *)n)->rect.height;
             if (y2 > by2) by2 = y2;
             num++;
          }
     }
   else
     return NULL;

   /* magic number - if we have > MAXREG regions to update, take bounding */
   if (num > MAXREG)
     {
        r = malloc(sizeof(Tilebuf_Rect));
        if (r)
          {
             EINA_INLIST_GET(r)->next = NULL;
             EINA_INLIST_GET(r)->prev = NULL;
             EINA_INLIST_GET(r)->last = NULL;
             r->x = bx1;
             r->y = by1;
             r->w = bx2 - bx1;
             r->h = by2 - by1;
             rects = (Tilebuf_Rect *)
               eina_inlist_append(EINA_INLIST_GET(rects),
                                  EINA_INLIST_GET(r));
          }
        return rects;
     }

   rbuf = malloc(sizeof(Tilebuf_Rect) * num);
   if (!rbuf) return NULL;

   for (i = 0, n = tb->rects.head; n; n = n->next)
     {
        rect_t cur;

        cur = ((rect_node_t *)n)->rect;
        if ((cur.width > 0) && (cur.height > 0))
          {
             r = &(rbuf[i]);
             EINA_INLIST_GET(r)->next = NULL;
             EINA_INLIST_GET(r)->prev = NULL;
             EINA_INLIST_GET(r)->last = NULL;
             r->x = cur.left;
             r->y = cur.top;
             r->w = cur.width;
             r->h = cur.height;
             rects = (Tilebuf_Rect *)
               eina_inlist_append(EINA_INLIST_GET(rects),
                                  EINA_INLIST_GET(r));
             i++;
          }
     }

   // It is possible that due to the clipping we do not return any rectangle here.
   if (!rects) free(rbuf);

   return rects;
}

EVAS_API void
evas_common_tilebuf_free_render_rects(Tilebuf_Rect *rects)
{
   free(rects);
}
#endif
