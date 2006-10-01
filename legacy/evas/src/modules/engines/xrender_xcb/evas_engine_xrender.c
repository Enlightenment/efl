/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "evas_common.h"
#include "evas_macros.h"
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
        if (visual.id == visual_iter.data->visual.id) {
          format = visual_iter.data->format;
        }
      }
    }
  }
  if (format.xid != 0) {
    xcb_render_pictforminfo_iterator_t forminfo_iter;

    forminfo_iter = xcb_render_query_pict_formats_formats_iterator (rep);
    for (; forminfo_iter.rem; xcb_render_pictforminfo_next (&forminfo_iter)) {
      if (forminfo_iter.data->id.xid == format.xid) {
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
   rs->fmt = fmt;
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   rs->allocated = 1;
   rs->draw.pixmap = xcb_pixmap_new(xcbinf->conn);
   xcb_create_pixmap(xcbinf->conn, fmt->depth, rs->draw.pixmap, xcbinf->root, w, h);
   if (rs->draw.pixmap.xid == 0)
     {
       free(rs);
       return NULL;
     }
   rs->xcbinf->references++;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = xcb_render_picture_new(xcbinf->conn);
   xcb_render_create_picture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);
   if (rs->pic.xid == 0)
     {
       xcb_free_pixmap(rs->xcbinf->conn, rs->draw.pixmap);
       rs->xcbinf->references--;
       free(rs);
       return NULL;
     }

   return rs;
}

Xcb_Render_Surface *
_xr_render_surface_adopt(Xcb_Image_Info *xcbinf, xcb_drawable_t draw, int w, int h, int alpha)
{
   Xcb_Render_Surface        *rs;
   xcb_render_pictforminfo_t *fmt;
   uint32_t                   mask;
   uint32_t                   values[3];

   if ((!xcbinf) || (draw.pixmap.xid == 0) || (w < 1) || (h < 1)) return NULL;
   fmt = xcb_render_find_visual_format(xcbinf->conn, xcbinf->vis);
   if (!fmt) return NULL;
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

   rs->fmt = fmt;

   rs->alpha = alpha;
   rs->depth = fmt->depth;
   if (fmt->depth == 32) rs->alpha = 1;
   rs->allocated = 0;
   rs->draw = draw;
   rs->xcbinf->references++;
   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = xcb_render_picture_new(xcbinf->conn);
   xcb_render_create_picture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);
   if (rs->pic.xid == 0)
     {
       rs->xcbinf->references--;
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

   if ((!xcbinf) || (!fmt) || (draw.pixmap.xid == 0) || (w < 1) || (h < 1)) return NULL;
   rs = calloc(1, sizeof(Xcb_Render_Surface));
   if (!rs) return NULL;
   rs->xcbinf = xcbinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = fmt;
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
   rs->pic = xcb_render_picture_new(xcbinf->conn);
   xcb_render_create_picture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);
   if (rs->pic.xid == 0)
     {
       rs->xcbinf->references--;
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
       if ((rs->allocated) && (rs->draw.pixmap.xid != 0))
          xcb_free_pixmap(rs->xcbinf->conn, rs->draw.pixmap);
       if (rs->pic.xid != 0)
          xcb_render_free_picture(rs->xcbinf->conn, rs->pic);
       _xr_image_info_free(rs->xcbinf);
       rs->xcbinf = NULL;
     }
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
	int i;
	Cutout_Rect *rects, *r;
	Evas_Object_List *l;

	rects = evas_common_draw_context_apply_cutouts(dc);
	for (num = 0, l = (Evas_Object_List *)rects; l; l = l->next) num++;
	rect = malloc(num * sizeof(xcb_rectangle_t));
	if (!rect) return;
	for (i = 0, l = (Evas_Object_List *)rects; l; l = l->next, i++)
	  {
	     r = (Cutout_Rect *)l;
	     rect[i].x = r->x;
	     rect[i].y = r->y;
	     rect[i].width = r->w;
	     rect[i].height = r->h;
	  }
	evas_common_draw_context_apply_free_cutouts(rects);
     }
   if (!rect) return;
   xcb_render_set_picture_clip_rectangles(rs->xcbinf->conn, rs->pic, 0, 0, num, rect);
   free(rect);
}

