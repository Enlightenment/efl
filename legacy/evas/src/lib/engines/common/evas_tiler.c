#include "evas_common.h"

#define TILE(tb, x, y) ((tb)->tiles.tiles[((y) * (tb)->tiles.w) + (x)])

#ifdef RECTUPDATE
#else
static int  tilebuf_x_intersect(Tilebuf *tb, int x, int w, int *x1, int *x2, int *x1_fill, int *x2_fill);
static int  tilebuf_y_intersect(Tilebuf *tb, int y, int h, int *y1, int *y2, int *y1_fill, int *y2_fill);
static int  tilebuf_intersect(int tsize, int tlen, int tnum, int x, int w, int *x1, int *x2, int *x1_fill, int *x2_fill);
#endif
static void tilebuf_setup(Tilebuf *tb);

void
evas_common_tilebuf_init(void)
{
}

Tilebuf *
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

void
evas_common_tilebuf_free(Tilebuf *tb)
{
#ifdef RECTUPDATE
   evas_common_regionbuf_free(tb->rb);
#else
   if (tb->tiles.tiles) free(tb->tiles.tiles);
#endif
   free(tb);
}

void
evas_common_tilebuf_set_tile_size(Tilebuf *tb, int tw, int th)
{
   tb->tile_size.w = tw;
   tb->tile_size.h = th;
   tilebuf_setup(tb);
}

void
evas_common_tilebuf_get_tile_size(Tilebuf *tb, int *tw, int *th)
{
   if (tw) *tw = tb->tile_size.w;
   if (th) *th = tb->tile_size.h;
}

int
evas_common_tilebuf_add_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
#ifdef RECTUPDATE
   int i;

   for (i = 0; i < h; i++)
     evas_common_regionbuf_span_add(tb->rb, x, x + w - 1, y + i);
   return 1;
#else
   int tx1, tx2, ty1, ty2, tfx1, tfx2, tfy1, tfy2, xx, yy;
   int num;

   num = 0;
   /* wipes out any motion vectors in tiles it touches into redraws */
   if (tilebuf_x_intersect(tb, x, w, &tx1, &tx2, &tfx1, &tfx2) &&
       tilebuf_y_intersect(tb, y, h, &ty1, &ty2, &tfy1, &tfy2))
     {
	for (yy = ty1; yy <= ty2; yy++)
	  {
	     Tilebuf_Tile *tbt;

	     tbt = &(TILE(tb, tx1, yy));
	     for (xx = tx1; xx <= tx2; xx++)
	       {
		  tbt->redraw = 1;
		  tbt++;
	       }
	  }
	num = (tx2 - tx1 + 1) * (ty2 - ty1 + 1);
     }
   return num;
#endif
}

int
evas_common_tilebuf_del_redraw(Tilebuf *tb, int x, int y, int w, int h)
{
#ifdef RECTUPDATE
   int i;

   for (i = 0; i < h; i++)
     evas_common_regionbuf_span_del(tb->rb, x, x + w - 1, y + i);
#else
   int tx1, tx2, ty1, ty2, tfx1, tfx2, tfy1, tfy2, xx, yy;
   int num;

   num = 0;
   /* wipes out any motion vectors in tiles it touches into redraws */
   if (tilebuf_x_intersect(tb, x, w, &tx1, &tx2, &tfx1, &tfx2) &&
       tilebuf_y_intersect(tb, y, h, &ty1, &ty2, &tfy1, &tfy2))
     {
	if (!tfx1) tx1++;
	if (!tfx2) tx2--;
	if (!tfy1) ty1++;
	if (!tfy2) ty2--;
	for (yy = ty1; yy <= ty2; yy++)
	  {
	     Tilebuf_Tile *tbt;

	     tbt = &(TILE(tb, tx1, yy));
	     for (xx = tx1; xx <= tx2; xx++)
	       {
		  tbt->redraw = 0;
		  tbt++;
	       }
	  }
	num = (tx2 - tx1 + 1) * (ty2 - ty1 + 1);
     }
   return num;
#endif
}

int
evas_common_tilebuf_add_motion_vector(Tilebuf *tb, int x, int y, int w, int h, int dx, int dy, int alpha)
{
   /* FIXME: need to actually impliment motion vectors. for now it just */
   /*        implements redraws */
   int num;

   num = evas_common_tilebuf_add_redraw(tb, x, y, w, h);
   num += evas_common_tilebuf_add_redraw(tb, x + dx, y + dy, w, h);
   return num;
}

void
evas_common_tilebuf_clear(Tilebuf *tb)
{
#ifdef RECTUPDATE
   evas_common_regionbuf_clear(tb->rb);
#else
   if (!tb->tiles.tiles) return;
   memset(tb->tiles.tiles, 0, tb->tiles.w * tb->tiles.h * sizeof(Tilebuf_Tile));
#endif
}

Tilebuf_Rect *
evas_common_tilebuf_get_render_rects(Tilebuf *tb)
{
#ifdef RECTUPDATE
   return evas_common_regionbuf_rects_get(tb->rb);
#else
   Tilebuf_Rect *rects = NULL;
   int x, y;

   for (y = 0; y < tb->tiles.h; y++)
     {
	for (x = 0; x < tb->tiles.w; x++)
	  {
	     if (TILE(tb, x, y).redraw)
	       {
		  int can_expand_x = 1, can_expand_y = 1;
		  Tilebuf_Rect *r = NULL;
		  int xx = 0, yy = 0;
		  r = calloc(1, sizeof(Tilebuf_Rect));
/* amalgamate tiles */
#if 1
		  while (can_expand_x)
		    {
		       xx++;
		       if ((x + xx) >= tb->tiles.w)
			 can_expand_x = 0;
		       else if (!(TILE(tb, x + xx, y).redraw))
			 can_expand_x = 0;
		       if (can_expand_x)
			 TILE(tb, x + xx, y).redraw = 0;
		    }
		  while (can_expand_y)
		    {
		       int i;

		       yy++;
		       if ((y + yy) >= tb->tiles.h)
			 can_expand_y = 0;
		       if (can_expand_y)
			 {
			    for (i = x; i < x + xx; i++)
			      {
				 if (!(TILE(tb, i, y + yy).redraw))
				   {
				      can_expand_y = 0;
				      break;
				   }
			      }
			 }
		       if (can_expand_y)
			 {
			    for (i = x; i < x + xx; i++)
			      TILE(tb, i, y + yy).redraw = 0;
			 }
		    }
		  TILE(tb, x, y).redraw = 0;
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
	       }
	  }
     }
   return rects;
#endif
}

void
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
