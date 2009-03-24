/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "evas_common.h"
/* #include "evas_macros.h" */
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"
#include <math.h>

/* As opposed to libXRender, we don't have
 * XDoubleToFixed in XCB :/
 */
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

/* this is a work around broken xrender - when/if this ever gets fixed in xorg
 * we can comment this out and one day remove it - for now keep it until such
 * a fix is spotted in the wild
 */
#define BROKEN_XORG_XRENDER 1

static inline void
set_filter(Xrender_Surface *s, int smooth)
{
   const char *f = smooth ? "best": "nearest";

   xcb_render_set_picture_filter (s->xinf->x11.connection, s->x11.xcb.pic, strlen (f), f, 0, NULL);
}

xcb_render_pictforminfo_t *
xcb_render_find_visual_format (xcb_connection_t *c, xcb_visualtype_t *visual)
{
  xcb_render_query_pict_formats_cookie_t cookie;
  xcb_render_query_pict_formats_reply_t *rep;
  xcb_render_pictscreen_iterator_t       screen_iter;
  xcb_render_pictformat_t                format = { 0 };

  cookie = xcb_render_query_pict_formats (c);
  rep = xcb_render_query_pict_formats_reply (c, cookie, NULL);
  if (!rep)
    return NULL;

  screen_iter = xcb_render_query_pict_formats_screens_iterator (rep);
  for (; screen_iter.rem; xcb_render_pictscreen_next (&screen_iter)) {
    xcb_render_pictdepth_iterator_t depth_iter;

    depth_iter = xcb_render_pictscreen_depths_iterator (screen_iter.data);
    for (; depth_iter.rem; xcb_render_pictdepth_next (&depth_iter)) {
      xcb_render_pictvisual_iterator_t visual_iter;

      visual_iter = xcb_render_pictdepth_visuals_iterator (depth_iter.data);
      for (; visual_iter.rem; xcb_render_pictvisual_next (&visual_iter)) {
        if (visual->visual_id == visual_iter.data->visual) {
          format = visual_iter.data->format;
        }
      }
    }
  }

  if (format != 0)
    {
       xcb_render_pictforminfo_iterator_t forminfo_iter;

       forminfo_iter = xcb_render_query_pict_formats_formats_iterator (rep);
       for (; forminfo_iter.rem; xcb_render_pictforminfo_next (&forminfo_iter)) {
          if (forminfo_iter.data->id == format) {
             xcb_render_pictforminfo_t *forminfo;

             forminfo = (xcb_render_pictforminfo_t *)malloc (sizeof (xcb_render_pictforminfo_t));
             memcpy (forminfo, forminfo_iter.data, sizeof (xcb_render_pictforminfo_t));
             free (rep);

             return forminfo;
          }
       }
    }

  return NULL;
}

Xrender_Surface *
_xr_xcb_render_surface_new(Ximage_Info *xinf, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha)
{
   Xrender_Surface *rs;
   uint32_t            mask;
   uint32_t            values[3];

   if ((!xinf) || (!fmt) || (w < 1) || (h < 1)) return NULL;

   rs = calloc(1, sizeof(Xrender_Surface));
   if (!rs) return NULL;
   rs->xinf = xinf;
   rs->width = w;
   rs->height = h;
   rs->x11.xcb.fmt = (xcb_render_pictforminfo_t *)malloc (sizeof (xcb_render_pictforminfo_t));
   if (!rs->x11.xcb.fmt)
     {
        free(rs);
        return NULL;
     }
   memcpy (rs->x11.xcb.fmt, fmt, sizeof (xcb_render_pictforminfo_t));
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   rs->allocated = 1;
   rs->x11.xcb.draw = xcb_generate_id(xinf->x11.connection);
   xcb_create_pixmap(xinf->x11.connection, fmt->depth, rs->x11.xcb.draw, xinf->x11.root, w, h);
   if (rs->x11.xcb.draw == XCB_NONE)
     {
       free(rs->x11.xcb.fmt);
       free(rs);
       return NULL;
     }
   rs->xinf->references++;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 1;
   values[2] = 0;
   rs->x11.xcb.pic = xcb_generate_id(xinf->x11.connection);
   xcb_render_create_picture(xinf->x11.connection, rs->x11.xcb.pic, rs->x11.xcb.draw, fmt->id, mask, values);
   if (rs->x11.xcb.pic == XCB_NONE)
     {
       xcb_free_pixmap(rs->xinf->x11.connection, rs->x11.xcb.draw);
       rs->xinf->references--;
       free(rs->x11.xcb.fmt);
       free(rs);
       return NULL;
     }

   return rs;
}

