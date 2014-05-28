#include "evas_common_private.h"
#include "evas_convert_main.h"
#include "evas_private.h"

EAPI Cutout_Rects*
evas_common_draw_context_cutouts_new(void)
{
   Cutout_Rects *rects;

   rects = calloc(1, sizeof(Cutout_Rects));
   return rects;
}

EAPI void
evas_common_draw_context_cutouts_free(Cutout_Rects* rects)
{
   if (!rects) return;
   rects->active = 0;
}

EAPI void
evas_common_draw_context_cutouts_del(Cutout_Rects* rects, int idx)
{
   if ((idx >= 0) && (idx < rects->active))
     {
        Cutout_Rect *rect;

	rect = rects->rects + idx;
        memmove(rect, rect + 1,
		sizeof(Cutout_Rect) * (rects->active - idx - 1));
        rects->active--;
     }
}

void
evas_common_init(void)
{
   evas_common_cpu_init();

   evas_common_blend_init();
   evas_common_image_init();
   evas_common_convert_init();
   evas_common_scale_init();
   evas_common_rectangle_init();
   evas_common_polygon_init();
   evas_common_line_init();
   evas_common_font_init();
   evas_common_draw_init();
   evas_common_tilebuf_init();
}

void
evas_common_shutdown(void)
{
   evas_font_dir_cache_free();
   evas_common_image_cache_free();
}

EAPI void
evas_common_draw_init(void)
{
}

EAPI RGBA_Draw_Context *
evas_common_draw_context_new(void)
{
   RGBA_Draw_Context *dc;

   dc = calloc(1, sizeof(RGBA_Draw_Context));
   dc->sli.h = 1;
   return dc;
}

EAPI void
evas_common_draw_context_free(RGBA_Draw_Context *dc)
{
   if (!dc) return;

#ifdef HAVE_PIXMAN
#if defined(PIXMAN_FONT) || defined(PIXMAN_RECT) || defined(PIXMAN_LINE) || defined(PIXMAN_POLY)
   if (dc->col.pixman_color_image)
     {
        pixman_image_unref(dc->col.pixman_color_image);
        dc->col.pixman_color_image = NULL;
     }
#endif
#endif

   evas_common_draw_context_apply_clean_cutouts(&dc->cutout);
   free(dc);
}

EAPI void
evas_common_draw_context_clear_cutouts(RGBA_Draw_Context *dc)
{
   evas_common_draw_context_cutouts_free(&dc->cutout);
}

EAPI void
evas_common_draw_context_font_ext_set(RGBA_Draw_Context *dc,
				      void *data,
				      void *(*gl_new)  (void *data, RGBA_Font_Glyph *fg),
				      void  (*gl_free) (void *ext_dat),
				      void  (*gl_draw) (void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y))
{
   dc->font_ext.data = data;
   dc->font_ext.func.gl_new = gl_new;
   dc->font_ext.func.gl_free = gl_free;
   dc->font_ext.func.gl_draw = gl_draw;
}

EAPI void
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

EAPI void
evas_common_draw_context_set_clip(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   dc->clip.use = 1;
   dc->clip.x = x;
   dc->clip.y = y;
   dc->clip.w = w;
   dc->clip.h = h;
}

EAPI void
evas_common_draw_context_unset_clip(RGBA_Draw_Context *dc)
{
   dc->clip.use = 0;
}

EAPI void
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

EAPI void
evas_common_draw_context_set_multiplier(RGBA_Draw_Context *dc, int r, int g, int b, int a)
{
   dc->mul.use = 1;
   R_VAL(&(dc->mul.col)) = (DATA8)r;
   G_VAL(&(dc->mul.col)) = (DATA8)g;
   B_VAL(&(dc->mul.col)) = (DATA8)b;
   A_VAL(&(dc->mul.col)) = (DATA8)a;
}

EAPI void
evas_common_draw_context_unset_multiplier(RGBA_Draw_Context *dc)
{
   dc->mul.use = 0;
}