/* initialized the transform to the identity */
static void init_transform (xcb_render_transform_t *t)
{
   t->matrix11 = t->matrix22 = t->matrix33 = DOUBLE_TO_FIXED(1);

   t->matrix12 = t->matrix13 = t->matrix21 = t->matrix23 =
   t->matrix31 = t->matrix32 = 0;
}

// when color multiplier is used want: instead
// CA src IN mask SRC temp; non-CA temp OVER dst. - i think. need to check.
void
_xr_render_surface_composite(Xcb_Render_Surface *srs, Xcb_Render_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth)
{
   Xcb_Render_Surface    *trs = NULL;
   xcb_render_transform_t xf;
   xcb_render_picture_t   mask;
   uint32_t               value_mask;
   uint32_t               value_list[1];
   int                    r, g, b, a;
   int                    op, is_scaling;

   if ((sw <= 0) || (sh <= 0) || (w <= 0) || (h <= 0)) return;

   is_scaling = (sw != w) || (sh != h);

   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list[0] = 0;
   xcb_render_change_picture(srs->xcbinf->conn, srs->pic, value_mask, value_list);
   xcb_render_change_picture(srs->xcbinf->conn, drs->pic, value_mask, value_list);

   op = XCB_RENDER_PICT_OP_SRC;
   if (srs->alpha) op = XCB_RENDER_PICT_OP_OVER;
   mask.xid = 0;
   if ((dc) && (dc->mul.use))
     {
       r = (int)(R_VAL(&dc->mul.col));
       g = (int)(G_VAL(&dc->mul.col));
       b = (int)(B_VAL(&dc->mul.col));
       a = (int)(A_VAL(&dc->mul.col));
       if ((r != 0xff) || (g != 0xff) || (b != 0xff) || (a != 0xff))
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
            op = XCB_RENDER_PICT_OP_OVER;
            value_mask = XCB_RENDER_CP_COMPONENT_ALPHA;
            value_list[0] = 1;
            mask = srs->xcbinf->mul->pic;
            xcb_render_change_picture(srs->xcbinf->conn, mask, value_mask, value_list);
            if ((r == 0xff) && (g == 0xff) && (b == 0xff) && (a != 0xff))
              {
              }
            else
              {
                 if ((srs->alpha) || (a != 0xff))
                   trs = _xr_render_surface_new(srs->xcbinf, sw + 1, sh + 1,
                                                srs->xcbinf->fmt32, 1);
                 /* trs = _xr_render_surface_new(srs->xcbinf, sw, sh,
                                                srs->xcbinf->fmt32, 1); */
                 else
                   trs = _xr_render_surface_new(srs->xcbinf, sw + 1, sh + 1,
                                                srs->fmt, srs->alpha);
                 /* trs = _xr_render_surface_new(srs->xcbinf, sw, sh,
                                                srs->fmt, srs->alpha); */
                 xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                    sx, sy, 0, 0, 0, 0, sw, sh);
		  /* fill right and bottom pixel so interpolation works right */
                 xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                    sx + sw, sy, 0, 0, sw, 0, 1, sh);
                 xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                    sx, sy + sh, 0, 0, 0, sh, sw + 1, 1);
                 xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, trs->pic,
                                    sx + sw, sy, 0, 0, sw, 0, 1, sh);
                 mask.xid = 0;
              }
         }
     }

   /*
   sf = MAX(sw, sh);
#define BMAX 26
   if      (sf <= 8    ) sf = 1 << (BMAX - 3);
   else if (sf <= 16   ) sf = 1 << (BMAX - 4);
   else if (sf <= 32   ) sf = 1 << (BMAX - 5);
   else if (sf <= 64   ) sf = 1 << (BMAX - 6);
   else if (sf <= 128  ) sf = 1 << (BMAX - 7);
   else if (sf <= 256  ) sf = 1 << (BMAX - 8);
   else if (sf <= 512  ) sf = 1 << (BMAX - 9);
   else if (sf <= 1024 ) sf = 1 << (BMAX - 10);
   else if (sf <= 2048 ) sf = 1 << (BMAX - 11);
   else if (sf <= 4096 ) sf = 1 << (BMAX - 12);
   else if (sf <= 8192 ) sf = 1 << (BMAX - 13);
   else if (sf <= 16384) sf = 1 << (BMAX - 14);
   else                  sf = 1 << (BMAX - 15);
   */

   init_transform(&xf);

   /* xf.matrix11 = (sf * sw) / w; */
   xf.matrix11 = DOUBLE_TO_FIXED((double) sw / (double) w);

   /* xf.matrix22 = (sf * sh) / h; */
   xf.matrix22 = DOUBLE_TO_FIXED((double) sh / (double) h);

   _xr_render_surface_clips_set(drs, dc, x, y, w, h);
   if (trs)
     {
	if (is_scaling)
	  set_filter(trs, smooth);

       xcb_render_set_picture_transform(trs->xcbinf->conn, trs->pic, xf);

       xcb_render_composite(srs->xcbinf->conn, op, trs->pic, mask, drs->pic,
                          0, 0, 0, 0, x, y, w, h);
       _xr_render_surface_free(trs);
     }
   else
     {
	if (is_scaling)
	  set_filter(srs, smooth);

       xcb_render_set_picture_transform(srs->xcbinf->conn, srs->pic, xf);

       xcb_render_composite(srs->xcbinf->conn, op, srs->pic, mask, drs->pic,
                          /* (sx * w) / sw,
                             (sy * h) / sh, */
                          ((sx * w) + (sw / 2)) / sw,
                          ((sy * h) + (sh / 2)) / sh,
                          0, 0, x, y, w, h);
      }
}