Xrender_Surface *
_xr_xcb_render_surface_adopt(Ximage_Info *xinf, xcb_drawable_t draw, int w, int h, int alpha)
{
   Xrender_Surface        *rs;
   uint32_t                   mask;
   uint32_t                   values[3];

   if ((!xinf) || (draw == 0) || (w < 1) || (h < 1)) return NULL;
   rs = calloc(1, sizeof(Xrender_Surface));
   if (!rs) return NULL;
   rs->xinf = xinf;
   rs->width = w;
   rs->height = h;
/*    if (fmt->depth == xinf->fmt32->depth) */
/*      rs->x11.xcb.fmt = xinf->fmt32; */
/*    else if (fmt->depth == xinf->fmt24->depth) */
/*      rs->x11.xcb.fmt = xinf->fmt24; */
/*    else if (fmt->depth == xinf->fmt8->depth) */
/*      rs->x11.xcb.fmt = xinf->fmt8; */
/*    else if (fmt->depth == xinf->fmt4->depth) */
/*      rs->x11.xcb.fmt = xinf->fmt4; */
/*    else */
/*      rs->x11.xcb.fmt = xinf->fmt1; */
/*    free(fmt); */

   rs->x11.xcb.fmt = xcb_render_find_visual_format(xinf->x11.connection, xinf->x11.visual);
   if (!rs->x11.xcb.fmt)
     {
        free(rs);
        return NULL;
     }
   rs->alpha = alpha;
   rs->depth = rs->x11.xcb.fmt->depth;
   if (rs->x11.xcb.fmt->depth == 32) rs->alpha = 1;
   rs->allocated = 0;
   rs->x11.xcb.draw = draw;
   rs->xinf->references++;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 1;
   values[2] = 0;
   rs->x11.xcb.pic = xcb_generate_id(xinf->x11.connection);
   xcb_render_create_picture(xinf->x11.connection, rs->x11.xcb.pic, rs->x11.xcb.draw, rs->x11.xcb.fmt->id, mask, values);
   if (rs->x11.xcb.pic == XCB_NONE)
     {
       rs->xinf->references--;
       free(rs->x11.xcb.fmt);
       free(rs);
       return NULL;
     }

   return rs;
}

Xrender_Surface *
_xr_xcb_render_surface_format_adopt(Ximage_Info *xinf, xcb_drawable_t draw, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha)
{
   Xrender_Surface *rs;
   uint32_t            mask;
   uint32_t            values[3];

   if ((!xinf) || (!fmt) || (draw == XCB_NONE) || (w < 1) || (h < 1)) return NULL;
   rs = calloc(1, sizeof(Xrender_Surface));
   if (!rs) return NULL;
   rs->xinf = xinf;
   rs->width = w;
   rs->height = h;
   rs->x11.xcb.fmt = (xcb_render_pictforminfo_t *)malloc (sizeof (xcb_render_pictforminfo_t));
   memcpy (rs->x11.xcb.fmt, fmt, sizeof (xcb_render_pictforminfo_t));
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   if (fmt->depth == 32) rs->alpha = 1;
   rs->xinf->references++;
   rs->allocated = 0;
   rs->x11.xcb.draw = draw;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 1;
   values[2] = 0;
   rs->x11.xcb.pic = xcb_generate_id(xinf->x11.connection);
   xcb_render_create_picture(xinf->x11.connection, rs->x11.xcb.pic, rs->x11.xcb.draw, fmt->id, mask, values);
   if (rs->x11.xcb.pic == XCB_NONE)
     {
       rs->xinf->references--;
       free(rs->x11.xcb.fmt);
       free(rs);
       return NULL;
     }

   return rs;
}

