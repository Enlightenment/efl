#include "evas_soft16.h"
#include "evas_soft16_scanline_blend.c"

static void
_soft16_image_draw_scaled_solid_solid(Soft16_Image *src,
				      Soft16_Image *dst,
				      RGBA_Draw_Context *dc,
				      int dst_offset, int w, int h,
				      int *offset_x, int *offset_y)
{
   DATA16 *dst_itr;
   int y, w_align;

   w_align = w & ~7;

   dst_itr = dst->pixels + dst_offset;
   for (y = 0; y < h; y++, dst_itr += dst->stride)
     {
	DATA16 *d, *s;
	int x;

	s = src->pixels + offset_y[y];
	pld(s, 0);
	pld(offset_x, 0);

	d = dst_itr;
	x = 0;
	while (x < w_align)
	  {
	     pld(s, 32);
	     pld(offset_x + x, 32);

	     UNROLL8({
		_soft16_pt_blend_solid_solid(d, s[offset_x[x]]);
		x++;
		d++;
	     });
	  }

	for (; x < w; x++, d++)
	  _soft16_pt_blend_solid_solid(d, s[offset_x[x]]);
     }
}
static void
_soft16_image_draw_scaled_transp_solid(Soft16_Image *src,
				       Soft16_Image *dst,
				       RGBA_Draw_Context *dc,
				       int dst_offset, int w, int h,
				       int *offset_x, int *offset_y)
{
   DATA16 *dst_itr;
   int y, w_align;

   w_align = w & ~7;

   dst_itr = dst->pixels + dst_offset;
   for (y = 0; y < h; y++, dst_itr += dst->stride)
     {
	DATA16 *d, *s;
	DATA8 *a;
	int x;

	s = src->pixels + offset_y[y];
	a = src->alpha + offset_y[y];
	pld(s, 0);
	pld(a, 0);
	pld(offset_x, 0);

	d = dst_itr;
	x = 0;
	while (x < w_align)
	  {
	     pld(s, 32);
	     pld(a, 8);
	     pld(offset_x + x, 32);

	     UNROLL8({
		int off_x = offset_x[x];
		_soft16_pt_blend_transp_solid(d, s[off_x], a[off_x]);
		x++;
		d++;
	     });
	  }

	for (; x < w; x++, d++)
	  _soft16_pt_blend_transp_solid(d, s[offset_x[x]], a[offset_x[x]]);
     }
}

static inline void
_soft16_image_draw_scaled_no_mul(Soft16_Image *src, Soft16_Image *dst,
				 RGBA_Draw_Context *dc,
				 int dst_offset, int w, int h,
				 int *offset_x, int *offset_y)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_scaled_transp_solid
	(src, dst, dc, dst_offset, w, h, offset_x, offset_y);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_scaled_solid_solid
	(src, dst, dc, dst_offset, w, h, offset_x, offset_y);
   else
      fprintf(stderr,
              "Unsupported draw of scaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITHOUT COLOR MUL\n",
              src->have_alpha, dst->have_alpha);
}

static void
_soft16_image_draw_scaled_solid_solid_mul_alpha(Soft16_Image *src,
						Soft16_Image *dst,
						RGBA_Draw_Context *dc,
						int dst_offset, int w, int h,
						int *offset_x, int *offset_y)
{
   DATA16 *dst_itr;
   int y, w_align, rel_alpha;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   w_align = w & ~7;

   dst_itr = dst->pixels + dst_offset;
   for (y = 0; y < h; y++, dst_itr += dst->stride)
     {
	DATA16 *d, *s;
	int x;

	s = src->pixels + offset_y[y];
	pld(s, 0);
	pld(offset_x, 0);

	d = dst_itr;
	x = 0;
	while (x < w_align)
	  {
	     pld(s, 32);
	     pld(offset_x + x, 32);

	     UNROLL8({
		_soft16_pt_blend_solid_solid_mul_alpha
		  (d, s[offset_x[x]], rel_alpha);
		x++;
		d++;
	     });
	  }

	for (; x < w; x++, d++)
	  _soft16_pt_blend_solid_solid_mul_alpha
	    (d, s[offset_x[x]], rel_alpha);
     }
}

