/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "evas_common.h"
#include "evas_macros.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"
#include <math.h>

/* this is a work around broken xrender - when/if this ever gets fixed in xorg
 * we can comment this out and one day remove it - for now keep it until such
 * a fix is spotted in the wild
 */
#define BROKEN_XORG_XRENDER 1

Xrender_Surface *
_xr_render_surface_new(Ximage_Info *xinf, int w, int h, XRenderPictFormat *fmt, int alpha)
{
   Xrender_Surface *rs;
   XRenderPictureAttributes att;
   
   if ((!xinf) || (!fmt) || (w < 1) || (h < 1)) return NULL;
   rs = calloc(1, sizeof(Xrender_Surface));
   if (!rs) return NULL;
   rs->xinf = xinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = fmt;
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   rs->allocated = 1;
   rs->draw = XCreatePixmap(xinf->disp, xinf->root, w, h, fmt->depth);
   if (rs->draw == None)
     {
	free(rs);
	return NULL;
     }
   rs->xinf->references++;
   att.dither = 0;
   att.component_alpha = 0;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, CPRepeat | CPDither | CPComponentAlpha, &att);
   if (rs->pic == None)
     {
	XFreePixmap(rs->xinf->disp, rs->draw);
	rs->xinf->references--;
        free(rs);
	return NULL;
     }
   return rs;
}

Xrender_Surface *
_xr_render_surface_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, int alpha)
{
   Xrender_Surface *rs;
   XRenderPictFormat *fmt;
   XRenderPictureAttributes att;
   
   if ((!xinf) || (draw == None) || (w < 1) || (h < 1)) return NULL;
   fmt = XRenderFindVisualFormat(xinf->disp, xinf->vis);
   if (!fmt) return NULL;
   rs = calloc(1, sizeof(Xrender_Surface));
   if (!rs) return NULL;
   rs->xinf = xinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = fmt;
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   if (fmt->depth == 32) rs->alpha = 1;
   rs->allocated = 0;
   rs->draw = draw;
   rs->xinf->references++;
   att.dither = 0;
   att.component_alpha = 0;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, CPRepeat | CPDither | CPComponentAlpha, &att);
   if (rs->pic == None)
     {
	rs->xinf->references--;
        free(rs);
	return NULL;
     }
   return rs;
}

Xrender_Surface *
_xr_render_surface_format_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, XRenderPictFormat *fmt, int alpha)
{
   Xrender_Surface *rs;
   XRenderPictureAttributes att;
   
   if ((!xinf) || (!fmt) || (draw == None) || (w < 1) || (h < 1)) return NULL;
   rs = calloc(1, sizeof(Xrender_Surface));
   if (!rs) return NULL;
   rs->xinf = xinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = fmt;
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   if (fmt->depth == 32) rs->alpha = 1;
   rs->xinf->references++;
   rs->allocated = 0;
   rs->draw = draw;
   att.dither = 0;
   att.component_alpha = 0;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, CPRepeat | CPDither | CPComponentAlpha, &att);
   if (rs->pic == None)
     {
	rs->xinf->references--;
        free(rs);
	return NULL;
     }
   return rs;
}

void
_xr_render_surface_free(Xrender_Surface *rs)
{
   if (!rs) return;
   if (rs->xinf)
     {
	if ((rs->allocated) && (rs->draw != None))
	   XFreePixmap(rs->xinf->disp, rs->draw);
	if (rs->pic != None)
	   XRenderFreePicture(rs->xinf->disp, rs->pic);
	_xr_image_info_free(rs->xinf);
	rs->xinf = NULL;
     }
   free(rs);
}

void
_xr_render_surface_repeat_set(Xrender_Surface *rs, int repeat)
{
   XRenderPictureAttributes att;
   
   att.repeat = repeat;
   XRenderChangePicture(rs->xinf->disp, rs->pic, CPRepeat, &att);
}

void
_xr_render_surface_solid_rectangle_set(Xrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h)
{
   XRenderColor col;
   int aa;
   
   aa = a + 1;
   r = (r * aa) >> 8;
   g = (g * aa) >> 8;
   b = (b * aa) >> 8;
   col.red   = (r << 8) | r;
   col.green = (g << 8) | g;
   col.blue  = (b << 8) | b;
   col.alpha = (a << 8) | a;
   XRenderFillRectangle(rs->xinf->disp, PictOpSrc, rs->pic, &col, x, y, w, h);
}