void
_xr_xcb_render_surface_free(Xrender_Surface *rs)
{
   if (!rs) return;
   if (rs->xinf)
     {
       if ((rs->allocated) && (rs->x11.xcb.draw != XCB_NONE))
          xcb_free_pixmap(rs->xinf->x11.connection, rs->x11.xcb.draw);
       if (rs->x11.xcb.pic != XCB_NONE)
          xcb_render_free_picture(rs->xinf->x11.connection, rs->x11.xcb.pic);
       _xr_xcb_image_info_free(rs->xinf);
       rs->xinf = NULL;
     }
   free(rs->x11.xcb.fmt);
   free(rs);
}

void
_xr_xcb_render_surface_repeat_set(Xrender_Surface *rs, int repeat)
{
   uint32_t mask;
   uint32_t value[1];

   mask = XCB_RENDER_CP_REPEAT;
   value[0] = repeat;
   xcb_render_change_picture(rs->xinf->x11.connection, rs->x11.xcb.pic, mask, value);
}

void
_xr_xcb_render_surface_solid_rectangle_set(Xrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h)
{
   xcb_render_color_t col;
   xcb_rectangle_t    rect;

   col.red   = (r << 8) | r;
   col.green = (g << 8) | g;
   col.blue  = (b << 8) | b;
   col.alpha = (a << 8) | a;
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_render_fill_rectangles(rs->xinf->x11.connection, XCB_RENDER_PICT_OP_SRC, rs->x11.xcb.pic, col, 1, &rect);
}

void
_xr_xcb_render_surface_argb_pixels_fill(Xrender_Surface *rs, int sw, int sh __UNUSED__, void *pixels, int x, int y, int w, int h, int ox, int oy)
{
   Ximage_Image  *xim;
   unsigned int  *p, *sp, *sple, *spe;
   unsigned int   jump, sjump;

   xim = _xr_xcb_image_new(rs->xinf, w, h, rs->depth);
   if (!xim) return;
   p = (unsigned int *)xim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   jump = ((xim->line_bytes / 4) - w);
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if
#ifdef WORDS_BIGENDIAN
     (xim->x11.xcb.xim->byte_order == XCB_IMAGE_ORDER_LSB_FIRST)
#else
     (xim->x11.xcb.xim->byte_order == XCB_IMAGE_ORDER_MSB_FIRST)
#endif
     {
	while (sp < spe)
	  {
	     sple = sp + w;
	     while (sp < sple)
	       {
		  *p++ = (*sp << 24) + ((*sp << 8) & 0xff0000) + ((*sp >> 8) & 0xff00) + (*sp >> 24);
//		  *p++ = (B_VAL(sp) << 24) | (G_VAL(sp) << 16) | (R_VAL(sp) << 8) | A_VAL(sp);
		  sp++;
	       }
	     p += jump;
	     sp += sjump;
	  }
     }
   else
     {
	while (sp < spe)
	  {
	     sple = sp + w;
	     while (sp < sple)
		*p++ = *sp++;
	     p += jump;
	     sp += sjump;
	  }
     }
   _xr_xcb_image_put(xim, rs->x11.xcb.draw, x + ox, y + oy, w, h);
}

