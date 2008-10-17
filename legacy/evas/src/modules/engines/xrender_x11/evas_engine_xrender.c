/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "evas_common.h"
//#include "evas_macros.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"
#include <math.h>

/* this is a work around broken xrender - when/if this ever gets fixed in xorg
 * we can comment this out and one day remove it - for now keep it until such
 * a fix is spotted in the wild
 */
#define BROKEN_XORG_XRENDER 1

/* should be const char*, but the prototype for XRenderSetPictureFilter
 * is silly
 */
static inline char *
get_filter(int smooth)
{
   return smooth ? FilterBest : FilterNearest;
}

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
   att.dither = 1;
   att.component_alpha = 0;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, 
				  CPRepeat | CPDither | CPComponentAlpha, &att);
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
   att.dither = 1;
   att.component_alpha = 0;
   att.repeat = 0;
   rs->pic = XRenderCreatePicture(xinf->disp, rs->draw, fmt, 
				  CPRepeat | CPDither | CPComponentAlpha, &att);
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
   
   col.red   = (r << 8) | r;
   col.green = (g << 8) | g;
   col.blue  = (b << 8) | b;
   col.alpha = (a << 8) | a;
   XRenderFillRectangle(rs->xinf->disp, PictOpSrc, rs->pic, &col, x, y, w, h);
}

void
_xr_render_surface_argb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h, int ox, int oy)
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
   _xr_image_put(xim, rs->draw, x + ox, y + oy, w, h);
}

void
_xr_render_surface_rgb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h, int ox, int oy)
{
   Ximage_Image  *xim;
   unsigned int  *p, *sp, *sple, *spe;
   unsigned int   jump, sjump;
   
   xim = _xr_image_new(rs->xinf, w, h, rs->depth);
   if (!xim) return;
   p = (unsigned int *)xim->data;
   sp = ((unsigned int *)pixels) + (y * sw) + x;
   sjump = sw - w;
   spe = sp + ((h - 1) * sw) + w;
   if (rs->depth == 16)
     {
	jump = ((xim->line_bytes / 2) - w);
	/* FIXME: if rs->depth == 16 - convert */
	Gfx_Func_Convert conv_func;
	int swap;
	
#ifdef WORDS_BIGENDIAN
	swap = (int)(xim->xim->byte_order == LSBFirst);
#else
	swap = (int)(xim->xim->byte_order == MSBFirst);
#endif
	/* FIXME: swap not handled */
	conv_func = evas_common_convert_func_get(sp, w, h, rs->depth,
						 rs->xinf->vis->red_mask,
						 rs->xinf->vis->green_mask,
						 rs->xinf->vis->blue_mask,
						 PAL_MODE_NONE, 0);
	if (conv_func)
	  conv_func(sp, p, sjump, jump, w, h, x, y, NULL);
     }
   else
     {
	jump = ((xim->line_bytes / 4) - w);
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
   _xr_image_put(xim, rs->draw, x + ox, y + oy, w, h);
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
        Cutout_Rects    *rects;
	Cutout_Rect     *r;
        int i;

        rects = evas_common_draw_context_apply_cutouts(dc);
        num = rects->active;
	rect = malloc(num * sizeof(XRectangle));
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
   XRenderSetPictureClipRectangles(rs->xinf->disp, rs->pic, 0, 0, rect, num);
   free(rect);
}

/* initialized the transform to the identity */
static void
init_xtransform(XTransform *t)
{
   int i, j;

   for (i = 0; i < 3; i++)
     for (j = 0; j < 3; j++)
       t->matrix[i][j] = XDoubleToFixed((i == j) ? 1 : 0);
}

static void
set_xtransform_scale(XTransform *t, int sw, int sh, int w, int h, int tx, int ty)
{
//   if ((sw > 1) && (w > 1))
//     { sw--;  w--; }
//   if ((sh > 1) && (h > 1))
//     { sh--;  h--; }
   t->matrix[0][0] = XDoubleToFixed((double)(sw) / (double)(w));
   t->matrix[1][1] = XDoubleToFixed((double)(sh) / (double)(h));
   t->matrix[2][0] = (tx * sw) / w;
   t->matrix[2][1] = (ty * sh) / h;
}

// when color multiplier is used want: instead
// CA src IN mask SRC temp; non-CA temp OVER dst. - i think. need to check.
void
_xr_render_surface_composite(Xrender_Surface *srs, Xrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth)
{
   Xrender_Surface *trs = NULL;
   XTransform xf;
   XRenderPictureAttributes att;
   Picture mask = None;
   int e, is_scaling, op;

   if ((sw <= 0) || (sh <= 0) || (w <= 0) || (h <= 0)) return;
   
   is_scaling = e = ((sw != w) || (sh != h));

   att.clip_mask = None;
   XRenderChangePicture(srs->xinf->disp, srs->pic, CPClipMask, &att);
   XRenderChangePicture(drs->xinf->disp, drs->pic, CPClipMask, &att);
   
   init_xtransform(&xf);

   op = PictOpOver;
   if (dc->render_op == _EVAS_RENDER_BLEND)
     {
	if (!srs->alpha) op = PictOpSrc;
     }
   else if (dc->render_op == _EVAS_RENDER_BLEND_REL)
     op = PictOpAtop;
   else if (dc->render_op == _EVAS_RENDER_MUL)
     op = PictOpIn;
   else if (dc->render_op == _EVAS_RENDER_COPY)
     op = PictOpSrc;
   else if (dc->render_op == _EVAS_RENDER_COPY_REL)
     op = PictOpIn;
   else if (dc->render_op == _EVAS_RENDER_MASK)
     op = PictOpInReverse;

   if ((dc) && (dc->mul.use))
     {
	int r, g, b, a;

	if ((op == PictOpOver) && (!dc->mul.col)) return;
	a = dc->mul.col >> 24;
	r = (dc->mul.col >> 16) & 0xff;
	g = (dc->mul.col >> 8) & 0xff;
	b = dc->mul.col & 0xff;
	if (a < 255) op = PictOpOver;
	if (dc->mul.col != 0xffffffff)
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
	     mask = srs->xinf->mul->pic;
	     if (dc->mul.col == (a * 0x01010101))
	       {
		 att.component_alpha = 0;
		 XRenderChangePicture(srs->xinf->disp, mask, CPComponentAlpha, &att);
	       }
	     else
	       {
		  if ((srs->alpha) || (a != 0xff))
		    trs = _xr_render_surface_new(srs->xinf, sw + 2, sh + 2,
						 srs->xinf->fmt32, 1);
		  else
		    trs = _xr_render_surface_new(srs->xinf, sw + 2, sh + 2,
						 srs->fmt, srs->alpha);
		  if (!trs) return;
		  
		  att.component_alpha = 1;
		  XRenderChangePicture(srs->xinf->disp, mask, CPComponentAlpha, &att);
		  XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
		  XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, mask,
				   trs->pic, 
				   sx, sy, 
				   sx, sy, 
				   0, 0, sw + 2, sh + 2);
		  mask = None;
	       }
	  }
     }

