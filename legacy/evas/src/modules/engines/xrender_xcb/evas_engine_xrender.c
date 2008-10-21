/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "evas_common.h"
/* #include "evas_macros.h" */
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_Xcb.h"
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
set_filter(Xcb_Render_Surface *s, int smooth)
{
   const char *f = smooth ? "best": "nearest";

   xcb_render_set_picture_filter (s->xcbinf->conn, s->pic, strlen (f), f, 0, NULL);
}

xcb_render_pictforminfo_t *
xcb_render_find_visual_format (xcb_connection_t *c, xcb_visualid_t visual)
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
        if (visual == visual_iter.data->visual) {
          format = visual_iter.data->format;
        }
      }
    }
  }
  if (format != 0) {
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
  free (rep);

  return NULL;
}

Xcb_Render_Surface *
_xr_render_surface_new(Xcb_Image_Info *xcbinf, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha)
{
   Xcb_Render_Surface *rs;
   uint32_t            mask;
   uint32_t            values[3];

   if ((!xcbinf) || (!fmt) || (w < 1) || (h < 1)) return NULL;

   rs = calloc(1, sizeof(Xcb_Render_Surface));
   if (!rs) return NULL;
   rs->xcbinf = xcbinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = (xcb_render_pictforminfo_t *)malloc (sizeof (xcb_render_pictforminfo_t));
   if (!rs->fmt)
     {
        free(rs);
        return NULL;
     }
   memcpy (rs->fmt, fmt, sizeof (xcb_render_pictforminfo_t));
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   rs->allocated = 1;
   rs->draw = xcb_generate_id(xcbinf->conn);
   xcb_create_pixmap(xcbinf->conn, fmt->depth, rs->draw, xcbinf->root, w, h);
   if (rs->draw == 0)
     {
       free(rs->fmt);
       free(rs);
       return NULL;
     }
   rs->xcbinf->references++;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = xcb_generate_id(xcbinf->conn);
   xcb_render_create_picture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);
   if (rs->pic == 0)
     {
       xcb_free_pixmap(rs->xcbinf->conn, rs->draw);
       rs->xcbinf->references--;
       free(rs->fmt);
       free(rs);
       return NULL;
     }

   return rs;
}

Xcb_Render_Surface *
_xr_render_surface_adopt(Xcb_Image_Info *xcbinf, xcb_drawable_t draw, int w, int h, int alpha)
{
   Xcb_Render_Surface        *rs;
   uint32_t                   mask;
   uint32_t                   values[3];

   if ((!xcbinf) || (draw == 0) || (w < 1) || (h < 1)) return NULL;
   rs = calloc(1, sizeof(Xcb_Render_Surface));
   if (!rs) return NULL;
   rs->xcbinf = xcbinf;
   rs->w = w;
   rs->h = h;
/*    if (fmt->depth == xcbinf->fmt32->depth) */
/*      rs->fmt = xcbinf->fmt32; */
/*    else if (fmt->depth == xcbinf->fmt24->depth) */
/*      rs->fmt = xcbinf->fmt24; */
/*    else if (fmt->depth == xcbinf->fmt8->depth) */
/*      rs->fmt = xcbinf->fmt8; */
/*    else if (fmt->depth == xcbinf->fmt4->depth) */
/*      rs->fmt = xcbinf->fmt4; */
/*    else */
/*      rs->fmt = xcbinf->fmt1; */
/*    free(fmt); */

   rs->fmt = xcb_render_find_visual_format(xcbinf->conn, xcbinf->vis);
   if (!rs->fmt)
     {
        free(rs);
        return NULL;
     }
   rs->alpha = alpha;
   rs->depth = rs->fmt->depth;
   if (rs->fmt->depth == 32) rs->alpha = 1;
   rs->allocated = 0;
   rs->draw = draw;
   rs->xcbinf->references++;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = xcb_generate_id(xcbinf->conn);
   xcb_render_create_picture(xcbinf->conn, rs->pic, rs->draw, rs->fmt->id, mask, values);
   if (rs->pic == 0)
     {
       rs->xcbinf->references--;
       free(rs->fmt);
       free(rs);
       return NULL;
     }

   return rs;
}

