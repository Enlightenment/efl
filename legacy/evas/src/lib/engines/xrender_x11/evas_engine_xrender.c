#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "evas_engine_api_xrender_x11.h"
#include "Evas_Engine_XRender_X11.h"

Xrender_Surface *
_xr_render_surface_new(Ximage_Info *xinf, int w, int h, XRenderPictFormat *fmt, int alpha)
{
   Xrender_Surface *rs;
   XRenderPictureAttributes att;
   
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
   att.dither = 1;
   att.component_alpha = 1;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, CPRepeat | CPDither | CPComponentAlpha, &att);
   return rs;
}

Xrender_Surface *
_xr_render_surface_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, int alpha)
{
   Xrender_Surface *rs;
   XRenderPictFormat *fmt;
   XRenderPictureAttributes att;
   
   rs = calloc(1, sizeof(Xrender_Surface));
   fmt = XRenderFindVisualFormat(xinf->disp, xinf->vis);
   rs->xinf = xinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = fmt;
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   if (fmt->depth == 32) rs->alpha = 1;
   rs->allocated = 0;
   rs->draw = draw;
   att.dither = 1;
   att.component_alpha = 1;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, CPRepeat | CPDither | CPComponentAlpha, &att);
   return rs;
}

Xrender_Surface *
_xr_render_surface_format_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, XRenderPictFormat *fmt, int alpha)
{
   Xrender_Surface *rs;
   XRenderPictureAttributes att;
   
   rs = calloc(1, sizeof(Xrender_Surface));
   rs->xinf = xinf;
   rs->w = w;
   rs->h = h;
   rs->fmt = fmt;
   rs->alpha = alpha;
   rs->depth = fmt->depth;
   if (fmt->depth == 32) rs->alpha = 1;
   rs->allocated = 0;
   rs->draw = draw;
   att.dither = 1;
   att.component_alpha = 1;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, CPRepeat | CPDither | CPComponentAlpha, &att);
   return rs;
}

void
_xr_render_surface_free(Xrender_Surface *rs)
{
   if (rs->allocated) XFreePixmap(rs->xinf->disp, rs->draw);
   XRenderFreePicture(rs->xinf->disp, rs->pic);
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
   
   aa = a +1;
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
		  a = A_VAL(sp);
		  aa = a + 1;
		  r = ((R_VAL(sp)) * aa) >> 8;
		  g = ((G_VAL(sp)) * aa) >> 8;
		  b = ((B_VAL(sp)) * aa) >> 8;
		  *p = (b << 24) | (g << 16) | (r << 8) | a;
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
		  a = A_VAL(sp);
		  aa = a + 1;
		  r = ((R_VAL(sp)) * aa) >> 8;
		  g = ((G_VAL(sp)) * aa) >> 8;
		  b = ((B_VAL(sp)) * aa) >> 8;
		  *p = (a << 24) | (r << 16) | (g << 8) | b;
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
		  *p = 0xff000000 | ((R_VAL(sp)) << 16) | ((G_VAL(sp)) << 8) | (B_VAL(sp));
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
_xr_render_surface_composite(Xrender_Surface *srs, Xrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth)
{
   XTransform xf;
   XRectangle rect;
   XRenderPictureAttributes att;
   Picture mask;
   int r, g, b, a, op;

   if ((sw <= 0) || (sh <= 0) || (w <= 0) || (h <= 0)) return;
   xf.matrix[0][0] = (0x10000 * sw) / w;
   xf.matrix[0][1] = 0;
   xf.matrix[0][2] = 0;

   xf.matrix[1][0] = 0;
   xf.matrix[1][1] = (0x10000 * sh) / h;
   xf.matrix[1][2] = 0;

   xf.matrix[2][0] = 0;
   xf.matrix[2][1] = 0;
   xf.matrix[2][2] = 0x10000;

   op = PictOpSrc;
   if (srs->alpha) op = PictOpOver;
   mask = None;
   if ((dc) && (dc->mul.use))
     {
	r = (int)(R_VAL(&dc->mul.col));
	g = (int)(G_VAL(&dc->mul.col));
	b = (int)(B_VAL(&dc->mul.col));
	a = (int)(A_VAL(&dc->mul.col));
	if (!(r == g == b == a == 0xff))
	  {
	     if ((srs->xinf->mul_r != r) || (srs->xinf->mul_g != g) ||
		 (srs->xinf->mul_b != b) || (srs->xinf->mul_a != a))
	       {
		  srs->xinf->mul_r = r;
		  srs->xinf->mul_g = g;
		  srs->xinf->mul_b = b;
		  srs->xinf->mul_a = a;
		  _xr_render_surface_solid_rectangle_set(srs->xinf->mul, r, g, b, a, 0, 0, 1, 1);
	       }
	     op = PictOpOver;
	     mask = srs->xinf->mul->pic;
	  }
     }
   
   XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
   att.clip_mask = None;
   XRenderChangePicture(srs->xinf->disp, srs->pic, CPClipMask, &att);
   XRenderChangePicture(srs->xinf->disp, drs->pic, CPClipMask, &att);
   rect.x = x; rect.y = y; rect.width = w; rect.height = h;
   if ((dc) && (dc->clip.use))
     {
	RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.width, rect.height,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
     }
   
   if (smooth) XRenderSetPictureFilter(srs->xinf->disp, srs->pic, "bilinear", NULL, 0);
   else XRenderSetPictureFilter(srs->xinf->disp, srs->pic, "nearest", NULL, 0);

   XRenderSetPictureClipRectangles(srs->xinf->disp, drs->pic, 0, 0, &rect, 1);
   XRenderComposite(srs->xinf->disp, op, srs->pic, mask, drs->pic,
		    ((sx * w) + (sw / 2)) / sw, 
		    ((sy * h) + (sh / 2)) / sh,
		    0, 0, x, y, w, h);
}

void
_xr_render_surface_copy(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h)
{
   XTransform xf;
   XRenderPictureAttributes att;

   if ((w <= 0) || (h <= 0)) return;
   xf.matrix[0][0] = 1;
   xf.matrix[0][1] = 0;
   xf.matrix[0][2] = 0;
   
   xf.matrix[1][0] = 0;
   xf.matrix[1][1] = 1;
   xf.matrix[1][2] = 0;
   
   xf.matrix[2][0] = 0;
   xf.matrix[2][1] = 0;
   xf.matrix[2][2] = 1;
   
   XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
   att.clip_mask = None;
   XRenderChangePicture(srs->xinf->disp, srs->pic, CPClipMask, &att);
   XRenderChangePicture(srs->xinf->disp, drs->pic, CPClipMask, &att);
   XRenderSetPictureFilter(srs->xinf->disp, srs->pic, "nearest", NULL, 0);
   
   XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None, drs->pic, 
		    sx, sy, 0, 0, x, y, w, h);
}