EAPI void
evas_common_draw_context_add_cutout(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
//   if (dc->cutout.rects > 512) return;
   if (dc->clip.use)
     {
#if 1 // this is a bit faster
        int xa1, xa2, xb1, xb2;

        xa1 = x;
        xa2 = xa1 + w - 1;
        xb1 = dc->clip.x;
        if (xa2 < xb1) return;
        xb2 = xb1 + dc->clip.w - 1;
        if (xa1 >= xb2) return;
        if (xa2 > xb2) xa2 = xb2;
        if (xb1 > xa1) xa1 = xb1;
        x = xa1;
        w = xa2 - xa1 + 1;

        xa1 = y;
        xa2 = xa1 + h - 1;
        xb1 = dc->clip.y;
        if (xa2 < xb1) return;
        xb2 = xb1 + dc->clip.h - 1;
        if (xa1 >= xb2) return;
        if (xa2 > xb2) xa2 = xb2;
        if (xb1 > xa1) xa1 = xb1;
        y = xa1;
        h = xa2 - xa1 + 1;
#else
        RECTS_CLIP_TO_RECT(x, y, w, h,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
#endif
	if ((w < 1) || (h < 1)) return;
     }
   evas_common_draw_context_cutouts_add(&dc->cutout, x, y, w, h);
}

int
evas_common_draw_context_cutout_split(Cutout_Rects* res, int idx, Cutout_Rect *split)
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

EAPI Cutout_Rects*
evas_common_draw_context_apply_cutouts(RGBA_Draw_Context *dc, Cutout_Rects *reuse)
{
   Cutout_Rects*        res = NULL;
   int                  i;
   int                  j;

   if (!dc->clip.use) return NULL;
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0)) return NULL;


   if (!reuse)
     {
        res = evas_common_draw_context_cutouts_new();
     }
   else
     {
        evas_common_draw_context_cutouts_free(reuse);
        res = reuse;
     }
   evas_common_draw_context_cutouts_add(res, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);

   for (i = 0; i < dc->cutout.active; ++i)
     {
        /* Don't loop on the element just added to the list as they are already correctly clipped. */
        int active = res->active;

        for (j = 0; j < active; )
          {
             if (evas_common_draw_context_cutout_split(res, j, dc->cutout.rects + i))
               ++j;
             else
               active--;
          }
     }
   /* merge rects */
#define RI res->rects[i]
#define RJ res->rects[j]
   if (res->active > 1)
     {
        int found = 1;
        
        while (found)
          {
             found = 0;
             for (i = 0; i < res->active; i++)
               {
                  for (j = i + 1; j < res->active; j++)
                    {
                       /* skip empty rects we are removing */
                       if (RJ.w == 0) continue;
                       /* check if its same width, immediately above or below */
                       if ((RJ.w == RI.w) && (RJ.x == RI.x))
                         {
                            if ((RJ.y + RJ.h) == RI.y) /* above */
                              {
                                 RI.y = RJ.y;
                                 RI.h += RJ.h;
                                 RJ.w = 0;
                                 found = 1;
                              }
                            else if ((RI.y + RI.h) == RJ.y) /* below */
                              {
                                 RI.h += RJ.h;
                                 RJ.w = 0;
                                 found = 1;
                              }
                         }
                       /* check if its same height, immediately left or right */
                       else if ((RJ.h == RI.h) && (RJ.y == RI.y))
                         {
                            if ((RJ.x + RJ.w) == RI.x) /* left */
                              {
                                 RI.x = RJ.x;
                                 RI.w += RJ.w;
                                 RJ.w = 0;
                                 found = 1;
                              }
                            else if ((RI.x + RI.w) == RJ.x) /* right */
                              {
                                 RI.w += RJ.w;
                                 RJ.w = 0;
                                 found = 1;
                              }
                         }
                    }
               }
          }

        /* Repack the cutout */
        j = 0;
        for (i = 0; i < res->active; i++)
          {
             if (RI.w == 0) continue;
             if (i != j)
               RJ = RI;
             j++;
          }
        res->active = j;
        return res;
     }
   return res;
}

EAPI void
evas_common_draw_context_apply_clear_cutouts(Cutout_Rects* rects)
{
   evas_common_draw_context_apply_clean_cutouts(rects);
   free(rects);
}

EAPI void
evas_common_draw_context_apply_clean_cutouts(Cutout_Rects* rects)
{
   free(rects->rects);
   rects->rects = NULL;
   rects->active = 0;
   rects->max = 0;
}

EAPI void
evas_common_draw_context_set_anti_alias(RGBA_Draw_Context *dc , unsigned char aa)
{
   dc->anti_alias = !!aa;
}

EAPI void
evas_common_draw_context_set_color_interpolation(RGBA_Draw_Context *dc, int color_space)
{
   dc->interpolation.color_space = color_space;
}

EAPI void
evas_common_draw_context_set_render_op(RGBA_Draw_Context *dc , int op)
{
   dc->render_op = op;
}

EAPI void
evas_common_draw_context_set_sli(RGBA_Draw_Context *dc, int y, int h)
{
   dc->sli.y = y;
   dc->sli.h = h;
}