static void
_soft16_image_draw_scaled_transp_solid_mul_alpha(Soft16_Image *src,
						 Soft16_Image *dst,
						 RGBA_Draw_Context *dc,
						 int dst_offset, int w, int h,
						 int *offset_x, int *offset_y)
{
   DATA16 *dst_itr;
   int y, w_align, rel_alpha;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   w_align = w & ~7;

   dst_itr = dst->pixels + dst_offset;
   for (y = 0; y < h; y++, dst_itr += dst->stride)
     {
	DATA16 *d, *s;
	DATA8 *a;
	int x;

	s = src->pixels + offset_y[y];
	a = src->alpha + offset_y[y];
	pld(s, 0);
	pld(a, 0);
	pld(offset_x, 0);

	d = dst_itr;
	x = 0;
	while (x < w_align)
	  {
	     pld(s, 32);
	     pld(a, 8);
	     pld(offset_x + x, 32);

	     UNROLL8({
		int off_x = offset_x[x];
		_soft16_pt_blend_transp_solid_mul_alpha
		  (d, s[off_x], a[off_x], rel_alpha);
		x++;
		d++;
	     });
	  }

	for (; x < w; x++, d++)
	  _soft16_pt_blend_transp_solid_mul_alpha
	    (d, s[offset_x[x]], a[offset_x[x]], rel_alpha);
     }
}

static inline void
_soft16_image_draw_scaled_mul_alpha(Soft16_Image *src, Soft16_Image *dst,
				    RGBA_Draw_Context *dc,
				    int dst_offset, int w, int h,
				    int *offset_x, int *offset_y)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_scaled_transp_solid_mul_alpha
         (src, dst, dc, dst_offset, w, h, offset_x, offset_y);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_scaled_solid_solid_mul_alpha
         (src, dst, dc, dst_offset, w, h, offset_x, offset_y);
   else
      fprintf(stderr,
              "Unsupported draw of scaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITH ALPHA MUL %d\n",
              src->have_alpha, dst->have_alpha, A_VAL(&dc->mul.col));
}

static void
_soft16_image_draw_scaled_solid_solid_mul_color(Soft16_Image *src,
						Soft16_Image *dst,
						RGBA_Draw_Context *dc,
						int dst_offset, int w, int h,
						int *offset_x, int *offset_y)
{
   DATA16 *dst_itr;
   int y, w_align, rel_alpha, r, g, b;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;

   r = R_VAL(&dc->mul.col);
   g = G_VAL(&dc->mul.col);
   b = B_VAL(&dc->mul.col);
   /* we'll divide by 256 to make it faster, try to improve things a bit */
   if (r > 127) r++;
   if (g > 127) g++;
   if (b > 127) b++;

   w_align = w & ~7;

   dst_itr = dst->pixels + dst_offset;

   if (rel_alpha == 31)
     for (y = 0; y < h; y++, dst_itr += dst->stride)
       {
	  DATA16 *d, *s;
	  int x;

	  s = src->pixels + offset_y[y];
	  pld(s, 0);
	  pld(offset_x, 0);

	  d = dst_itr;
	  x = 0;
	  while (x < w_align)
	    {
	       pld(s, 32);
	       pld(offset_x + x, 32);

	       UNROLL8({
		  _soft16_pt_blend_solid_solid_mul_color_solid
		    (d, s[offset_x[x]], r, g, b);
		  x++;
		  d++;
	       });
	    }

	  for (; x < w; x++, d++)
	    _soft16_pt_blend_solid_solid_mul_color_solid
	      (d, s[offset_x[x]], r, g, b);
       }
   else
     for (y = 0; y < h; y++, dst_itr += dst->stride)
       {
	  DATA16 *d, *s;
	  int x;

	  s = src->pixels + offset_y[y];
	  pld(s, 0);
	  pld(offset_x, 0);

	  d = dst_itr;
	  x = 0;
	  while (x < w_align)
	    {
	       pld(s, 32);
	       pld(offset_x + x, 32);

	       UNROLL8({
		  _soft16_pt_blend_solid_solid_mul_color_transp
		    (d, s[offset_x[x]], rel_alpha, r, g, b);
		  x++;
		  d++;
	       });
	    }

	  for (; x < w; x++, d++)
	    _soft16_pt_blend_solid_solid_mul_color_transp
	      (d, s[offset_x[x]], rel_alpha, r, g, b);
       }
}

