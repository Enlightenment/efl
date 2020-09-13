#include "evas_common_private.h"
#include "evas_convert_main.h"
#include "evas_private.h"

EVAS_API Cutout_Rects *
evas_common_draw_context_cutouts_new(void)
{
   Cutout_Rects *rects;

   rects = calloc(1, sizeof(Cutout_Rects));
   return rects;
}

static void
evas_common_draw_context_cutouts_dup(Cutout_Rects *rects2, const Cutout_Rects *rects)
{
   if (!rects) return;
   rects2->active = rects->active;
   rects2->max = rects->active;
   rects2->last_add = rects->last_add;
   if (rects2->max > 0)
     {
        const size_t sz = sizeof(Cutout_Rect) * rects2->max;
        rects2->rects = malloc(sz);
        memcpy(rects2->rects, rects->rects, sz);
        return;
     }
   else rects2->rects = NULL;
}

EVAS_API void
evas_common_draw_context_cutouts_free(Cutout_Rects* rects)
{
   if (!rects) return;
   rects->active = 0;
   rects->last_add.w = 0;
}

EVAS_API void
evas_common_draw_context_cutouts_real_free(Cutout_Rects* rects)
{
   if (!rects) return;
   free(rects->rects);
   free(rects);
}

EVAS_API void
evas_common_draw_context_cutouts_del(Cutout_Rects* rects, int idx)
{
   if ((idx >= 0) && (idx < rects->active))
     {
        Cutout_Rect *rect;

        rect = rects->rects + idx;
        memmove(rect, rect + 1,
                sizeof(Cutout_Rect) * (rects->active - idx - 1));
        rects->active--;
        rects->last_add.w = 0;
     }
}

static int _init_count = 0;
static Eina_Trash *_ctxt_spares = NULL;
static int _ctxt_spares_count = 0;
static SLK(_ctx_spares_lock);

static void
_evas_common_draw_context_real_free(RGBA_Draw_Context *dc)
{
#ifdef HAVE_PIXMAN
# if defined(PIXMAN_FONT) || defined(PIXMAN_RECT) || defined(PIXMAN_LINE) || defined(PIXMAN_POLY)
   if (dc->col.pixman_color_image)
     pixman_image_unref(dc->col.pixman_color_image);
# endif
#endif
   evas_common_draw_context_apply_clean_cutouts(&dc->cutout);
   evas_common_draw_context_cutouts_real_free(dc->cache.rects);
   free(dc);
}

static void
_evas_common_draw_context_stash(RGBA_Draw_Context *dc)
{
   if (_ctxt_spares_count >= 8)
     {
        _evas_common_draw_context_real_free(dc);
        return ;
     }

#ifdef HAVE_PIXMAN
# if defined(PIXMAN_FONT) || defined(PIXMAN_RECT) || defined(PIXMAN_LINE) || defined(PIXMAN_POLY)
   if (dc->col.pixman_color_image)
     {
        pixman_image_unref(dc->col.pixman_color_image);
        dc->col.pixman_color_image = NULL;
     }
# endif
#endif
   evas_common_draw_context_apply_clean_cutouts(&dc->cutout);
   evas_common_draw_context_cutouts_real_free(dc->cache.rects);
   SLKL(_ctx_spares_lock);
   eina_trash_push(&_ctxt_spares, dc);
   _ctxt_spares_count++;
   SLKU(_ctx_spares_lock);
}

static RGBA_Draw_Context *
_evas_common_draw_context_find(void)
{
   RGBA_Draw_Context *dc = NULL;

   if (_ctxt_spares)
     {
        SLKL(_ctx_spares_lock);
        dc = eina_trash_pop(&_ctxt_spares);
        _ctxt_spares_count--;
        SLKU(_ctx_spares_lock);
     }

   if (!dc) dc = malloc(sizeof(RGBA_Draw_Context));

   return dc;
}

EVAS_API void
evas_common_init(void)
{
   if (_init_count++) return;

   SLKI(_ctx_spares_lock);
   evas_common_cpu_init();

   evas_common_blend_init();
   evas_common_image_init();
   evas_common_convert_init();
   evas_common_scale_init();
   evas_common_scale_sample_init();
   evas_common_rectangle_init();
   evas_common_polygon_init();
   evas_common_line_init();
   evas_common_font_init();
   evas_common_draw_init();
   evas_common_tilebuf_init();
}