void
_xr_xcb_render_surface_rgb_pixels_fill(Xrender_Surface *rs, int sw, int sh __UNUSED__, void *pixels, int x, int y, int w, int h, int ox, int oy)
{
   Ximage_Image *xim;
   unsigned int *p, *sp, *sple, *spe;
   unsigned int  jump, sjump;

   xim = _xr_xcb_image_new(rs->xinf, w, h, rs->depth);
   if (!xim) return;
   p = (unsigned int *)xim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if (rs->depth == 16)
     {
	/* FIXME: if rs->depth == 16 - convert */
	Gfx_Func_Convert conv_func;
	int swap;

	jump = ((xim->line_bytes / 2) - w);

#ifdef WORDS_BIGENDIAN
	swap = (int)(xim->x11.xcb.xim->byte_order == XCB_IMAGE_ORDER_LSB_FIRST);
#else
	swap = (int)(xim->x11.xcb.xim->byte_order == XCB_IMAGE_ORDER_MSB_FIRST);
#endif
	/* FIXME: swap not handled */
	conv_func = evas_common_convert_func_get((DATA8 *)sp, w, h, rs->depth,
						 ((xcb_visualtype_t *)rs->xinf->x11.visual)->red_mask,
						 ((xcb_visualtype_t *)rs->xinf->x11.visual)->green_mask,
						 ((xcb_visualtype_t *)rs->xinf->x11.visual)->blue_mask,
						 PAL_MODE_NONE, 0);
	if (conv_func)
	  conv_func(sp, (DATA8 *)p, sjump, jump, w, h, x, y, NULL);
     }
   else
     {
	jump = ((xim->line_bytes / 4) - w);
	if
#ifdef WORDS_BIGENDIAN
	  (xim->x11.xcb.xim->byte_order == XCB_IMAGE_ORDER_LSB_FIRST)
#else
	  (xim->x11.xcb.xim->byte_order == XCB_IMAGE_ORDER_MSB_FIRST)
#endif
	    {
	       while (sp < spe)
		 {
		    sple = sp + w;
		    while (sp < sple)
		      {
			 *p++ = (*sp << 24) + ((*sp << 8) & 0xff0000) + ((*sp >> 8) & 0xff00) + 0xff;
//		  *p++ = ((B_VAL(sp)) << 24) | ((G_VAL(sp)) << 16) | ((R_VAL(sp)) << 8) | 0x000000ff;
			 sp++;
		      }
		    p += jump;
		    sp += sjump;
		 }
	    }
	else
	  {
	     while (sp < spe)
	       {
		  sple = sp + w;
		  while (sp < sple)
		    *p++ = 0xff000000 | *sp++;
		  p += jump;
		  sp += sjump;
	       }
	  }
     }
   _xr_xcb_image_put(xim, rs->x11.xcb.draw, x + ox, y + oy, w, h);
}