void
_xr_render_surface_rectangle_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   XRenderColor col;
   XRectangle rect;
   XRenderPictureAttributes att;
   int r, g, b, a, aa, op;

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
   op = PictOpSrc;
   if (a < 0xff) op = PictOpOver;
   att.clip_mask = None;
   XRenderChangePicture(rs->xinf->disp, rs->pic, CPClipMask, &att);
   if ((dc) && (dc->clip.use))
     {
	rect.x = dc->clip.x; rect.y = dc->clip.y;
	rect.width = dc->clip.w; rect.height = dc->clip.h;
	XRenderSetPictureClipRectangles(rs->xinf->disp, rs->pic, 0, 0, &rect, 1);
     }
   XRenderFillRectangle(rs->xinf->disp, op, rs->pic, &col, x, y, w, h);
}

void
_xr_render_surface_line_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2)
{
   XRectangle rect;
   XRenderPictureAttributes att;
   int op;
   
   op = PictOpSrc;
   att.clip_mask = None;
   XRenderChangePicture(rs->xinf->disp, rs->pic, CPClipMask, &att);
   if ((dc) && (dc->clip.use))
     {
	rect.x = dc->clip.x; rect.y = dc->clip.y;
	rect.width = dc->clip.w; rect.height = dc->clip.h;
	XRenderSetPictureClipRectangles(rs->xinf->disp, rs->pic, 0, 0, &rect, 1);
     }
     {
	int r, g, b, a;
	XPointDouble poly[4];
	int dx, dy;
	double len, ddx, ddy;
	
	dx = x2 - x1;
	dy = y2 - y1;
	len = sqrt((dx * dx) + (dy * dy));
	ddx = (0.5 * dx) / len;
	ddy = (0.5 * dy) / len;
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
   XRectangle rect;
   XRenderPictureAttributes att;
   int op;
   
   op = PictOpSrc;
   num = 0;
   for (pt = points; pt; pt = (RGBA_Polygon_Point *)(((Evas_Object_List *)pt)->next)) num++;
   if (num < 3) return;
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
   rect.x = 0; rect.y = 0; rect.width = rs->w; rect.height = rs->h;
   att.clip_mask = None;
   XRenderChangePicture(rs->xinf->disp, rs->pic, CPClipMask, &att);
   if ((dc) && (dc->clip.use))
     {
	RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.width, rect.height,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
     }
   XRenderSetPictureClipRectangles(rs->xinf->disp, rs->pic, 0, 0, &rect, 1);
   XRenderCompositeDoublePoly(rs->xinf->disp, op,
			      rs->xinf->mul->pic, rs->pic, 
			      rs->xinf->fmt8, 0, 0, 0, 0,
			      pts, num, Complex);
   free(pts);
}
   