void
_xr_render_surface_argb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h)
{
   Ximage_Image  *xim;
   unsigned int  *p, *sp, *sple, *spe;
   unsigned int   jump, sjump;
   unsigned int   a, r, g, b, aa;
   
   xim = _xr_image_new(rs->xinf, w, h, rs->depth);
   if (!xim) return;
   p = (unsigned int *)xim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   jump = ((xim->line_bytes / 4) - w);
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if
#ifdef WORDS_BIGENDIAN
     (xim->xim->byte_order == LSBFirst)
#else
     (xim->xim->byte_order == MSBFirst)
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
   _xr_image_put(xim, rs->draw, x, y, w, h);
}

void
_xr_render_surface_rgb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h)
{
   Ximage_Image  *xim;
   unsigned int  *p, *sp, *sple, *spe;
   unsigned int   jump, sjump;
   
   xim = _xr_image_new(rs->xinf, w, h, rs->depth);
   if (!xim) return;
   p = (unsigned int *)xim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   jump = ((xim->line_bytes / 4) - w);
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if
#ifdef WORDS_BIGENDIAN
     (xim->xim->byte_order == LSBFirst)
#else
     (xim->xim->byte_order == MSBFirst)
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
   _xr_image_put(xim, rs->draw, x, y, w, h);
}

void
_xr_render_surface_clips_set(Xrender_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh)
{
   int num = 0;
   XRectangle *rect = NULL;
   
   if ((dc) && (dc->clip.use))
     {
	RECTS_CLIP_TO_RECT(rx, ry, rw, rh,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
     }
   if ((!dc) || (!dc->cutout.rects))
     {
	rect = malloc(sizeof(XRectangle));
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
	rect = malloc(num * sizeof(XRectangle));
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
   XRenderSetPictureClipRectangles(rs->xinf->disp, rs->pic, 0, 0, rect, num);
   free(rect);
}

/* initialized the transform to the identity */
static void init_transform (XTransform *t)
{
   int i, j;

   for (i = 0; i < 3; i++)
     for (j = 0; j < 3; j++)
       t->matrix[i][j] = XDoubleToFixed((i == j) ? 1 : 0);
}


// when color multiplier is used want: instead
// CA src IN mask SRC temp; non-CA temp OVER dst. - i think. need to check.
void
_xr_render_surface_composite(Xrender_Surface *srs, Xrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth)
{
   Xrender_Surface *trs = NULL;
   XTransform xf;
   XRenderPictureAttributes att;
   Picture mask;
   int r, g, b, a, op;
   int e;

   if ((sw <= 0) || (sh <= 0) || (w <= 0) || (h <= 0)) return;
   
   e = 0;
   if ((sw != w) || (sh != h))
      e = 1;

   att.clip_mask = None;
   XRenderChangePicture(srs->xinf->disp, srs->pic, CPClipMask, &att);
   XRenderChangePicture(srs->xinf->disp, drs->pic, CPClipMask, &att);
   
   op = PictOpSrc;
   if (srs->alpha) op = PictOpOver;
   mask = None;
   if ((dc) && (dc->mul.use))
     {
	r = (int)(R_VAL(&dc->mul.col));
	g = (int)(G_VAL(&dc->mul.col));
	b = (int)(B_VAL(&dc->mul.col));
	a = (int)(A_VAL(&dc->mul.col));
	if ((r != 0xff) || (g != 0xff) || (b != 0xff) || (a != 0xff))
	  {
	     if ((srs->xinf->mul_r != r) || (srs->xinf->mul_g != g) ||
		 (srs->xinf->mul_b != b) || (srs->xinf->mul_a != a))
	       {
		  srs->xinf->mul_r = r;
		  srs->xinf->mul_g = g;
		  srs->xinf->mul_b = b;
		  srs->xinf->mul_a = a;
		  _xr_render_surface_solid_rectangle_set(srs->xinf->mul, 
							 r, g, b, a, 
							 0, 0, 1, 1);
	       }
	     op = PictOpOver;
	     mask = srs->xinf->mul->pic;
	     if ((r == 0xff) && (g == 0xff) && (b == 0xff) && (a != 0xff))
	       {
		 att.component_alpha = 0;
		 XRenderChangePicture(srs->xinf->disp, mask, CPComponentAlpha, &att);
	       }
	     else
	       {
		  if ((srs->alpha) || (a != 0xff))
		    trs = _xr_render_surface_new(srs->xinf, sw + e, sh + e,
						 srs->xinf->fmt32, 1);
		  else
		    trs = _xr_render_surface_new(srs->xinf, sw + e, sh + e,
						 srs->fmt, srs->alpha);
		  if (!trs) return;

		  att.component_alpha = 1;
		  XRenderChangePicture(srs->xinf->disp, mask, CPComponentAlpha, &att);
		  XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, mask,
				   trs->pic, sx, sy, 0, 0, 0, 0, sw, sh);
		  /* fill right and bottom pixel so interpolation works right */
		  if (e)
		    {
		      XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, mask,
				       trs->pic, sx + sw - 1, sy, 0, 0, sw, 0, 1, sh);
		      XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, mask,
				       trs->pic, sx, sy + sh - 1, 0, 0, 0, sh, sw, 1);
		      XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, mask,
				       trs->pic, sx + sw - 1, sy + sh - 1, 0, 0, sw, sh, 1, 1);
		    }
		  mask = None;
	       }
	  }
     }

   init_transform(&xf);
   xf.matrix[0][0] = XDoubleToFixed((double) sw / (double) w);
   xf.matrix[1][1] = XDoubleToFixed((double) sh / (double) h);

   _xr_render_surface_clips_set(drs, dc, x, y, w, h);
   if (trs)
     {
	if (smooth)
	  XRenderSetPictureFilter(trs->xinf->disp, trs->pic, FilterBest, NULL, 0);
	else 
	  XRenderSetPictureFilter(trs->xinf->disp, trs->pic, FilterNearest, NULL, 0);
	XRenderSetPictureTransform(trs->xinf->disp, trs->pic, &xf);
	
	XRenderComposite(trs->xinf->disp, op, trs->pic, mask, drs->pic,
			 0, 0, 0, 0, x, y, w, h);
	_xr_render_surface_free(trs);
     }
   else
     {
	if (srs->bordered && e)
	  {
	    trs = _xr_render_surface_new(srs->xinf, sw + 1, sh + 1,
					 srs->fmt, srs->alpha);
	    if (!trs) return;

	    XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None,
			     trs->pic, sx, sy, 0, 0, 0, 0, sw, sh);
	    XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None,
			     trs->pic, sx + sw - 1, sy, 0, 0, sw, 0, 1, sh);
	    XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None,
			     trs->pic, sx, sy + sh - 1, 0, 0, 0, sh, sw, 1);
	    XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None,
			     trs->pic, sx + sw - 1, sy + sh - 1, 0, 0, sw, sh, 1, 1);

	    if (smooth)
		XRenderSetPictureFilter(trs->xinf->disp, trs->pic, FilterBest, NULL, 0);
	    else 
		XRenderSetPictureFilter(trs->xinf->disp, trs->pic, FilterNearest, NULL, 0);

	    XRenderSetPictureTransform(trs->xinf->disp, trs->pic, &xf);
	    XRenderComposite(trs->xinf->disp, op, trs->pic, mask, drs->pic,
			     0, 0, 0, 0, x, y, w, h);
	    _xr_render_surface_free(trs);
	  }
	else
	  {
	    if (smooth)
		XRenderSetPictureFilter(srs->xinf->disp, srs->pic, FilterBest, NULL, 0);
	    else 
		XRenderSetPictureFilter(srs->xinf->disp, srs->pic, FilterNearest, NULL, 0);

	    XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
	    XRenderComposite(srs->xinf->disp, op, srs->pic, mask, drs->pic,
			     ((sx * w) + (sw / 2)) / sw, 
			     ((sy * h) + (sh / 2)) / sh,
			     0, 0, x, y, w, h);
	  }
     }
}

