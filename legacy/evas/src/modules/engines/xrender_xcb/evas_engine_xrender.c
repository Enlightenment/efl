#include "evas_common.h"
#include "evas_macros.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_Xcb.h"
#include <math.h>


XCBRenderPICTFORMINFO *
XCBRenderFindVisualFormat (XCBConnection *c, XCBVISUALID visual)
{
  XCBRenderQueryPictFormatsCookie cookie;
  XCBRenderQueryPictFormatsRep   *rep;
  XCBRenderPICTSCREENIter         screen_iter;
  XCBRenderPICTFORMAT             format = { 0 };

  cookie = XCBRenderQueryPictFormats (c);
  rep = XCBRenderQueryPictFormatsReply (c, cookie, NULL);
  if (!rep)
    return NULL;

  screen_iter = XCBRenderQueryPictFormatsScreensIter (rep);
  for (; screen_iter.rem; XCBRenderPICTSCREENNext (&screen_iter)) {
    XCBRenderPICTDEPTHIter depth_iter;

    depth_iter = XCBRenderPICTSCREENDepthsIter (screen_iter.data);
    for (; depth_iter.rem; XCBRenderPICTDEPTHNext (&depth_iter)) {
      XCBRenderPICTVISUALIter visual_iter;

      visual_iter = XCBRenderPICTDEPTHVisualsIter (depth_iter.data);
      for (; visual_iter.rem; XCBRenderPICTVISUALNext (&visual_iter)) {
        if (visual.id == visual_iter.data->visual.id) {
          format = visual_iter.data->format;
        }
      }
    }
  }
  if (format.xid != 0) {
    XCBRenderPICTFORMINFOIter forminfo_iter;

    forminfo_iter = XCBRenderQueryPictFormatsFormatsIter (rep);
    for (; forminfo_iter.rem; XCBRenderPICTFORMINFONext (&forminfo_iter)) {
      if (forminfo_iter.data->id.xid == format.xid) {
        XCBRenderPICTFORMINFO *forminfo;

        forminfo = (XCBRenderPICTFORMINFO *)malloc (sizeof (XCBRenderPICTFORMINFO));
        memcpy (forminfo, forminfo_iter.data, sizeof (XCBRenderPICTFORMINFO));
        free (rep);

        return forminfo;
      }
    }
  }
  free (rep);

  return NULL;
}

XCBrender_Surface *
_xr_render_surface_new(XCBimage_Info *xcbinf, int w, int h, XCBRenderPICTFORMINFO *fmt, int alpha)
{
   XCBrender_Surface *rs;
   CARD32             mask;
   CARD32             values[3];

   rs = calloc(1, sizeof(XCBrender_Surface));
   if (!rs) return NULL;
   rs->xcbinf = xcbinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = fmt;
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   rs->allocated = 1;
   rs->draw.pixmap = XCBPIXMAPNew(xcbinf->conn);
   XCBCreatePixmap(xcbinf->conn, fmt->depth, rs->draw.pixmap, xcbinf->root, w, h);
   rs->xcbinf->references++;
   mask = XCBRenderCPRepeat | XCBRenderCPDither | XCBRenderCPComponentAlpha;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = XCBRenderPICTURENew(xcbinf->conn);
   XCBRenderCreatePicture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);

   return rs;
}

XCBrender_Surface *
_xr_render_surface_adopt(XCBimage_Info *xcbinf, XCBDRAWABLE draw, int w, int h, int alpha)
{
   XCBrender_Surface     *rs;
   XCBRenderPICTFORMINFO *fmt;
   CARD32                 mask;
   CARD32                 values[3];

   fmt = XCBRenderFindVisualFormat(xcbinf->conn, xcbinf->vis);
   if (!fmt) return NULL;
   rs = calloc(1, sizeof(XCBrender_Surface));
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
   mask = XCBRenderCPRepeat | XCBRenderCPDither | XCBRenderCPComponentAlpha;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = XCBRenderPICTURENew(xcbinf->conn);
   XCBRenderCreatePicture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);

   return rs;
}

XCBrender_Surface *
_xr_render_surface_format_adopt(XCBimage_Info *xcbinf, XCBDRAWABLE draw, int w, int h, XCBRenderPICTFORMINFO *fmt, int alpha)
{
   XCBrender_Surface *rs;
   CARD32 mask;
   CARD32 values[3];

   rs = calloc(1, sizeof(XCBrender_Surface));
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
   mask = XCBRenderCPRepeat | XCBRenderCPDither | XCBRenderCPComponentAlpha;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   rs->pic = XCBRenderPICTURENew(xcbinf->conn);
   XCBRenderCreatePicture(xcbinf->conn, rs->pic, rs->draw, fmt->id, mask, values);

   return rs;
}