void
_xr_xcb_render_surface_clips_set(Xrender_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh)
{
   int              num = 0;
   xcb_rectangle_t *rect = NULL;

   if ((dc) && (dc->clip.use))
     {
	RECTS_CLIP_TO_RECT(rx, ry, rw, rh,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
     }
   if ((!dc) || (!dc->cutout.rects))
     {
	rect = malloc(sizeof(xcb_rectangle_t));
	if (!rect) return;
	rect->x = rx;
	rect->y = ry;
	rect->width = rw;
	rect->height = rh;
	num = 1;
     }
   else
     {
        Cutout_Rects    *rects;
	Cutout_Rect     *r;
	int             i;

	rects = evas_common_draw_context_apply_cutouts(dc);
	num = rects->active;
	rect = malloc(num * sizeof(xcb_rectangle_t));
	if (!rect) return;
	for (i = 0; i < num; i++)
	  {
	     r = rects->rects + i;
	     rect[i].x = r->x;
	     rect[i].y = r->y;
	     rect[i].width = r->w;
	     rect[i].height = r->h;
	  }
	evas_common_draw_context_apply_clear_cutouts(rects);
     }
   if (!rect) return;
   xcb_render_set_picture_clip_rectangles(rs->xinf->x11.connection, rs->x11.xcb.pic, 0, 0, num, rect);
   free(rect);
}

/* initialized the transform to the identity */
static void
init_transform (xcb_render_transform_t *t)
{
   t->matrix11 = t->matrix22 = t->matrix33 = DOUBLE_TO_FIXED(1);

   t->matrix12 = t->matrix13 = t->matrix21 = t->matrix23 =
   t->matrix31 = t->matrix32 = 0;
}

static void
set_transform_scale(xcb_render_transform_t *t,
                    int                     sw,
                    int                     sh,
                    int                     w,
                    int                     h,
                    int                     tx,
                    int                     ty)
{
/*    if ((sw > 1) && (w > 1)) */
/*      { sw--;  w--; } */
/*    if ((sh > 1) && (h > 1)) */
/*      { sh--;  h--; } */
   t->matrix11 = DOUBLE_TO_FIXED((double)sw / (double)w);
   t->matrix22 = DOUBLE_TO_FIXED((double)sh / (double)h);
   t->matrix31 = (tx * sw) / w;
   t->matrix32 = (ty * sh) / h;
}

// when color multiplier is used want: instead
// CA src IN mask SRC temp; non-CA temp OVER dst. - i think. need to check.
void
_xr_xcb_render_surface_composite(Xrender_Surface *srs,
                             Xrender_Surface *drs,
                             RGBA_Draw_Context  *dc,
                             int                 sx,
                             int                 sy,
                             int                 sw,
                             int                 sh,
                             int                 x,
                             int                 y,
                             int                 w,
                             int                 h,
                             int                 smooth)
{
   Xrender_Surface    *trs = NULL;
   xcb_render_transform_t xf;
   xcb_render_picture_t   mask = XCB_NONE;
   uint32_t               value_mask;
   uint32_t               value_list[1];
   int                    e, is_scaling;
   xcb_render_pict_op_t   op;

   if ((sw <= 0) || (sh <= 0) || (w <= 0) || (h <= 0)) return;

   is_scaling = e = (sw != w) || (sh != h);

   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list[0] = 0;
   xcb_render_change_picture(srs->xinf->x11.connection, srs->x11.xcb.pic, value_mask, value_list);
   xcb_render_change_picture(drs->xinf->x11.connection, drs->x11.xcb.pic, value_mask, value_list);

   init_transform(&xf);

   op = XCB_RENDER_PICT_OP_OVER;
   if (dc->render_op == _EVAS_RENDER_BLEND)
     {
       if (!srs->alpha) op = XCB_RENDER_PICT_OP_SRC;
     }
   else if (dc->render_op == _EVAS_RENDER_BLEND_REL)
       op = XCB_RENDER_PICT_OP_ATOP;
   else if (dc->render_op == _EVAS_RENDER_MUL)
       op = XCB_RENDER_PICT_OP_IN;
   else if (dc->render_op == _EVAS_RENDER_COPY)
       op = XCB_RENDER_PICT_OP_SRC;
   else if (dc->render_op == _EVAS_RENDER_COPY_REL)
       op = XCB_RENDER_PICT_OP_IN;
   else if (dc->render_op == _EVAS_RENDER_MASK)
       op = XCB_RENDER_PICT_OP_IN_REVERSE;

   if ((dc) && (dc->mul.use))
     {
       int r, g, b, a;

       if ((op == XCB_RENDER_PICT_OP_OVER) && (!dc->mul.col)) return;
       a = dc->mul.col >> 24;
       r = (dc->mul.col >> 16) & 0xff;
       g = (dc->mul.col >> 8) & 0xff;
       b = dc->mul.col & 0xff;
       if (dc->mul.col != 0xffffffff)
         {
            if ((srs->xinf->mul_r != r) || (srs->xinf->mul_g != g) ||
                (srs->xinf->mul_b != b) || (srs->xinf->mul_a != a))
              {
                 srs->xinf->mul_r = r;
                 srs->xinf->mul_g = g;
                 srs->xinf->mul_b = b;
                 srs->xinf->mul_a = a;
                 _xr_xcb_render_surface_solid_rectangle_set(srs->xinf->mul,
                                                        r,
                                                        g,
                                                        b,
                                                        a,
                                                        0, 0, 1, 1);
              }
            mask = srs->xinf->mul->x11.xcb.pic;
            if (dc->mul.col == (a * 0x01010101))
              {
                 value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
                 value_list[0] = 0;
                 xcb_render_change_picture(srs->xinf->x11.connection, mask, value_mask, value_list);
              }
            else
              {
                 if ((srs->alpha) || (a != 0xff))
                   trs = _xr_xcb_render_surface_new(srs->xinf, sw + 2, sh + 2,
                                                srs->xinf->x11.fmt32, 1);
                 else
                   trs = _xr_xcb_render_surface_new(srs->xinf, sw + 2, sh + 2,
                                                srs->x11.xcb.fmt, srs->alpha);
                 if (!trs) return;

                 value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
                 value_list[0] = 1;
                 xcb_render_change_picture(srs->xinf->x11.connection, mask, value_mask, value_list);
                 xcb_render_set_picture_transform(trs->xinf->x11.connection, srs->x11.xcb.pic, xf);
                 xcb_render_composite(srs->xinf->x11.connection, XCB_RENDER_PICT_OP_SRC, srs->x11.xcb.pic, mask, trs->x11.xcb.pic,
                                      sx, sy, sx, sy, 0, 0, sw + 2, sh + 2);
                  mask = XCB_NONE;
              }
         }
     }

//#define HFW + (sw / 2)
//#define HFH + (sh / 2)
#define HFW
#define HFH

   _xr_xcb_render_surface_clips_set(drs, dc, x, y, w, h);
   if (trs)
     {
        set_filter(trs, smooth);

        set_transform_scale(&xf, sw, sh, w, h, -1, -1);
        xcb_render_set_picture_transform(trs->xinf->x11.connection, trs->x11.xcb.pic, xf);

        value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
        value_list[0] = 0;
	if (dc->render_op == _EVAS_RENDER_MUL)
           value_list[0] = 1;
        xcb_render_change_picture(trs->xinf->x11.connection, trs->x11.xcb.pic, value_mask, value_list);

        xcb_render_composite(trs->xinf->x11.connection, op, trs->x11.xcb.pic, mask, drs->x11.xcb.pic,
                             (w HFW) / sw, (h HFH) / sh,
                             (w HFW) / sw, (h HFH) / sh,
                             x, y, w, h);
        _xr_xcb_render_surface_free(trs);
     }
   else
     {
        if (srs->bordered && is_scaling)
	  {
	    trs = _xr_xcb_render_surface_new(srs->xinf, sw + 2, sh + 2,
					 srs->x11.xcb.fmt, srs->alpha);
	    if (!trs) return;

            value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
            value_list[0] = 0;
            xcb_render_change_picture(srs->xinf->x11.connection, srs->x11.xcb.pic, value_mask, value_list);
            xcb_render_set_picture_transform(srs->xinf->x11.connection, srs->x11.xcb.pic, xf);
            xcb_render_composite(srs->xinf->x11.connection, XCB_RENDER_PICT_OP_SRC, srs->x11.xcb.pic, XCB_NONE, trs->x11.xcb.pic,
                                 sx, sy, sx, sx, 0, 0, sw + 2, sh + 2);

            set_filter(trs, smooth);

	    set_transform_scale(&xf, sw, sh, w, h, -1, -1);
            xcb_render_set_picture_transform(trs->xinf->x11.connection, trs->x11.xcb.pic, xf);

	    if (dc->render_op == _EVAS_RENDER_MUL)
	      {
                 value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
                 value_list[0] = 1;
                 xcb_render_change_picture(trs->xinf->x11.connection, trs->x11.xcb.pic, value_mask, value_list);
	      }

            xcb_render_composite(trs->xinf->x11.connection, op, trs->x11.xcb.pic, mask, drs->x11.xcb.pic,
                                 (w HFW) / sw, (h HFH) / sh,
                                 (w HFW) / sw, (h HFH) / sh,
                                 x, y, w, h);
	    _xr_xcb_render_surface_free(trs);
	  }
	else
	  {
            set_filter(srs, smooth);

	    set_transform_scale(&xf, sw, sh, w, h, 0, 0);
            xcb_render_set_picture_transform(srs->xinf->x11.connection, srs->x11.xcb.pic, xf);

            value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
            value_list[0] = 0;
	    if (dc->render_op == _EVAS_RENDER_MUL)
               value_list[0] = 1;
            xcb_render_change_picture(srs->xinf->x11.connection, srs->x11.xcb.pic, value_mask, value_list);

            xcb_render_composite(srs->xinf->x11.connection, op, srs->x11.xcb.pic, mask, drs->x11.xcb.pic,
                                 ((((sx + 1) * w) HFW) / sw),
                                 ((((sy + 1) * h) HFH) / sh),
                                 ((((sx + 1) * w) HFW) / sw),
                                 ((((sy + 1) * h) HFH) / sh),
                                 x, y, w, h);
	  }
      }
}

void
_xr_xcb_render_surface_copy(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h)
{
   xcb_render_transform_t xf;
   uint32_t               value_mask;
   uint32_t               value_list[1];

   if ((w <= 0) || (h <= 0) || (!srs) || (!drs)) return;

   init_transform(&xf);
#ifdef BROKEN_XORG_XRENDER
   /* FIXME: why do we need to change the identity matrix if the src surface
    *        is 1 bit deep?
    */
   if (srs->depth == 1)
     {
	xf.matrix11 = xf.matrix22 = xf.matrix33 = 1;
     }
#endif
   xcb_render_set_picture_transform(srs->xinf->x11.connection, srs->x11.xcb.pic, xf);
/*    set_filter(srs, 0); */

   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list[0] = 0;
   xcb_render_change_picture(srs->xinf->x11.connection, srs->x11.xcb.pic, value_mask, value_list);
   xcb_render_change_picture(drs->xinf->x11.connection, drs->x11.xcb.pic, value_mask, value_list);

   xcb_render_composite(srs->xinf->x11.connection, XCB_RENDER_PICT_OP_SRC, srs->x11.xcb.pic, XCB_NONE, drs->x11.xcb.pic,
                        sx, sy, 0, 0, x, y, w, h);
}

void
_xr_xcb_render_surface_rectangle_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   xcb_render_color_t   col;
   xcb_rectangle_t      rect;
   uint32_t             value_mask;
   uint32_t             value_list;
   int                  r, g, b, a;
   xcb_render_pict_op_t op;

   if ((!rs) || (!dc)) return;
   if ((w <= 0) || (h <= 0)) return;
   a = dc->col.col >> 24;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   col.red   = (r << 8) | r;
   col.green = (g << 8) | g;
   col.blue  = (b << 8) | b;
   col.alpha = (a << 8) | a;

   op = XCB_RENDER_PICT_OP_SRC;
   if (dc->render_op == _EVAS_RENDER_BLEND)
     {
	if (!dc->col.col) return;
	if (a == 0xff) op = XCB_RENDER_PICT_OP_SRC;
     }
   else if (dc->render_op == _EVAS_RENDER_BLEND_REL)
     {
	if (!dc->col.col) return;
	op = XCB_RENDER_PICT_OP_ATOP;
     }
   else if (dc->render_op == _EVAS_RENDER_MUL)
     {
	if (dc->col.col == 0xffffffff) return;
	op = XCB_RENDER_PICT_OP_IN;
     }
   else if (dc->render_op == _EVAS_RENDER_COPY)
	op = XCB_RENDER_PICT_OP_SRC;
   else if (dc->render_op == _EVAS_RENDER_COPY_REL)
	op = XCB_RENDER_PICT_OP_IN;
   else if (dc->render_op == _EVAS_RENDER_MASK)
	op = XCB_RENDER_PICT_OP_IN_REVERSE;

   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list = 0;
   xcb_render_change_picture(rs->xinf->x11.connection, rs->x11.xcb.pic, value_mask, &value_list);

   _xr_xcb_render_surface_clips_set(rs, dc, x, y, w, h);
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_render_fill_rectangles(rs->xinf->x11.connection, op, rs->x11.xcb.pic, col, 1, &rect);
}