void
_xr_render_surface_copy(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h)
{
   XTransform xf;
   XRenderPictureAttributes att;
   
   if ((w <= 0) || (h <= 0) || (!srs) || (!drs)) return;

#ifdef BROKEN_XORG_XRENDER   
   /* FIXME: why do we need to change the identity matrix ifthe src surface
    * is 1 bit deep?
    */
   if (srs->depth == 1)
     {
	init_transform(&xf);
	xf.matrix[0][0] = xf.matrix[1][1] = xf.matrix[2][2] = 1;
	XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
     }
#endif
   
   att.clip_mask = None;
   XRenderChangePicture(srs->xinf->disp, srs->pic, CPClipMask, &att);
   XRenderChangePicture(srs->xinf->disp, drs->pic, CPClipMask, &att);
   XRenderSetPictureFilter(srs->xinf->disp, srs->pic, FilterNearest, NULL, 0);
   
   XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None, drs->pic, 
		    sx, sy, 0, 0, x, y, w, h);
}

void
_xr_render_surface_rectangle_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   XRenderColor col;
   XRenderPictureAttributes att;
   int r, g, b, a, aa, op;

   if ((w <= 0) || (h <= 0) || (!rs) || (!dc)) return;
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
   op = PictOpSrc;
   if (a < 0xff) op = PictOpOver;
   att.clip_mask = None;
   XRenderChangePicture(rs->xinf->disp, rs->pic, CPClipMask, &att);

   _xr_render_surface_clips_set(rs, dc, x, y, w, h);
   XRenderFillRectangle(rs->xinf->disp, op, rs->pic, &col, x, y, w, h);
}