EVAS_API void
evas_common_shutdown(void)
{
   if (--_init_count) return;

   evas_font_dir_cache_free();
   evas_common_font_shutdown();
   evas_common_image_shutdown();
   evas_common_image_cache_free();
   evas_common_scale_sample_shutdown();
// just in case any thread is still doing things... don't del this here
//   RGBA_Draw_Context *dc;
//   SLKL(_ctx_spares_lock);
//   EINA_LIST_FREE(_ctxt_spares, dc) _evas_common_draw_context_real_free(dc);
//   _ctxt_spares_count = 0;
//   SLKU(_ctx_spares_lock);
//   SLKD(_ctx_spares_lock);
}

EVAS_API void
evas_common_draw_init(void)
{
}

EVAS_API RGBA_Draw_Context *
evas_common_draw_context_new(void)
{
   RGBA_Draw_Context *dc;
   dc = _evas_common_draw_context_find();
   if (!dc) return NULL;
   memset(dc, 0, sizeof(RGBA_Draw_Context));
   return dc;
}

EVAS_API RGBA_Draw_Context *
evas_common_draw_context_dup(RGBA_Draw_Context *dc)
{
   RGBA_Draw_Context *dc2 = _evas_common_draw_context_find();

   if (!dc) return dc2;
   memcpy(dc2, dc, sizeof(RGBA_Draw_Context));
   evas_common_draw_context_cutouts_dup(&dc2->cutout, &dc->cutout);
#ifdef HAVE_PIXMAN
# if defined(PIXMAN_FONT) || defined(PIXMAN_RECT) || defined(PIXMAN_LINE) || defined(PIXMAN_POLY)
   dc2->col.pixman_color_image = NULL;
# endif
#endif
   dc2->cache.rects = NULL;
   dc2->cache.used = 0;
   return dc2;
}

EVAS_API void
evas_common_draw_context_free(RGBA_Draw_Context *dc)
{
   if (!dc) return;
   _evas_common_draw_context_stash(dc);
}

EVAS_API void
evas_common_draw_context_clear_cutouts(RGBA_Draw_Context *dc)
{
   evas_common_draw_context_cutouts_free(&dc->cutout);
}

EVAS_API void
evas_common_draw_context_font_ext_set(RGBA_Draw_Context *dc,
                                      void *data,
                                      void *(*gl_new)  (void *data, RGBA_Font_Glyph *fg),
                                      void  (*gl_free) (void *ext_dat),
                                      void  (*gl_draw) (void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y, int w, int h),
                                      void *(*gl_image_new) (void *gc, RGBA_Font_Glyph *fg, int alpha, Evas_Colorspace cspace),
                                      void  (*gl_image_free) (void *image),
                                      void  (*gl_image_draw) (void *gc, void *im, int dx, int dy, int dw, int dh, int smooth))
{
   dc->font_ext.data = data;
   dc->font_ext.func.gl_new = gl_new;
   dc->font_ext.func.gl_free = gl_free;
   dc->font_ext.func.gl_draw = gl_draw;
   dc->font_ext.func.gl_image_new = gl_image_new;
   dc->font_ext.func.gl_image_free = gl_image_free;
   dc->font_ext.func.gl_image_draw = gl_image_draw;
}

EVAS_API void
evas_common_draw_context_clip_clip(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   if (dc->clip.use)
     {
	RECTS_CLIP_TO_RECT(dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h,
			   x, y, w, h);
     }
   else
     evas_common_draw_context_set_clip(dc, x, y, w, h);
}

EVAS_API void
evas_common_draw_context_set_clip(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   dc->clip.use = 1;
   dc->clip.x = x;
   dc->clip.y = y;
   dc->clip.w = w;
   dc->clip.h = h;
}

EVAS_API void
evas_common_draw_context_unset_clip(RGBA_Draw_Context *dc)
{
   dc->clip.use = 0;
}