void
_xr_xcb_render_surface_line_draw(Xrender_Surface *rs __UNUSED__, RGBA_Draw_Context *dc __UNUSED__, int x1 __UNUSED__, int y1 __UNUSED__, int x2 __UNUSED__, int y2 __UNUSED__)
{
/*    uint32_t         value_mask; */
/*    uint32_t         value_list[1]; */
/*    int            op; */

/*    if ((!rs) || (!dc)) return; */
/*    op = XCB_RENDER_PICT_OP_SRC; */
/*    value_mask = XCB_RENDER_CP_CLIP_MASK; */
/*    value_list[0] = 0; */
/*    xcb_render_change_picture(rs->xinf->x11.connection, rs->x11.xcb.pic, value_mask, value_list); */
/*    _xr_xcb_render_surface_clips_set(rs, dc, 0, 0, rs->width, rs->height); */

/*      { */
/*	int r, g, b, a; */
/*	XPointDouble poly[4]; */
/*	int dx, dy; */
/*	double len, ddx, ddy; */

/*	dx = x2 - x1; */
/*	dy = y2 - y1; */
/*	len = sqrt((double)(dx * dx) + (double)(dy * dy)); */
/*	ddx = (0.5 * dx) / len; */
/*	ddy = (0.5 * dy) / len; */
/*	poly[0].x =  (x1 + ddx); */
/*	poly[0].y =  (y1 - ddy); */
/*	poly[1].x =  (x2 + ddx); */
/*	poly[1].y =  (y2 - ddy); */
/*	poly[2].x =  (x2 - ddx); */
/*	poly[2].y =  (y2 + ddy); */
/*	poly[3].x =  (x1 - ddx); */
/*	poly[3].y =  (y1 + ddy); */

/*	a = (dc->col.col >> 24) & 0xff; */
/*	if (a == 0) return; */
/*	if (a < 0xff) op = XCB_RENDER_PICT_OP_OVER; */
/*	r = (dc->col.col >> 16) & 0xff; */
/*	g = (dc->col.col >> 8 ) & 0xff; */
/*	b = (dc->col.col      ) & 0xff; */
/*	if ((rs->xinf->mul_r != r) || (rs->xinf->mul_g != g) || */
/*	    (rs->xinf->mul_b != b) || (rs->xinf->mul_a != a)) */
/*	  { */
/*	     rs->xinf->mul_r = r; */
/*	     rs->xinf->mul_g = g; */
/*	     rs->xinf->mul_b = b; */
/*	     rs->xinf->mul_a = a; */
/*	     _xr_xcb_render_surface_solid_rectangle_set(rs->xinf->mul, r, g, b, a, 0, 0, 1, 1); */
/*	  } */
/*	XRenderCompositeDoublePoly(rs->xinf->x11.connection, op, */
/*				   rs->xinf->mul->pic, rs->x11.xcb.pic,  */
/*				   rs->xinf->fmt8, 0, 0, 0, 0, */
/*				   poly, 4, EvenOddRule); */
/*      } */
}