Xcb_Render_Surface *
_xr_render_surface_format_adopt(Xcb_Image_Info *xcbinf, xcb_drawable_t draw, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha)
{
   Xcb_Render_Surface *rs;
   uint32_t            mask;
   uint32_t            values[3];

   if ((!xcbinf) || (!fmt) || (draw == 0) || (w < 1) || (h < 1)) return NULL;
   rs = calloc(1, sizeof(Xcb_Render_Surface));
   if (!rs) return NULL;
   rs->xcbinf = xcbinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = (xcb_render_pictforminfo_t *)malloc (sizeof (xcb_render_pictforminfo_t));
   memcpy (rs->fmt, fmt, sizeof (xcb_render_pictforminfo_t));
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   if (fmt->depth == 32) rs->alpha = 1;
   rs->xcbinf->references++;
   rs->allocated = 0;
   rs->draw = draw;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = xcb_generate_id(xcbinf->conn);
   xcb_render_create_picture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);
   if (rs->pic == 0)
     {
       rs->xcbinf->references--;
       free(rs->fmt);
       free(rs);
       return NULL;
     }

   return rs;
}

void
_xr_render_surface_free(Xcb_Render_Surface *rs)
{
   if (!rs) return;
   if (rs->xcbinf)
     {
       if ((rs->allocated) && (rs->draw != 0))
          xcb_free_pixmap(rs->xcbinf->conn, rs->draw);
       if (rs->pic != 0)
          xcb_render_free_picture(rs->xcbinf->conn, rs->pic);
       _xr_image_info_free(rs->xcbinf);
       rs->xcbinf = NULL;
     }
   free(rs->fmt);
   free(rs);
}

void
_xr_render_surface_repeat_set(Xcb_Render_Surface *rs, int repeat)
{
   uint32_t mask;
   uint32_t value[1];

   mask = XCB_RENDER_CP_REPEAT;
   value[0] = repeat;
   xcb_render_change_picture(rs->xcbinf->conn, rs->pic, mask, value);
}

void
_xr_render_surface_solid_rectangle_set(Xcb_Render_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h)
{
   xcb_render_color_t col;
   xcb_rectangle_t    rect;
   int                aa;

   aa = a + 1;
   r = (r * aa) >> 8;
   g = (g * aa) >> 8;
   b = (b * aa) >> 8;
   col.red   = (r << 8) | r;
   col.green = (g << 8) | g;
   col.blue  = (b << 8) | b;
   col.alpha = (a << 8) | a;
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_render_fill_rectangles(rs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, rs->pic, col, 1, &rect);
}