EVAS_API void
evas_common_draw_context_set_color(RGBA_Draw_Context *dc, int r, int g, int b, int a)
{
   R_VAL(&(dc->col.col)) = (DATA8)r;
   G_VAL(&(dc->col.col)) = (DATA8)g;
   B_VAL(&(dc->col.col)) = (DATA8)b;
   A_VAL(&(dc->col.col)) = (DATA8)a;
#ifdef HAVE_PIXMAN
#if defined(PIXMAN_FONT) || defined(PIXMAN_RECT) || defined(PIXMAN_LINE) || defined(PIXMAN_POLY)
   if (dc->col.pixman_color_image)
     pixman_image_unref(dc->col.pixman_color_image);

   pixman_color_t pixman_color;

   pixman_color.alpha =  (dc->col.col & 0xff000000) >> 16;
   pixman_color.red = (dc->col.col & 0x00ff0000) >> 8;
   pixman_color.green = (dc->col.col & 0x0000ff00);
   pixman_color.blue = (dc->col.col & 0x000000ff) << 8;

   dc->col.pixman_color_image = pixman_image_create_solid_fill(&pixman_color);
#endif
#endif

}

EVAS_API void
evas_common_draw_context_set_multiplier(RGBA_Draw_Context *dc, int r, int g, int b, int a)
{
   dc->mul.use = 1;
   R_VAL(&(dc->mul.col)) = (DATA8)r;
   G_VAL(&(dc->mul.col)) = (DATA8)g;
   B_VAL(&(dc->mul.col)) = (DATA8)b;
   A_VAL(&(dc->mul.col)) = (DATA8)a;
}

EVAS_API void
evas_common_draw_context_unset_multiplier(RGBA_Draw_Context *dc)
{
   dc->mul.use = 0;
}