void
_xr_render_surface_free(XCBrender_Surface *rs)
{
   if (rs->allocated) XCBFreePixmap(rs->xcbinf->conn, rs->draw.pixmap);
   XCBRenderFreePicture(rs->xcbinf->conn, rs->pic);
   _xr_image_info_free(rs->xcbinf);
   rs->xcbinf = NULL;
   free(rs);
}

void
_xr_render_surface_repeat_set(XCBrender_Surface *rs, int repeat)
{
   CARD32 mask;
   CARD32 value[1];

   mask = XCBRenderCPRepeat;
   value[0] = repeat;
   XCBRenderChangePicture(rs->xcbinf->conn, rs->pic, mask, value);
}

void
_xr_render_surface_solid_rectangle_set(XCBrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h)
{
   XCBRenderCOLOR col;
   XCBRECTANGLE   rect;
   int            aa;

   aa = a +1;
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
   XCBRenderFillRectangles(rs->xcbinf->conn, XCBRenderPictOpSrc, rs->pic, col, 1, &rect);
}

void
_xr_render_surface_argb_pixels_fill(XCBrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h)
{
   XCBimage_Image  *xcbim;
   unsigned int  *p, *sp, *sple, *spe;
   unsigned int   jump, sjump;
   unsigned int   a, r, g, b, aa;

   xcbim = _xr_image_new(rs->xcbinf, w, h, rs->depth);
   if (!xcbim) return;
   p = (unsigned int *)xcbim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   jump = ((xcbim->line_bytes / 4) - w);
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if
#ifdef WORDS_BIGENDIAN
     (xcbim->xcbim->image_byte_order == XCBImageOrderLSBFirst)
#else
     (xcbim->xcbim->image_byte_order == XCBImageOrderMSBFirst)
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
		  switch (a = A_VAL(sp))
		  {
		    case 0:
			*p = 0;
		    break;
		    case 255:
			*p = *sp;
		    break;
		    default:
			aa = a + 1;
			r = ((R_VAL(sp)) * aa) >> 8;
			g = ((G_VAL(sp)) * aa) >> 8;
			b = ((B_VAL(sp)) * aa) >> 8;
			*p = (a << 24) | (r << 16) | (g << 8) | b;
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
_xr_render_surface_rgb_pixels_fill(XCBrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h)
{
   XCBimage_Image  *xcbim;
   unsigned int  *p, *sp, *sple, *spe;
   unsigned int   jump, sjump;

   xcbim = _xr_image_new(rs->xcbinf, w, h, rs->depth);
   if (!xcbim) return;
   p = (unsigned int *)xcbim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   jump = ((xcbim->line_bytes / 4) - w);
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if
#ifdef WORDS_BIGENDIAN
     (xcbim->xcbim->image_byte_order == XCBImageOrderLSBFirst)
#else
     (xcbim->xcbim->image_byte_order == XCBImageOrderMSBFirst)
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
		  *p = 0xff000000 | ((R_VAL(sp)) << 16) | ((G_VAL(sp)) << 8) | (B_VAL(sp));
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
_xr_render_surface_clips_set(XCBrender_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh)
{
   int num = 0;
   XCBRECTANGLE *rect = NULL;

   if ((dc) && (dc->clip.use))
     {
	RECTS_CLIP_TO_RECT(rx, ry, rw, rh,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
     }
   if ((!dc) || (!dc->cutout.rects))
     {
	rect = malloc(sizeof(XCBRECTANGLE));
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
	rect = malloc(num * sizeof(XCBRECTANGLE));
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
   XCBRenderSetPictureClipRectangles(rs->xcbinf->conn, rs->pic, 0, 0, num, rect);
   free(rect);
}

// when color multiplier is used want: instead
// CA src IN mask SRC temp; non-CA temp OVER dst. - i think. need to check.
void
_xr_render_surface_composite(XCBrender_Surface *srs, XCBrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth)
{
   XCBrender_Surface *trs = NULL;
   XCBRenderTRANSFORM xf;
   XCBRenderPICTURE   mask;
   CARD32             value_mask;
   CARD32             value_list[1];
   int                r, g, b, a;
   int                sf;
   int                op;

   if ((sw <= 0) || (sh <= 0) || (w <= 0) || (h <= 0)) return;
   value_mask = XCBRenderCPClipMask;
   value_list[0] = 0;
   XCBRenderChangePicture(srs->xcbinf->conn, srs->pic, value_mask, value_list);
   XCBRenderChangePicture(srs->xcbinf->conn, drs->pic, value_mask, value_list);

   op = XCBRenderPictOpSrc;
   if (srs->alpha) op = XCBRenderPictOpOver;
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
            op = XCBRenderPictOpOver;
            value_mask = XCBRenderCPComponentAlpha;
            value_list[0] = 1;
            mask = srs->xcbinf->mul->pic;
            XCBRenderChangePicture(srs->xcbinf->conn, mask, value_mask, value_list);
            if ((r == 0xff) && (g == 0xff) && (b == 0xff) && (a != 0xff))
              {
              }
            else
              {
                 xf.matrix11 = 1;
                 xf.matrix12 = 0;
                 xf.matrix13 = 0;

                 xf.matrix21 = 0;
                 xf.matrix22 = 1;
                 xf.matrix23 = 0;

                 xf.matrix31 = 0;
                 xf.matrix32 = 0;
                 xf.matrix33 = 1;
                 if ((srs->alpha) || (a != 0xff))
                   trs = _xr_render_surface_new(srs->xcbinf, sw + 1, sh + 1,
                                                srs->xcbinf->fmt32, 1);
                 else
                   trs = _xr_render_surface_new(srs->xcbinf, sw + 1, sh + 1,
                                                srs->fmt, srs->alpha);
                 XCBRenderSetPictureTransform(srs->xcbinf->conn, srs->pic, xf);
                 XCBRenderComposite(srs->xcbinf->conn, XCBRenderPictOpSrc, srs->pic, mask, trs->pic,
                                    sx, sy, 0, 0, 0, 0, sw, sh);
		  /* fill right and bottom pixel so interpolation works right */
                 XCBRenderComposite(srs->xcbinf->conn, XCBRenderPictOpSrc, srs->pic, mask, trs->pic,
                                    sx + sw, sy, 0, 0, sw, 0, 1, sh);
                 XCBRenderComposite(srs->xcbinf->conn, XCBRenderPictOpSrc, srs->pic, mask, trs->pic,
                                    sx, sy + sh, 0, 0, 0, sh, sw + 1, 1);
                 XCBRenderComposite(srs->xcbinf->conn, XCBRenderPictOpSrc, srs->pic, mask, trs->pic,
                                    sx + sw, sy, 0, 0, sw, 0, 1, sh);
                 mask.xid = 0;
              }
         }
     }


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

   xf.matrix11 = (sf * sw) / w;
   xf.matrix12 = 0;
   xf.matrix13 = 0;

   xf.matrix21 = 0;
   xf.matrix22 = (sf * sh) / h;
   xf.matrix23 = 0;

   xf.matrix31 = 0;
   xf.matrix32 = 0;
   xf.matrix33 = sf;

   _xr_render_surface_clips_set(drs, dc, x, y, w, h);
   if (trs)
     {
       if (smooth)
         XCBRenderSetPictureFilter (trs->xcbinf->conn, trs->pic, strlen ("best"), "best", 0, NULL);
       else
         XCBRenderSetPictureFilter (trs->xcbinf->conn, trs->pic, strlen ("nearest"), "nearest", 0, NULL);
       XCBRenderSetPictureTransform(trs->xcbinf->conn, trs->pic, xf);

       XCBRenderComposite(srs->xcbinf->conn, op, trs->pic, mask, drs->pic,
                          0, 0, 0, 0, x, y, w, h);
       _xr_render_surface_free(trs);
     }
   else
     {
       if (smooth)
         XCBRenderSetPictureFilter (srs->xcbinf->conn, srs->pic, strlen ("best"), "best", 0, NULL);
       else
         XCBRenderSetPictureFilter (srs->xcbinf->conn, srs->pic, strlen ("nearest"), "nearest", 0, NULL);
       XCBRenderSetPictureTransform(srs->xcbinf->conn, srs->pic, xf);

       XCBRenderComposite(srs->xcbinf->conn, op, srs->pic, mask, drs->pic,
                          ((sx * w) + (sw / 2)) / sw,
                          ((sy * h) + (sh / 2)) / sh,
                          0, 0, x, y, w, h);
      }
}

void
_xr_render_surface_copy(XCBrender_Surface *srs, XCBrender_Surface *drs, int sx, int sy, int x, int y, int w, int h)
{
   XCBRenderTRANSFORM xf;
   XCBRenderPICTURE   mask = { 0 };
   CARD32             value_mask;
   CARD32             value_list[1];

   if ((w <= 0) || (h <= 0) || (!srs) || (!drs)) return;
   xf.matrix11 = 1;
   xf.matrix12 = 0;
   xf.matrix13 = 0;

   xf.matrix21 = 0;
   xf.matrix22 = 1;
   xf.matrix23 = 0;

   xf.matrix31 = 0;
   xf.matrix32 = 0;
   xf.matrix33 = 1;

   XCBRenderSetPictureTransform(srs->xcbinf->conn, srs->pic, xf);
   value_mask = XCBRenderCPClipMask;
   value_list[0] = 0;
   XCBRenderChangePicture(srs->xcbinf->conn, srs->pic, value_mask, value_list);
   XCBRenderChangePicture(srs->xcbinf->conn, drs->pic, value_mask, value_list);
   XCBRenderSetPictureFilter(srs->xcbinf->conn, srs->pic, strlen("nearest"), "nearest", 0, NULL);

   XCBRenderComposite(srs->xcbinf->conn, XCBRenderPictOpSrc, srs->pic, mask, drs->pic,
                      sx, sy, 0, 0, x, y, w, h);
}

void
_xr_render_surface_rectangle_draw(XCBrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   XCBRenderCOLOR col;
   XCBRECTANGLE   rect;
   CARD32         value_mask;
   CARD32         value_list[1];
   int            r, g, b, a, aa, op;

   if ((w <= 0) || (h <= 0)) return;
   a = (dc->col.col >> 24) & 0xff;
   if (a == 0) return;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   aa = a +1;
   r = (r * aa) >> 8;
   g = (g * aa) >> 8;
   b = (b * aa) >> 8;
   col.red   = (r << 8) | r;
   col.green = (g << 8) | g;
   col.blue  = (b << 8) | b;
   col.alpha = (a << 8) | a;
   op = XCBRenderPictOpSrc;
   if (a < 0xff) op = XCBRenderPictOpOver;
   value_mask = XCBRenderCPClipMask;
   value_list[0] = 0;
   XCBRenderChangePicture(rs->xcbinf->conn, rs->pic, value_mask, value_list);

   _xr_render_surface_clips_set(rs, dc, x, y, w, h);
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XCBRenderFillRectangles(rs->xcbinf->conn, op, rs->pic, col, 1, &rect);
}

void
_xr_render_surface_line_draw(XCBrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2)
{
/*    CARD32         value_mask; */
/*    CARD32         value_list[1]; */
/*    int            op; */

/*    op = XCBRenderPictOpSrc; */
/*    value_mask = XCBRenderCPClipMask; */
/*    value_list[0] = 0; */
/*    XCBRenderChangePicture(rs->xcbinf->conn, rs->pic, value_mask, value_list); */
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
/*	if (a < 0xff) op = XCBRenderPictOpOver; */
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
_xre_poly_draw(XCBrender_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points)
{
/*    RGBA_Polygon_Point *pt; */
/*    int i, num; */
/*    XPointDouble *pts; */
/*    int r, g, b, a; */
/*    CARD32         value_mask; */
/*    CARD32         value_list[1]; */
/*    int op; */

/*    op = XCBRenderPictOpSrc; */
/*    num = 0; */
/*    for (pt = points; pt; pt = (RGBA_Polygon_Point *)(((Evas_Object_List *)pt)->next)) num++; */
/*    if (num < 3) return; */
/*    a = (dc->col.col >> 24) & 0xff; */
/*    if (a == 0) return; */
/*    if (a < 0xff) op = XCBRenderPictOpOver; */
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
/*    value_mask = XCBRenderCPClipMask; */
/*    value_list[0] = 0; */
/*    XCBRenderChangePicture(rs->xcbinf->conn, rs->pic, value_mask, value_list); */

/*    _xr_render_surface_clips_set(rs, dc, 0, 0, rs->w, rs->h); */
/*    XRenderCompositeDoublePoly(rs->xcbinf->conn, op, */
/*			      rs->xcbinf->mul->pic, rs->pic,  */
/*			      rs->xcbinf->fmt8, 0, 0, 0, 0, */
/*			      pts, num, Complex); */
/*    free(pts); */
}