void
_xr_render_surface_argb_pixels_fill(Xcb_Render_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h)
{
   Xcb_Image_Image  *xcbim;
   unsigned int     *p, *sp, *sple, *spe;
   unsigned int      jump, sjump;
   unsigned int      a, r, g, b, aa;

   xcbim = _xr_image_new(rs->xcbinf, w, h, rs->depth);
   if (!xcbim) return;
   p = (unsigned int *)xcbim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   jump = ((xcbim->line_bytes / 4) - w);
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if
#ifdef WORDS_BIGENDIAN
     (xcbim->xcbim->image_byte_order == XCB_IMAGE_ORDER_LSB_FIRST)
#else
     (xcbim->xcbim->image_byte_order == XCB_IMAGE_ORDER_MSB_FIRST)
#endif
     {
	while (sp < spe)
	  {
	     sple = sp + w;
	     while (sp < sple)
	       {
		  switch (a = A_VAL(sp))
		  {
		    case 0:
			*p = 0;
		    break;
		    case 255:
			*p = (B_VAL(sp) << 24) | (G_VAL(sp) << 16) | (R_VAL(sp) << 8) | 0xff;
		    break;
		    default:
			aa = a + 1;
			r = ((R_VAL(sp)) * aa) >> 8;
			g = ((G_VAL(sp)) * aa) >> 8;
			b = ((B_VAL(sp)) * aa) >> 8;
			*p = (b << 24) | (g << 16) | (r << 8) | a;
		    break;
		  }
		  p++;
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
	       {
                  switch (a = (*sp & 0xff000000))
		  {
		    case 0:
			*p = 0;
		    break;
		    case 0xff000000:
			*p = *sp;
		    break;
                  default:
                    aa = (a >> 24) + 1;
                    *p = a + (((((*sp) >> 8) & 0xff) * aa) & 0xff00) +
                      (((((*sp) & 0x00ff00ff) * aa) >> 8) & 0x00ff00ff);
		    break;
		  }
		  p++;
		  sp++;
	       }
	     p += jump;
	     sp += sjump;
	  }
     }
   _xr_image_put(xcbim, rs->draw, x, y, w, h);
}

void
_xr_render_surface_rgb_pixels_fill(Xcb_Render_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h)
{
   Xcb_Image_Image *xcbim;
   unsigned int    *p, *sp, *sple, *spe;
   unsigned int     jump, sjump;

   xcbim = _xr_image_new(rs->xcbinf, w, h, rs->depth);
   if (!xcbim) return;
   p = (unsigned int *)xcbim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   jump = ((xcbim->line_bytes / 4) - w);
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if
#ifdef WORDS_BIGENDIAN
     (xcbim->xcbim->image_byte_order == XCB_IMAGE_ORDER_LSB_FIRST)
#else
     (xcbim->xcbim->image_byte_order == XCB_IMAGE_ORDER_MSB_FIRST)
#endif
     {
	while (sp < spe)
	  {
	     sple = sp + w;
	     while (sp < sple)
	       {
		  *p = ((B_VAL(sp)) << 24) | ((G_VAL(sp)) << 16) | ((R_VAL(sp)) << 8) | 0x000000ff;
		  p++;
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
	       {
                  *p = 0xff000000 | *sp;
		  p++;
		  sp++;
	       }
	     p += jump;
	     sp += sjump;
	  }
     }
   _xr_image_put(xcbim, rs->draw, x, y, w, h);
}

void
_xr_render_surface_clips_set(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh)
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
   xcb_render_set_picture_clip_rectangles(rs->xcbinf->conn, rs->pic, 0, 0, num, rect);
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
                    int                     h)
{
   if ((sw > 1) && (w > 1))
     { sw--;  w--; }
   if ((sh > 1) && (h > 1))
     { sh--;  h--; }
   t->matrix11 = DOUBLE_TO_FIXED((double)sw / (double)w);
   t->matrix22 = DOUBLE_TO_FIXED((double)sh / (double)h);
}

// when color multiplier is used want: instead
// CA src IN mask SRC temp; non-CA temp OVER dst. - i think. need to check.
void
_xr_render_surface_composite(Xcb_Render_Surface *srs,
                             Xcb_Render_Surface *drs,
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
   Xcb_Render_Surface    *trs = NULL;
   xcb_render_transform_t xf;
   xcb_render_picture_t   mask = XCB_NONE;
   uint32_t               value_mask;
   uint32_t               value_list[1];
   int                    e, is_scaling;
   xcb_render_pict_op_t   op;

   if ((sw < 1) || (sh < 1) || (w < 1) || (h < 1)) return;

   is_scaling = e = (sw != w) || (sh != h);

   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list[0] = 0;
   xcb_render_change_picture(srs->xcbinf->conn, srs->pic, value_mask, value_list);
   xcb_render_change_picture(drs->xcbinf->conn, drs->pic, value_mask, value_list);

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
            if ((srs->xcbinf->mul_r != r) || (srs->xcbinf->mul_g != g) ||
                (srs->xcbinf->mul_b != b) || (srs->xcbinf->mul_a != a))
              {
                 srs->xcbinf->mul_r = r;
                 srs->xcbinf->mul_g = g;
                 srs->xcbinf->mul_b = b;
                 srs->xcbinf->mul_a = a;
                 _xr_render_surface_solid_rectangle_set(srs->xcbinf->mul,
                                                        r,
                                                        g,
                                                        b,
                                                        a,
                                                        0, 0, 1, 1);
              }
            mask = srs->xcbinf->mul->pic;
            if (dc->mul.col == (a * 0x01010101))
              {
                 value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
                 value_list[0] = 0;
                 xcb_render_change_picture(srs->xcbinf->conn, mask, value_mask, value_list);
              }
            else
              {
                 if ((srs->alpha) || (a != 0xff))
                   trs = _xr_render_surface_new(srs->xcbinf, sw + e, sh + e,
                                                srs->xcbinf->fmt32, 1);
                 else
                   trs = _xr_render_surface_new(srs->xcbinf, sw + e, sh + e,
                                                srs->fmt, srs->alpha);
                 if (!trs) return;

                 value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
                 value_list[0] = 0;
                 xcb_render_change_picture(srs->xcbinf->conn, mask, value_mask, value_list);
                 xcb_render_set_picture_transform(trs->xcbinf->conn, srs->pic, xf);
                 xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                      sx, sy, 0, 0, 0, 0, sw, sh);
                 /* fill right and bottom pixel so interpolation works right */
		  if (e)
		    {
                       xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                            sx + sw - 1, sy, 0, 0, sw, 0, 1, sh);
                       xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                            sx, sy + sh - 1, 0, 0, 0, sh, sw, 1);
                       xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                            sx + sw - 1, sy + sh - 1, 0, 0, sw, sh, 1, 1);
                    }
                  mask = XCB_NONE;
              }
         }
     }

   _xr_render_surface_clips_set(drs, dc, x, y, w, h);
   if (trs)
     {
        set_filter(trs, smooth);

        set_transform_scale(&xf, sw, sh, w, h);
        xcb_render_set_picture_transform(trs->xcbinf->conn, trs->pic, xf);

        value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
        value_list[0] = 0;
	if (dc->render_op == _EVAS_RENDER_MUL)
           value_list[0] = 1;
        xcb_render_change_picture(trs->xcbinf->conn, trs->pic, value_mask, value_list);

        xcb_render_composite(trs->xcbinf->conn, op, trs->pic, mask, drs->pic,
                             0, 0, 0, 0, x, y, w, h);
        _xr_render_surface_free(trs);
     }
   else
     {
        if (srs->bordered && is_scaling)
	  {
	    trs = _xr_render_surface_new(srs->xcbinf, sw + 1, sh + 1,
					 srs->fmt, srs->alpha);
	    if (!trs) return;

            value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
            value_list[0] = 0;
            xcb_render_change_picture(srs->xcbinf->conn, srs->pic, value_mask, value_list);
            xcb_render_set_picture_transform(srs->xcbinf->conn, srs->pic, xf);
            xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, XCB_NONE, trs->pic,
                                 sx, sy, 0, 0, 0, 0, sw, sh);

            xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, XCB_NONE, trs->pic,
                                 sx + sw - 1, sy, 0, 0, sw, 0, 1, sh);
            xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, XCB_NONE, trs->pic,
                                 sx, sy + sh - 1, 0, 0, 0, sh, sw, 1);
            xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, XCB_NONE, trs->pic,
                                 sx + sw - 1, sy + sh - 1, 0, 0, sw, sh, 1, 1);

            set_filter(trs, smooth);

	    set_transform_scale(&xf, sw, sh, w, h);
            xcb_render_set_picture_transform(trs->xcbinf->conn, trs->pic, xf);

	    if (dc->render_op == _EVAS_RENDER_MUL)
	      {
                 value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
                 value_list[0] = 1;
                 xcb_render_change_picture(trs->xcbinf->conn, trs->pic, value_mask, value_list);
	      }

            xcb_render_composite(trs->xcbinf->conn, op, trs->pic, mask, drs->pic,
                                 0, 0, 0, 0, x, y, w, h);
	    _xr_render_surface_free(trs);
	  }
	else
	  {
            set_filter(srs, smooth);

	    set_transform_scale(&xf, sw, sh, w, h);
            xcb_render_set_picture_transform(srs->xcbinf->conn, srs->pic, xf);

            value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
            value_list[0] = 0;
	    if (dc->render_op == _EVAS_RENDER_MUL)
               value_list[0] = 1;
            xcb_render_change_picture(srs->xcbinf->conn, srs->pic, value_mask, value_list);

            xcb_render_composite(srs->xcbinf->conn, op, srs->pic, mask, drs->pic,
			     ((sx * w) + (sw / 2)) / sw, 
			     ((sy * h) + (sh / 2)) / sh,
			     0, 0, x, y, w, h);
	  }
      }
}