EVAS_API void
evas_common_draw_context_add_cutout(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
//   if (dc->cutout.rects > 512) return;
   if (dc->clip.use)
     {
#if 1 // this is a bit faster
        int x1, x2, y1, y2;
        int cx1, cx2, cy1, cy2;

        x2 = x + w;
        cx1 = dc->clip.x;
        if (x2 <= cx1) return;
        x1 = x;
        cx2 = cx1 + dc->clip.w;
        if (x1 >= cx2) return;

        if (x1 < cx1) x1 = cx1;
        if (x2 > cx2) x2 = cx2;

        y2 = y + h;
        cy1 = dc->clip.y;
        if (y2 <= cy1) return;
        y1 = y;
        cy2 = cy1 + dc->clip.h;
        if (y1 >= cy2) return;

        if (y1 < cy1) y1 = cy1;
        if (y2 > cy2) y2 = cy2;

        x = x1;
        y = y1;
        w = x2 - x1;
        h = y2 - y1;
#else
        RECTS_CLIP_TO_RECT(x, y, w, h,
                           dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
#endif
     }
   if ((w * h) <= (8 * 8)) return;
   if (dc->cutout.last_add.w > 0)
     {
        if ((dc->cutout.last_add.x == x) && (dc->cutout.last_add.y == y) &&
            (dc->cutout.last_add.w == w) && (dc->cutout.last_add.h == h)) return;
     }
   dc->cutout.last_add.x = x;
   dc->cutout.last_add.y = y;
   dc->cutout.last_add.w = w;
   dc->cutout.last_add.h = h;
   evas_common_draw_context_cutouts_add(&dc->cutout, x, y, w, h);
}

static int
evas_common_draw_context_cutout_split(Cutout_Rects *res, int idx, Cutout_Rect *split)
{
   /* 1 input rect, multiple out */
   Cutout_Rect in = res->rects[idx];

   /* this is to save me a LOT of typing */
#define INX1 (in.x)
#define INX2 (in.x + in.w)
#define SPX1 (split->x)
#define SPX2 (split->x + split->w)
#define INY1 (in.y)
#define INY2 (in.y + in.h)
#define SPY1 (split->y)
#define SPY2 (split->y + split->h)
#define X1_IN (in.x < split->x)
#define X2_IN ((in.x + in.w) > (split->x + split->w))
#define Y1_IN (in.y < split->y)
#define Y2_IN ((in.y + in.h) > (split->y + split->h))
#define R_NEW(_r, _x, _y, _w, _h) { evas_common_draw_context_cutouts_add(_r, _x, _y, _w, _h); }
   if (!RECTS_INTERSECT(in.x, in.y, in.w, in.h,
			split->x, split->y, split->w, split->h))
     {
        /* No colision => no clipping, don't touch it. */
	return 1;
     }

   /* S    = split (ie cut out rect) */
   /* +--+ = in (rect to be cut) */

   /*
    *  +---+
    *  |   |
    *  | S |
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && X2_IN && Y1_IN && Y2_IN)
     {
        R_NEW(res, in.x, in.y, in.w, SPY1 - in.y);
	R_NEW(res, in.x, SPY1, SPX1 - in.x, SPY2 - SPY1);
	R_NEW(res, SPX2, SPY1, INX2 - SPX2, SPY2 - SPY1);
        /* out => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[idx].h = INY2 - SPY2;
        res->rects[idx].y = SPY2;
	return 1;
     }
   /* SSSSSSS
    * S+---+S
    * S|SSS|S
    * S|SSS|S
    * S|SSS|S
    * S+---+S
    * SSSSSSS
    */
   if (!X1_IN && !X2_IN && !Y1_IN && !Y2_IN)
     {
        evas_common_draw_context_cutouts_del(res, idx);
	return 0;
     }
   /* SSS
    * S+---+
    * S|S  |
    * S|S  |
    * S|S  |
    * S+---+
    * SSS
    */
   if (!X1_IN && X2_IN && !Y1_IN && !Y2_IN)
     {
        /* in => (SPX2, in.y, INX2 - SPX2, in.h) */
        res->rects[idx].w = INX2 - SPX2;
        res->rects[idx].x = SPX2;
	return 1;
     }
   /*    S
    *  +---+
    *  | S |
    *  | S |
    *  | S |
    *  +---+
    *    S
    */
   if (X1_IN && X2_IN && !Y1_IN && !Y2_IN)
     {
        R_NEW(res, in.x, in.y, SPX1 - in.x, in.h);
        /* in => (SPX2, in.y, INX2 - SPX2, in.h) */
        res->rects[idx].w = INX2 - SPX2;
        res->rects[idx].x = SPX2;
	return 1;
     }
   /*     SSS
    *  +---+S
    *  |  S|S
    *  |  S|S
    *  |  S|S
    *  +---+S
    *     SSS
    */
   if (X1_IN && !X2_IN && !Y1_IN && !Y2_IN)
     {
        /* in => (in.x, in.y, SPX1 - in.x, in.h) */
        res->rects[idx].w = SPX1 - in.x;
	return 1;
     }
   /* SSSSSSS
    * S+---+S
    * S|SSS|S
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && !X2_IN && !Y1_IN && Y2_IN)
     {
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[idx].h = INY2 - SPY2;
        res->rects[idx].y = SPY2;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    * S|SSS|S
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && !X2_IN && Y1_IN && Y2_IN)
     {
        R_NEW(res, in.x, SPY2, in.w, INY2 - SPY2);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[idx].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    * S|SSS|S
    * S+---+S
    * SSSSSSS
    */
   if (!X1_IN && !X2_IN && Y1_IN && !Y2_IN)
     {
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[idx].h = SPY1 - in.y;
	return 1;
     }
   /* SSS
    * S+---+
    * S|S  |
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(res, SPX2, in.y, INX2 - SPX2, SPY2 - in.y);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[idx].h = INY2 - SPY2;
        res->rects[idx].y = SPY2;
	return 1;
     }
   /*    S
    *  +---+
    *  | S |
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, in.y, SPX1 - in.x, SPY2 - in.y);
	R_NEW(res, SPX2, in.y, INX2 - SPX2, SPY2 - in.y);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[idx].h = INY2 - SPY2;
        res->rects[idx].y = SPY2;
	return 1;
     }
   /*     SSS
    *  +---+S
    *  |  S|S
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && !X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, in.y, SPX1 - in.x, SPY2 - in.y);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[idx].h = INY2 - SPY2;
        res->rects[idx].y = SPY2;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    * S|S  |
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, SPY2, in.w, INY2 - SPY2);
	R_NEW(res, SPX2, SPY1, INX2 - SPX2, SPY2 - SPY1);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[idx].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |  S|S
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && !X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, SPY2, in.w, INY2 - SPY2);
	R_NEW(res, in.x, SPY1, SPX1 - in.x, SPY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[idx].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    * S|S  |
    * S+---+
    * SSS
    */
   if (!X1_IN && X2_IN && Y1_IN && !Y2_IN)
     {
        R_NEW(res, SPX2, SPY1, INX2 - SPX2, INY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[idx].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    *  | S |
    *  +---+
    *    S
    */
   if (X1_IN && X2_IN && Y1_IN && !Y2_IN)
     {
	R_NEW(res, in.x, SPY1, SPX1 - in.x, INY2 - SPY1);
        R_NEW(res, SPX2, SPY1, INX2 - SPX2, INY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[idx].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    *  |  S|S
    *  +---+S
    *     SSS
    */
   if (X1_IN && !X2_IN && Y1_IN && !Y2_IN)
     {
        R_NEW(res, in.x, SPY1, SPX1 - in.x, INY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[idx].h = SPY1 - in.y;
	return 1;
     }
   evas_common_draw_context_cutouts_del(res, idx);
   return 0;
#undef INX1
#undef INX2
#undef SPX1
#undef SPX2
#undef INY1
#undef INY2
#undef SPY1
#undef SPY2
#undef X1_IN
#undef X2_IN
#undef Y1_IN
#undef Y2_IN
#undef R_NEW
}

EVAS_API void
evas_common_draw_context_target_set(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   dc->cutout_target.x = x;
   dc->cutout_target.y = y;
   dc->cutout_target.w = w;
   dc->cutout_target.h = h;
}

static int
_srt_y(const void *d1, const void *d2)
{
   const Cutout_Rect *r1 = d1, *r2 = d2;
   if (r1->y == r2->y) return r1->x - r2->x;
   return r1->y - r2->y;
}

static int
_srt_x(const void *d1, const void *d2)
{
   const Cutout_Rect *r1 = d1, *r2 = d2;
   if (r1->x == r2->x) return r1->y - r2->y;
   return r1->x - r2->x;
}

EVAS_API Cutout_Rects *
evas_common_draw_context_apply_cutouts(RGBA_Draw_Context *dc, Cutout_Rects *reuse)
{
   Cutout_Rects        *res = NULL;
   int                  i, j, active, found = 0;

   if (!dc->clip.use) return NULL;
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0)) return NULL;

   if (!reuse) res = evas_common_draw_context_cutouts_new();
   else
     {
        evas_common_draw_context_cutouts_free(reuse);
        res = reuse;
     }
   // this avoids a nasty case of O(n^2)/2 below with lots of rectangles
   // to merge so only do this merging if the number of rects is small enough
   // not to blow out into insanity
   evas_common_draw_context_cutouts_add(res, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
   for (i = 0; i < dc->cutout.active; i++)
     {
        if ((dc->cutout_target.w != 0) &&
            (!RECTS_INTERSECT(dc->cutout.rects[i].x, dc->cutout.rects[i].y,
                              dc->cutout.rects[i].w, dc->cutout.rects[i].h,
                              dc->cutout_target.x, dc->cutout_target.y,
                              dc->cutout_target.w, dc->cutout_target.h)))
          continue;
        // Don't loop on the element just added to the list as they are
        // already correctly clipped.
        active = res->active;
        for (j = 0; j < active; )
          {
             if (evas_common_draw_context_cutout_split
                 (res, j, dc->cutout.rects + i)) j++;
             else active--;
          }
     }
   /* merge rects */
#define RI res->rects[i]
#define RJ res->rects[j]
   if (res->active > 1)
     {
        if (res->active > 5)
          {
             // fast path for larger numbers of rects to merge by using
             // qsort to sort by y and x to limit the number of rects
             // we have to walk as rects that have a different y cannot
             // be merged anyway (or x).
             qsort(res->rects, res->active, sizeof(res->rects[0]), _srt_y);
             for (i = 0; i < res->active; i++)
               {
                  if (RI.w == 0) continue; // skip empty rect
                  for (j = i + 1; j < res->active; j++)
                    {
                       if (RJ.y != RI.y) break; // new line, sorted thus skip
                       if (RJ.w == 0) continue; // skip empty rect
                       // if J is the same height (could be merged)
                       if (RJ.h == RI.h)
                         {
                            // if J is immediately to the right of I
                            if (RJ.x == (RI.x + RI.w))
                              {
                                 RI.w = (RJ.x + RJ.w) - RI.x; // expand RI
                                 RJ.w = 0; // invalidate
                                 found++;
                              }
                            // since we sort y and THEN x, if height matches
                            // but it's not immediately adjacent, no more
                            // rects exists that can be merged
                            else break;
                         }
                    }
               }
             qsort(res->rects, res->active, sizeof(res->rects[0]), _srt_x);
             for (i = 0; i < res->active; i++)
               {
                  if (RI.w == 0) continue; // skip empty rect
                  for (j = i + 1; j < res->active; j++)
                    {
                       if (RJ.x != RI.x) break; // new line, sorted thus skip
                       if (RJ.w == 0) continue; // skip empty rect
                       // if J is the same height (could be merged)
                       if (RJ.w == RI.w)
                         {
                            // if J is immediately to the right of I
                            if (RJ.y == (RI.y + RI.h))
                              {
                                 RI.h = (RJ.y + RJ.h) - RI.y; // expand RI
                                 RJ.w = 0; // invalidate
                                 found++;
                              }
                            // since we sort y and THEN x, if height matches
                            // but it's not immediately adjacent, no more
                            // rects exists that can be merged
                            else break;
                         }
                    }
               }
          }
        else
          {
             // for a small number of rects, keep things simple as the count
             // is small and big-o complexity isnt a problem yet
             found = 1;
             while (found)
               {
                  found = 0;
                  for (i = 0; i < res->active; i++)
                    {
                       for (j = i + 1; j < res->active; j++)
                         {
                            // skip empty rects we are removing
                            if (RJ.w == 0) continue;
                            // check if its same width, immediately above or below
                            if ((RJ.w == RI.w) && (RJ.x == RI.x))
                              {
                                 if ((RJ.y + RJ.h) == RI.y) // above
                                   {
                                      RI.y = RJ.y;
                                      RI.h += RJ.h;
                                      RJ.w = 0;
                                      found++;
                                   }
                                 else if ((RI.y + RI.h) == RJ.y) // below
                                   {
                                      RI.h += RJ.h;
                                      RJ.w = 0;
                                      found++;
                                   }
                              }
                            // check if its same height, immediately left or right
                            else if ((RJ.h == RI.h) && (RJ.y == RI.y))
                              {
                                 if ((RJ.x + RJ.w) == RI.x) // left
                                   {
                                      RI.x = RJ.x;
                                      RI.w += RJ.w;
                                      RJ.w = 0;
                                      found++;
                                   }
                                 else if ((RI.x + RI.w) == RJ.x) // right
                                   {
                                      RI.w += RJ.w;
                                      RJ.w = 0;
                                      found++;
                                   }
                              }
                         }
                    }
               }
          }

        // Repack the cutout
        j = 0;
        for (i = 0; i < res->active; i++)
          {
             if (RI.w == 0) continue;
             if (i != j) RJ = RI;
             j++;
          }
        res->active = j;
     }
   return res;
}

EVAS_API void
evas_common_draw_context_apply_clear_cutouts(Cutout_Rects *rects)
{
   evas_common_draw_context_apply_clean_cutouts(rects);
   free(rects);
}

EVAS_API void
evas_common_draw_context_apply_clean_cutouts(Cutout_Rects *rects)
{
   free(rects->rects);
   rects->rects = NULL;
   rects->active = 0;
   rects->max = 0;
   rects->last_add.w = 0;
}

EVAS_API void
evas_common_draw_context_set_anti_alias(RGBA_Draw_Context *dc , unsigned char aa)
{
   dc->anti_alias = !!aa;
}

EVAS_API void
evas_common_draw_context_set_color_interpolation(RGBA_Draw_Context *dc, int color_space)
{
   dc->interpolation.color_space = color_space;
}

EVAS_API void
evas_common_draw_context_set_render_op(RGBA_Draw_Context *dc , int op)
{
   dc->render_op = op;
}