void
_xr_xcb_render_surface_polygon_draw(Xrender_Surface *rs __UNUSED__, RGBA_Draw_Context *dc __UNUSED__, RGBA_Polygon_Point *points __UNUSED__)
{
/*    RGBA_Polygon_Point *pt; */
/*    int i, num; */
/*    XPointDouble *pts; */
/*    int r, g, b, a; */
/*    uint32_t         value_mask; */
/*    uint32_t         value_list[1]; */
/*    int op; */

/*    if ((!rs) || (!dc)) return; */
/*    num = 0; EINA_INLIST_FOREACH(points, pt) num++; */
/*    if (num < 3) return; */
/*    a = (dc->col.col >> 24) & 0xff; */
/*    if (a == 0) return; */
/*    op = XCB_RENDER_PICT_OP_OVER; */
/*    r = (dc->col.col >> 16) & 0xff; */
/*    g = (dc->col.col >> 8 ) & 0xff; */
/*    b = (dc->col.col      ) & 0xff; */
/*    if ((rs->xinf->mul_r != r) || (rs->xinf->mul_g != g) || */
/*        (rs->xinf->mul_b != b) || (rs->xinf->mul_a != a)) */
/*      { */
/*	rs->xinf->mul_r = r; */
/*	rs->xinf->mul_g = g; */
/*	rs->xinf->mul_b = b; */
/*	rs->xinf->mul_a = a; */
/*	_xr_xcb_render_surface_solid_rectangle_set(rs->xinf->mul, r, g, b, a, 0, 0, 1, 1); */
/*      } */
/*    pts = malloc(num * sizeof(XPointDouble)); */
/*    if (!pts) return; */
/*    i = 0; */
/*    EINA_INLIST_FOREACH(points, pt) */
/*      { */
/*	if (i < num) */
/*	  { */
/*	     pts[i].x = pt->x; */
/*	     pts[i].y = pt->y; */
/*	     i++; */
/*	  } */
/*      } */
/*    value_mask = XCB_RENDER_CP_CLIP_MASK; */
/*    value_list[0] = 0; */
/*    xcb_render_change_picture(rs->xinf->x11.connection, rs->x11.xcb.pic, value_mask, value_list); */

/*    _xr_xcb_render_surface_clips_set(rs, dc, 0, 0, rs->width, rs->height); */
/*    XRenderCompositeDoublePoly(rs->xinf->x11.connection, op, */
/*			      rs->xinf->mul->pic, rs->x11.xcb.pic,  */
/*			      rs->xinf->fmt8, 0, 0, 0, 0, */
/*			      pts, num, Complex); */
/*    free(pts); */
}