void
_xr_render_surface_copy(Xcb_Render_Surface *srs, Xcb_Render_Surface *drs, int sx, int sy, int x, int y, int w, int h)
{
   xcb_render_transform_t xf;
   uint32_t               value_mask;
   uint32_t               value_list[1];

   if ((w < 1) || (h < 1) || (!srs) || (!drs)) return;

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
   xcb_render_set_picture_transform(srs->xcbinf->conn, srs->pic, xf);
/*    set_filter(srs, 0); */

   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list[0] = 0;
   xcb_render_change_picture(srs->xcbinf->conn, srs->pic, value_mask, value_list);
   xcb_render_change_picture(drs->xcbinf->conn, drs->pic, value_mask, value_list);

   xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, XCB_NONE, drs->pic,
                        sx, sy, 0, 0, x, y, w, h);
}

void
_xr_render_surface_rectangle_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   xcb_render_color_t   col;
   xcb_rectangle_t      rect;
   uint32_t             value_mask;
   uint32_t             value_list;
   int                  r, g, b, a;
   xcb_render_pict_op_t op;

   if ((!rs) || (!dc)) return;
   if ((w < 1) || (h < 1)) return;
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
   xcb_render_change_picture(rs->xcbinf->conn, rs->pic, value_mask, &value_list);

   _xr_render_surface_clips_set(rs, dc, x, y, w, h);
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_render_fill_rectangles(rs->xcbinf->conn, op, rs->pic, col, 1, &rect);
}

