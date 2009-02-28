#include "evas_common_soft16.h"

static always_inline void
_glyph_pt_mask_solid_solid(DATA16 *dst,
			   const DATA16 rgb565,
			   const DATA32 rgb565_unpack,
			   const DATA8 *mask)
{
   DATA8 alpha = *mask >> 3;

   if (alpha == 31) *dst = rgb565;
   else if (alpha > 0)
     {
	DATA32 d;

	d = RGB_565_UNPACK(*dst);
	d = RGB_565_UNPACKED_BLEND_UNMUL(rgb565_unpack, d, alpha);
	*dst = RGB_565_PACK(d);
     }
}

static void
_glyph_scanline_mask_solid_solid(DATA16 *dst,
				 int size,
				 const DATA16 rgb565,
				 const DATA32 rgb565_unpack,
				 const DATA8 *mask)
{
   DATA16 *start, *end;

   start = dst;
   pld(start, 0);
   pld(mask, 0);
   end = start + (size & ~3);

   while (start < end)
     {
	pld(start, 16);
	pld(mask, 4);
	UNROLL4({
	   _glyph_pt_mask_solid_solid(start, rgb565, rgb565_unpack, mask);
	   start++;
	   mask++;
	});
     }

   end = start + (size & 3);
   for (; start < end; start++, mask++)
      _glyph_pt_mask_solid_solid(start, rgb565, rgb565_unpack, mask);
}

static always_inline void
_glyph_pt_mask_transp_solid(DATA16 *dst,
			    DATA32 rgb565_unpack,
			    DATA8 alpha,
			    const DATA8 *mask)
{
   DATA32 a, b;
   int rel_alpha;

   rel_alpha = *mask >> 3;
   alpha = (alpha * rel_alpha) >> 5;
   if (alpha == 0)
     return;

   alpha++;

   a = ((rgb565_unpack * rel_alpha) >> 5) & RGB_565_UNPACKED_MASK;
   b = RGB_565_UNPACK(*dst);
   b = RGB_565_UNPACKED_BLEND(a, b, alpha);
   *dst = RGB_565_PACK(b);
}

static void
_glyph_scanline_mask_transp_solid(DATA16 *dst,
				  int size,
				  const DATA32 rgb565_unpack,
				  const DATA8 rel_alpha,
				  const DATA8 *mask)
{
   DATA16 *start, *end;

   start = dst;
   pld(start, 0);
   pld(mask, 0);
   end = start + (size & ~3);

   while (start < end)
     {
	pld(start, 16);
	pld(mask, 4);
	UNROLL4({
	   _glyph_pt_mask_transp_solid(start, rgb565_unpack, rel_alpha, mask);
	   start++;
	   mask++;
	});
     }

   end = start + (size & 3);
   for (; start < end; start++, mask++)
      _glyph_pt_mask_transp_solid(start, rgb565_unpack, rel_alpha, mask);
}

static inline void
_calc_ext(const Soft16_Image *dst, const RGBA_Draw_Context *dc,
	  Evas_Rectangle *ext)
{
   ext->x = 0;
   ext->y = 0;
   ext->w = dst->cache_entry.w;
   ext->h = dst->cache_entry.h;

   if (dc->clip.use)
     {
	int v;

	ext->x = dc->clip.x;
	ext->y = dc->clip.y;
	ext->w = dc->clip.w;
	ext->h = dc->clip.h;
	if (ext->x < 0)
	  {
	     ext->w += ext->x;
	     ext->x = 0;
	  }
	if (ext->y < 0)
	  {
	     ext->h += ext->y;
	     ext->y = 0;
	  }

	v = dst->cache_entry.w - ext->x;
	if (ext->w > v) ext->w = v;

	v = dst->cache_entry.h - ext->y;
	if (ext->h > v) ext->h = v;
     }
}

static inline void
_glyph_scanline(Soft16_Image *dst, const DATA8 *p_mask,
		const Evas_Rectangle ext, int dx, int dy, int max_x, int max_y,
		int w, DATA8 alpha, const DATA16 rgb565,
		const DATA32 rgb565_unpack)
{
   int size, in_x, in_w;
   DATA16 *p_pixels;

   if ((dx >= max_x) || (dy < ext.y) || (dy >= max_y)) return;

   in_x = 0;
   in_w = 0;

   if (dx + w > max_x) in_w += (dx + w) - max_x;

   if (dx < ext.x)
     {
	in_w += ext.x - dx;
	in_x = ext.x - dx;
	dx = ext.x;
     }

   size = w - in_w;
   p_pixels = dst->pixels + (dy * dst->stride) + dx;
   p_mask += in_x;

   if (size > 1)
     {
	if (alpha == 31)
	   _glyph_scanline_mask_solid_solid
	       (p_pixels, size, rgb565, rgb565_unpack, p_mask);
	else if (alpha != 0)
	   _glyph_scanline_mask_transp_solid
	       (p_pixels, size, rgb565_unpack, alpha, p_mask);
     }
   else if (size == 1)
     {
	if (alpha == 31)
	   _glyph_pt_mask_solid_solid(p_pixels, rgb565, rgb565_unpack, p_mask);
	else if (alpha != 0)
	   _glyph_pt_mask_transp_solid(p_pixels, rgb565_unpack, alpha, p_mask);
     }
}