//#define HFW + (sw / 2)
//#define HFH + (sh / 2)	
#define HFW
#define HFH
   
   _xr_render_surface_clips_set(drs, dc, x, y, w, h);
   if (trs)
     {
	XRenderSetPictureFilter(trs->xinf->disp, trs->pic, get_filter(smooth), NULL, 0);

	set_xtransform_scale(&xf, sw, sh, w, h, -1, -1);
	XRenderSetPictureTransform(trs->xinf->disp, trs->pic, &xf);

	att.component_alpha = 0;
	if (dc->render_op == _EVAS_RENDER_MUL) att.component_alpha = 1;
	XRenderChangePicture(trs->xinf->disp, trs->pic, CPComponentAlpha, &att);

	XRenderComposite(trs->xinf->disp, op, trs->pic, mask, drs->pic,
			 (w HFW) / sw, (h HFH) / sh,
			 (w HFW) / sw, (h HFH) / sh,
			 x, y, w, h);
	_xr_render_surface_free(trs);
     }
   else
     {
	if (srs->bordered && is_scaling)
	  {
	     trs = _xr_render_surface_new(srs->xinf, sw + 2, sh + 2,
					  srs->fmt, srs->alpha);
	     if (!trs) return;
	     
	     att.component_alpha = 0;
	     XRenderChangePicture(srs->xinf->disp, srs->pic, CPComponentAlpha, &att);
	     XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
	     
	     XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None,
			      trs->pic, sx, sy, sx, sy, 0, 0, sw + 2, sh + 2);
	     
	     XRenderSetPictureFilter(trs->xinf->disp, trs->pic, get_filter(smooth), NULL, 0);
	     
	     set_xtransform_scale(&xf, sw, sh, w, h, -1, -1);
	     XRenderSetPictureTransform(trs->xinf->disp, trs->pic, &xf);
	     
	     if (dc->render_op == _EVAS_RENDER_MUL)
	       {
		  att.component_alpha = 1;
		  XRenderChangePicture(trs->xinf->disp, trs->pic, CPComponentAlpha, &att);
	       }
	     
	     XRenderComposite(trs->xinf->disp, op, trs->pic, mask, drs->pic,
			      (w HFW) / sw, (h HFH) / sh,
			      (w HFW) / sw, (h HFH) / sh,
//			      1, 1, 1, 1,
			      x, y, w, h);
	     _xr_render_surface_free(trs);
	  }
	else
	  {
	     XRenderSetPictureFilter(srs->xinf->disp, srs->pic, get_filter(smooth), NULL, 0);
	     
	     set_xtransform_scale(&xf, sw, sh, w, h, 0, 0);
	     XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
	     
	     att.component_alpha = 0;
	     if (dc->render_op == _EVAS_RENDER_MUL)
	       att.component_alpha = 1;
	     XRenderChangePicture(srs->xinf->disp, srs->pic, CPComponentAlpha, &att);
	     
	     XRenderComposite(srs->xinf->disp, op, srs->pic, mask, drs->pic,
			      ((((sx + 1) * w) HFW) / sw),
			      ((((sy + 1) * h) HFH) / sh),
			      ((((sx + 1) * w) HFW) / sw),
			      ((((sy + 1) * h) HFH) / sh),
			      x, y, w, h);
	  }
     }
}