void
_xr_render_surface_line_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2)
{
/*    uint32_t         value_mask; */
/*    uint32_t         value_list[1]; */
/*    int            op; */

/*    if ((!rs) || (!dc)) return; */
/*    op = XCB_RENDER_PICT_OP_SRC; */
/*    value_mask = XCB_RENDER_CP_CLIP_MASK; */
/*    value_list[0] = 0; */
/*    xcb_render_change_picture(rs->xcbinf->conn, rs->pic, value_mask, value_list); */
/*    _xr_render_surface_clips_set(rs, dc, 0, 0, rs->w, rs->h); */

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
/*	if ((rs->xcbinf->mul_r != r) || (rs->xcbinf->mul_g != g) || */
/*	    (rs->xcbinf->mul_b != b) || (rs->xcbinf->mul_a != a)) */
/*	  { */
/*	     rs->xcbinf->mul_r = r; */
/*	     rs->xcbinf->mul_g = g; */
/*	     rs->xcbinf->mul_b = b; */
/*	     rs->xcbinf->mul_a = a; */
/*	     _xr_render_surface_solid_rectangle_set(rs->xcbinf->mul, r, g, b, a, 0, 0, 1, 1); */
/*	  } */
/*	XRenderCompositeDoublePoly(rs->xcbinf->conn, op, */
/*				   rs->xcbinf->mul->pic, rs->pic,  */
/*				   rs->xcbinf->fmt8, 0, 0, 0, 0, */
/*				   poly, 4, EvenOddRule); */
/*      } */
}

void
_xre_poly_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points)
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
/*    if ((rs->xcbinf->mul_r != r) || (rs->xcbinf->mul_g != g) || */
/*        (rs->xcbinf->mul_b != b) || (rs->xcbinf->mul_a != a)) */
/*      { */
/*	rs->xcbinf->mul_r = r; */
/*	rs->xcbinf->mul_g = g; */
/*	rs->xcbinf->mul_b = b; */
/*	rs->xcbinf->mul_a = a; */
/*	_xr_render_surface_solid_rectangle_set(rs->xcbinf->mul, r, g, b, a, 0, 0, 1, 1); */
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
/*    xcb_render_change_picture(rs->xcbinf->conn, rs->pic, value_mask, value_list); */

/*    _xr_render_surface_clips_set(rs, dc, 0, 0, rs->w, rs->h); */
/*    XRenderCompositeDoublePoly(rs->xcbinf->conn, op, */
/*			      rs->xcbinf->mul->pic, rs->pic,  */
/*			      rs->xcbinf->fmt8, 0, 0, 0, 0, */
/*			      pts, num, Complex); */
/*    free(pts); */
}