void
_xr_render_surface_line_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2)
{
   XRenderPictureAttributes att;
   int op;
   
   if ((!rs) || (!dc)) return;
   op = PictOpOver;
   att.clip_mask = None;
   XRenderChangePicture(rs->xinf->disp, rs->pic, CPClipMask, &att);
   _xr_render_surface_clips_set(rs, dc, 0, 0, rs->w, rs->h);

     {
	int r, g, b, a;
	XPointDouble poly[4];
	int dx, dy;
	double len, ddx, ddy;
	
	dx = x2 - x1;
	dy = y2 - y1;
	len = sqrt((double)(dx * dx) + (double)(dy * dy));
	ddx = (0.5 * dx) / len;
	ddy = (0.5 * dy) / len;
	if (ddx < 0) ddx = -0.5 - ddx;
	else ddx = 0.5 - ddx;
	if (ddy < 0) ddy = -0.5 - ddy;
	else ddy = 0.5 - ddy;
	poly[0].x =  (x1 + ddx);
	poly[0].y =  (y1 - ddy);
	poly[1].x =  (x2 + ddx);
	poly[1].y =  (y2 - ddy);
	poly[2].x =  (x2 - ddx);
	poly[2].y =  (y2 + ddy);
	poly[3].x =  (x1 - ddx);
	poly[3].y =  (y1 + ddy);

	a = (dc->col.col >> 24) & 0xff;
	if (a == 0) return;
	if (a < 0xff) op = PictOpOver;
	r = (dc->col.col >> 16) & 0xff;
	g = (dc->col.col >> 8 ) & 0xff;
	b = (dc->col.col      ) & 0xff;
	if ((rs->xinf->mul_r != r) || (rs->xinf->mul_g != g) ||
	    (rs->xinf->mul_b != b) || (rs->xinf->mul_a != a))
	  {
	     rs->xinf->mul_r = r;
	     rs->xinf->mul_g = g;
	     rs->xinf->mul_b = b;
	     rs->xinf->mul_a = a;
	     _xr_render_surface_solid_rectangle_set(rs->xinf->mul, r, g, b, a, 0, 0, 1, 1);
	  }
	XRenderCompositeDoublePoly(rs->xinf->disp, op,
				   rs->xinf->mul->pic, rs->pic, 
				   rs->xinf->fmt8, 0, 0, 0, 0,
				   poly, 4, EvenOddRule);
     }
}

void
_xre_poly_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points)
{
   RGBA_Polygon_Point *pt;
   int i, num;
   XPointDouble *pts;
   int r, g, b, a;
   XRenderPictureAttributes att;
   int op;
   
   if ((!rs) || (!dc)) return;
//   op = PictOpSrc;
   num = 0;
   for (pt = points; pt; pt = (RGBA_Polygon_Point *)(((Evas_Object_List *)pt)->next)) num++;
   if (num < 3) return;
   a = (dc->col.col >> 24) & 0xff;
   if (a == 0) return;
//   if (a < 0xff)
     op = PictOpOver;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   if ((rs->xinf->mul_r != r) || (rs->xinf->mul_g != g) ||
       (rs->xinf->mul_b != b) || (rs->xinf->mul_a != a))
     {
	rs->xinf->mul_r = r;
	rs->xinf->mul_g = g;
	rs->xinf->mul_b = b;
	rs->xinf->mul_a = a;
	_xr_render_surface_solid_rectangle_set(rs->xinf->mul, r, g, b, a, 0, 0, 1, 1);
     }
   pts = malloc(num * sizeof(XPointDouble));
   if (!pts) return;
   i = 0;
   for (pt = points; pt; pt = (RGBA_Polygon_Point *)(((Evas_Object_List *)pt)->next))
     {
	if (i < num)
	  {
	     pts[i].x = pt->x;
	     pts[i].y = pt->y;
	     i++;
	  }
     }
   att.clip_mask = None;
   XRenderChangePicture(rs->xinf->disp, rs->pic, CPClipMask, &att);

   _xr_render_surface_clips_set(rs, dc, 0, 0, rs->w, rs->h);
   XRenderCompositeDoublePoly(rs->xinf->disp, op,
			      rs->xinf->mul->pic, rs->pic, 
			      rs->xinf->fmt8, 0, 0, 0, 0,
			      pts, num, Complex);
   free(pts);
}
   