static void
_soft16_font_glyph_draw_grayscale(Soft16_Image *dst,
				  RGBA_Draw_Context *dc __UNUSED__, RGBA_Font_Glyph *fg __UNUSED__,
				  int x, int y, DATA8 alpha, DATA16 rgb565,
				  const Evas_Rectangle ext, int bw, int bh,
				  int bpitch, const DATA8 *bitmap)
{
   const DATA32 rgb565_unpack = RGB_565_UNPACK(rgb565);
   int i, max_x, max_y;

   max_x = ext.x + ext.w;
   max_y = ext.y + ext.h;

   for (i = 0; i < bh; i++, bitmap += bpitch)
      _glyph_scanline(dst, bitmap, ext, x, y + i, max_x, max_y, bw,
		      alpha, rgb565, rgb565_unpack);
}

static inline void
_glyph_create_mask_line(DATA8 *mask, const DATA8 *bitmap, int w)
{
   const DATA8 bitrepl[2] = {0x0, 0xff};
   int i;

   for (i = 0; i < w; i += 8, bitmap++)
     {
	int j, size;
	DATA32 bits;

	if (i + 8 < w) size = 8;
	else           size = w - i;

	bits = *bitmap;

	for (j = size - 1; j >= 0; j--, mask++)
	  *mask = bitrepl[(bits >> j) & 0x1];
     }
}

static void
_soft16_font_glyph_draw_mono(Soft16_Image *dst,
			     RGBA_Draw_Context *dc __UNUSED__, RGBA_Font_Glyph *fg __UNUSED__,
			     int x, int y, DATA8 alpha, DATA16 rgb565,
			     const Evas_Rectangle ext, int bw, int bh,
			     int bpitch, const DATA8 *bitmap)
{
   const DATA32 rgb565_unpack = RGB_565_UNPACK(rgb565);
   DATA8 *mask;
   int i, max_x, max_y;

   max_x = ext.x + ext.w;
   max_y = ext.y + ext.h;

   mask = alloca(bpitch);
   for (i = 0; i < bh; i++, bitmap += bpitch)
     {
	_glyph_create_mask_line(mask, bitmap, bw);
	_glyph_scanline(dst, mask, ext, x, y + i, max_x, max_y, bw,
			alpha, rgb565, rgb565_unpack);
     }
}

void
soft16_font_glyph_draw(void *data, void *dest __UNUSED__, void *context,
		       RGBA_Font_Glyph *fg, int x, int y)
{
   Soft16_Image *dst;
   RGBA_Draw_Context *dc;
   const DATA8 *bitmap;
   DATA8 alpha, r, g, b;
   DATA16 rgb565;
   Evas_Rectangle ext;
   int bpitch, bw, bh;

   dst = data;
   dc = context;

   alpha = A_VAL(&dc->col.col) >> 3;
   if (alpha == 0) return; /* precision is 5 bits, 3 bits lost */

   r = R_VAL(&dc->col.col) >> 3;
   g = G_VAL(&dc->col.col) >> 2;
   b = B_VAL(&dc->col.col) >> 3;

   if (r > alpha) r = alpha;
   if (g > (alpha << 1)) g = (alpha << 1);
   if (b > alpha) b = alpha;

   rgb565 = (r << 11) | (g << 5) | b;

   bitmap = fg->glyph_out->bitmap.buffer;
   bh = fg->glyph_out->bitmap.rows;
   bw = fg->glyph_out->bitmap.width;
   bpitch = fg->glyph_out->bitmap.pitch;
   if (bpitch < bw) bpitch = bw;

   _calc_ext(dst, dc, &ext);

   if ((fg->glyph_out->bitmap.num_grays == 256) &&
       (fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays))
      _soft16_font_glyph_draw_grayscale(dst, dc, fg, x, y, alpha, rgb565,
					ext, bw, bh, bpitch, bitmap);
   else
      _soft16_font_glyph_draw_mono(dst, dc, fg, x, y, alpha, rgb565,
				   ext, bw, bh, bpitch, bitmap);
}

void *
soft16_font_glyph_new(void *data __UNUSED__, RGBA_Font_Glyph *fg __UNUSED__)
{
   return (void *)1; /* core requires != NULL to work */
}

void
soft16_font_glyph_free(void *ext_dat __UNUSED__)
{
}