void
_xr_render_surface_copy(Xcb_Render_Surface *srs, Xcb_Render_Surface *drs, int sx, int sy, int x, int y, int w, int h)
{
   xcb_render_transform_t xf;
   xcb_render_picture_t   mask = { 0 };
   uint32_t               value_mask;
   uint32_t               value_list[1];

   if ((w < 1) || (h < 1) || (!srs) || (!drs)) return;

#ifdef BROKEN_XORG_XRENDER
   /* FIXME: why do we need to change the identity matrix if the src surface
    *        is 1 bit deep?
    */
   if (srs->depth == 1)
     {
	init_transform(&xf);
	xf.matrix11 = xf.matrix22 = xf.matrix33 = 1;
	xcb_render_set_picture_transform(srs->xcbinf->conn, srs->pic, xf);
     }
#endif

   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list[0] = 0;
   xcb_render_change_picture(srs->xcbinf->conn, srs->pic, value_mask, value_list);
   xcb_render_change_picture(srs->xcbinf->conn, drs->pic, value_mask, value_list);

   xcb_render_composite(srs->xcbinf->conn, XCB_RENDER_PICT_OP_SRC, srs->pic, mask, drs->pic,
                      sx, sy, 0, 0, x, y, w, h);
}

void
_xr_render_surface_rectangle_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   xcb_render_color_t col;
   xcb_rectangle_t    rect;
   uint32_t           value_mask;
   uint32_t           value_list[1];
   int                r, g, b, a, aa, op;

   if ((w < 1) || (h < 1) || (!rs) || (!dc)) return;
   a = (dc->col.col >> 24) & 0xff;
   if (a == 0) return;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   aa = a + 1;
   r = (r * aa) >> 8;
   g = (g * aa) >> 8;
   b = (b * aa) >> 8;
   col.red   = (r << 8) | r;
   col.green = (g << 8) | g;
   col.blue  = (b << 8) | b;
   col.alpha = (a << 8) | a;
   op = XCB_RENDER_PICT_OP_SRC;
   if (a < 0xff) op = XCB_RENDER_PICT_OP_OVER;
   value_mask = XCB_RENDER_CP_CLIP_MASK;
   value_list[0] = 0;
   xcb_render_change_picture(rs->xcbinf->conn, rs->pic, value_mask, value_list);

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
/*    num = 0; */
/*    for (pt = points; pt; pt = (RGBA_Polygon_Point *)(((Evas_Object_List *)pt)->next)) num++; */
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
/*    for (pt = points; pt; pt = (RGBA_Polygon_Point *)(((Evas_Object_List *)pt)->next)) */
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