void
_xr_render_surface_copy(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h)
{
   XTransform xf;
   XRenderPictureAttributes att;
   
   if ((w <= 0) || (h <= 0) || (!srs) || (!drs)) return;

   init_xtransform(&xf);
#ifdef BROKEN_XORG_XRENDER   
   /* FIXME: why do we need to change the identity matrix ifthe src surface
    * is 1 bit deep?
    */
   if (srs->depth == 1)
     {
	xf.matrix[0][0] = xf.matrix[1][1] = xf.matrix[2][2] = 1;
     }
#endif
   XRenderSetPictureTransform(srs->xinf->disp, srs->pic, &xf);
//   XRenderSetPictureFilter(srs->xinf->disp, srs->pic, FilterNearest, NULL, 0);
   
   att.clip_mask = None;
   XRenderChangePicture(srs->xinf->disp, srs->pic, CPClipMask, &att);
   XRenderChangePicture(drs->xinf->disp, drs->pic, CPClipMask, &att);
   
   XRenderComposite(srs->xinf->disp, PictOpSrc, srs->pic, None, drs->pic, 
		    sx, sy, 0, 0, x, y, w, h);
}

void
_xr_render_surface_rectangle_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   XRenderColor col;
   XRenderPictureAttributes att;
   int r, g, b, a, op;

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
   op = PictOpOver;
   if (dc->render_op == _EVAS_RENDER_BLEND)
     {
	if (!dc->col.col) return;
	if (a == 0xff) op = PictOpSrc;
     }
   else if (dc->render_op == _EVAS_RENDER_BLEND_REL)
     {
	if (!dc->col.col) return;
	op = PictOpAtop;
     }
   else if (dc->render_op == _EVAS_RENDER_MUL)
     {
	if (dc->col.col == 0xffffffff) return;
	op = PictOpIn;
     }
   else if (dc->render_op == _EVAS_RENDER_COPY)
	op = PictOpSrc;
   else if (dc->render_op == _EVAS_RENDER_COPY_REL)
	op = PictOpIn;
   else if (dc->render_op == _EVAS_RENDER_MASK)
	op = PictOpInReverse;

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
   
   if ((!rs) || (!dc) || (!dc->col.col)) return;
   op = PictOpOver;
   if (dc->render_op == _EVAS_RENDER_BLEND)
     {
	if (!dc->col.col) return;
     }
   else if (dc->render_op == _EVAS_RENDER_BLEND_REL)
     {
	if (!dc->col.col) return;
	op = PictOpAtop;
     }
   else if (dc->render_op == _EVAS_RENDER_MUL)
     {
	if (dc->col.col == 0xffffffff) return;
	op = PictOpIn;
     }
   else if (dc->render_op == _EVAS_RENDER_COPY)
	op = PictOpSrc;
   else if (dc->render_op == _EVAS_RENDER_COPY_REL)
	op = PictOpIn;
   else if (dc->render_op == _EVAS_RENDER_MASK)
	op = PictOpInReverse;
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
   num = 0; EINA_INLIST_ITER_NEXT(points, pt) num++;
   if (num < 3) return;
   op = PictOpOver;
   if (dc->render_op == _EVAS_RENDER_BLEND)
     {
	if (!dc->col.col) return;
     }
   else if (dc->render_op == _EVAS_RENDER_BLEND_REL)
     {
	if (!dc->col.col) return;
	op = PictOpAtop;
     }
   else if (dc->render_op == _EVAS_RENDER_MUL)
     {
	if (dc->col.col == 0xffffffff) return;
	op = PictOpIn;
     }
   else if (dc->render_op == _EVAS_RENDER_COPY)
	op = PictOpSrc;
   else if (dc->render_op == _EVAS_RENDER_COPY_REL)
	op = PictOpIn;
   else if (dc->render_op == _EVAS_RENDER_MASK)
	op = PictOpInReverse;
   a = (dc->col.col >> 24) & 0xff;
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
   EINA_INLIST_ITER_NEXT(points, pt)
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
   