static void
_soft16_image_draw_scaled_transp_solid_mul_color(Soft16_Image *src,
						 Soft16_Image *dst,
						 RGBA_Draw_Context *dc,
						 int dst_offset, int w, int h,
						 int *offset_x, int *offset_y)
{
   DATA16 *dst_itr;
   int y, w_align, rel_alpha, r, g, b;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   r = R_VAL(&dc->mul.col);
   g = G_VAL(&dc->mul.col);
   b = B_VAL(&dc->mul.col);
   /* we'll divide by 256 to make it faster, try to improve things a bit */
   if (r > 127) r++;
   if (g > 127) g++;
   if (b > 127) b++;

   w_align = w & ~7;

   dst_itr = dst->pixels + dst_offset;

   if (rel_alpha == 0)
     for (y = 0; y < h; y++, dst_itr += dst->stride)
       {
	  DATA16 *d, *s;
	  DATA8 *a;
	  int x;

	  s = src->pixels + offset_y[y];
	  a = src->alpha + offset_y[y];
	  pld(s, 0);
	  pld(a, 0);
	  pld(offset_x, 0);

	  d = dst_itr;
	  x = 0;
	  while (x < w_align)
	    {
	       pld(s, 32);
	       pld(a, 8);
	       pld(offset_x + x, 32);

	       UNROLL8({
		  int off_x = offset_x[x];
		  _soft16_pt_blend_transp_solid_mul_color_solid
		    (d, s[off_x], a[off_x], r, g, b);
		  x++;
		  d++;
	       });
	    }

	  for (; x < w; x++, d++)
	    _soft16_pt_blend_transp_solid_mul_color_solid
	      (d, s[offset_x[x]], a[offset_x[x]], r, g, b);
       }
   else
     for (y = 0; y < h; y++, dst_itr += dst->stride)
       {
	  DATA16 *d, *s;
	  DATA8 *a;
	  int x;

	  s = src->pixels + offset_y[y];
	  a = src->alpha + offset_y[y];
	  pld(s, 0);
	  pld(a, 0);
	  pld(offset_x, 0);

	  d = dst_itr;
	  x = 0;
	  while (x < w_align)
	    {
	       pld(s, 32);
	       pld(a, 8);
	       pld(offset_x + x, 32);

	       UNROLL8({
		  int off_x = offset_x[x];
		  _soft16_pt_blend_transp_solid_mul_color_transp
		    (d, s[off_x], a[off_x], rel_alpha, r, g, b);
		  x++;
		  d++;
	       });
	    }

	  for (; x < w; x++, d++)
	    _soft16_pt_blend_transp_solid_mul_color_transp
	      (d, s[offset_x[x]], a[offset_x[x]], rel_alpha, r, g, b);
       }
}

static inline void
_soft16_image_draw_scaled_mul_color(Soft16_Image *src, Soft16_Image *dst,
				    RGBA_Draw_Context *dc,
				    int dst_offset, int w, int h,
				    int *offset_x, int *offset_y)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_scaled_transp_solid_mul_color
         (src, dst, dc, dst_offset, w, h, offset_x, offset_y);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_scaled_solid_solid_mul_color
         (src, dst, dc, dst_offset, w, h, offset_x, offset_y);
   else
      fprintf(stderr,
              "Unsupported draw of scaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITH COLOR MUL 0x%08x\n",
              src->have_alpha, dst->have_alpha, dc->mul.col);
}

static inline void
_soft16_image_draw_scaled_mul(Soft16_Image *src, Soft16_Image *dst,
			      RGBA_Draw_Context *dc,
			      int dst_offset, int w, int h,
			      int *offset_x, int *offset_y)
{
   if ((A_VAL(&dc->mul.col) == R_VAL(&dc->mul.col)) &&
       (A_VAL(&dc->mul.col) == G_VAL(&dc->mul.col)) &&
       (A_VAL(&dc->mul.col) == B_VAL(&dc->mul.col)))
      _soft16_image_draw_scaled_mul_alpha
	(src, dst, dc, dst_offset, w, h, offset_x, offset_y);
   else
      _soft16_image_draw_scaled_mul_color
	(src, dst, dc, dst_offset, w, h, offset_x, offset_y);
}

void
soft16_image_draw_scaled_sampled(Soft16_Image *src, Soft16_Image *dst,
				 RGBA_Draw_Context *dc,
				 const Evas_Rectangle sr,
				 const Evas_Rectangle dr,
				 const Evas_Rectangle cr)
{
   int x, y, dst_offset, *offset_x, *offset_y;

   /* pre-calculated scale tables */
   offset_x = alloca(cr.w * sizeof(*offset_x));
   for (x = 0; x < cr.w; x++)
     offset_x[x] = (((x + cr.x - dr.x) * sr.w) / dr.w) + sr.x;

   offset_y = alloca(cr.h * sizeof(*offset_y));
   for (y = 0; y < cr.h; y++)
     offset_y[y] = (((((y + cr.y - dr.y) * sr.h) / dr.h) + sr.y)
		    * src->stride);

   dst_offset = cr.x + (cr.y * dst->stride);

   if ((!dc->mul.use) || (dc->mul.col == 0xffffffff))
     _soft16_image_draw_scaled_no_mul
       (src, dst, dc, dst_offset, cr.w, cr.h, offset_x, offset_y);
   else if (dc->mul.col != 0x00000000)
     _soft16_image_draw_scaled_mul
       (src, dst, dc, dst_offset, cr.w, cr.h, offset_x, offset_y);
}
