#include "evas_common.h"
#include "evas_private.h"
#include "evas_blend_private.h"

#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

#include "evas_font_ot.h"

struct prword
{
   EINA_INLIST;
   struct cinfo *cinfo;
   Evas_Text_Props text_props;
   DATA8 *im;
   int roww;
   int width;
   int height;
   int baseline;
};

struct cinfo
{
   FT_UInt index;
   struct
     {
        int x, y;
     } pos;
   int posx;
   RGBA_Font_Glyph *fg;
   struct
     {
        int w,h;
        int rows;
        unsigned char *data;
     } bm;
};

typedef struct _Evas_Glyph Evas_Glyph;
struct _Evas_Glyph
{
   RGBA_Font_Glyph *fg;
   void *data;
   Eina_Rectangle coord;
   FT_UInt idx;
   int j;
};

EAPI void
evas_common_font_draw_init(void)
{
}

/*
 * BiDi handling: We receive the shaped string + other props from text_props,
 * we need to reorder it so we'll have the visual string (the way we draw)
 * and then for kerning we have to switch the order of the kerning query (as the prev
 * is on the right, and not on the left).
 */
static void
evas_common_font_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y,
                               const Evas_Text_Props *text_props, RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w,
                               int ext_h, int im_w, int im_h __UNUSED__)
{
   DATA32 *im;
   Evas_Glyph *glyphs;
   unsigned int length;
   unsigned int it;

   im = dst->image.data;

   if (!text_props->bin) return ;

   glyphs = (void*) eina_binbuf_string_get(text_props->bin);
   length = eina_binbuf_length_get(text_props->bin) / sizeof (Evas_Glyph);
   for (it = 0; it < length; ++it)
     {
        FT_UInt idx;
        RGBA_Font_Glyph *fg;
        int chr_x, chr_y;

        fg = glyphs[it].fg;
        idx = glyphs[it].idx;

        if (dc->font_ext.func.gl_new)
          {
             /* extension calls */
             fg->ext_dat = dc->font_ext.func.gl_new(dc->font_ext.data, fg);
             fg->ext_dat_free = dc->font_ext.func.gl_free;
          }

        chr_x = x + glyphs[it].coord.x/* EVAS_FONT_WALK_PEN_X + EVAS_FONT_WALK_X_OFF + EVAS_FONT_WALK_X_BEAR */;
        chr_y = y + glyphs[it].coord.y/* EVAS_FONT_WALK_PEN_Y + EVAS_FONT_WALK_Y_OFF + EVAS_FONT_WALK_Y_BEAR */;

        if (chr_x < (ext_x + ext_w))
          {
             DATA8 *data;
             int i, j, w, h;

             data = glyphs[it].data;
             j = glyphs[it].j;
             w = glyphs[it].coord.w;
             if (j < w) j = w;
             h = glyphs[it].coord.h;
             /*
              if ((fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays)
              && (fg->glyph_out->bitmap.num_grays == 256)
              )
              */

#ifdef HAVE_PIXMAN
# ifdef PIXMAN_FONT             
             int index;
             DATA32 *font_alpha_buffer;
             pixman_image_t *font_mask_image;

             font_alpha_buffer = alloca(w * h * sizeof(DATA32));
             for (index = 0; index < (w * h); index++)
               font_alpha_buffer[index] = data[index] << 24;
             
             font_mask_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, w, h,
                                                        font_alpha_buffer, 
                                                        w * sizeof(DATA32));

             if (!font_mask_image)  return;
# endif
#endif

               {
                  if ((j > 0) && (chr_x + w > ext_x))
                    {
                       if ((fg->ext_dat) && (dc->font_ext.func.gl_draw))
                         {
                            /* ext glyph draw */
                            dc->font_ext.func.gl_draw(dc->font_ext.data,
                                                      (void *)dst,
                                                      dc, fg, chr_x,
                                                      y - (chr_y - y));
                         }
                       else
                         {
                            if ((fg->glyph_out->bitmap.num_grays == 256) &&
                                (fg->glyph_out->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY))
                              {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_FONT
                                 if ((dst->pixman.im) && 
                                     (dc->col.pixman_color_image))
                                   pixman_image_composite(PIXMAN_OP_OVER, 
                                                          dc->col.pixman_color_image, 
                                                          font_mask_image, 
                                                          dst->pixman.im,
                                                          chr_x, 
                                                          y - (chr_y - y), 
                                                          0, 0, 
                                                          chr_x, 
                                                          y - (chr_y - y), 
                                                          w, h);
                                 else
# endif                                   
#endif
                                   {
                                      for (i = 0; i < h; i++)
                                        {
                                           int dx, dy;
                                           int in_x, in_w;
                                           
                                           in_x = 0;
                                           in_w = 0;
                                           dx = chr_x;
                                           dy = y - (chr_y - i - y);

					   if ((dx < (ext_x + ext_w)) &&
					       (dy >= (ext_y)) &&
					       (dy < (ext_y + ext_h)))
					     {
					       if (dx + w > (ext_x + ext_w))
						 in_w += (dx + w) - (ext_x + ext_w);
					       if (dx < ext_x)
						 {
						   in_w += ext_x - dx;
						   in_x = ext_x - dx;
						   dx = ext_x;
						 }
					       if (in_w < w)
						 {
						   func(NULL, data + (i * j) + in_x, dc->col.col,
							im + (dy * im_w) + dx, w - in_w);
						 }
					     }
                                        }
                                   }
                              }
                            else
                              {
                                 DATA8 *tmpbuf = NULL, *dp, *tp, bits;
                                 int bi, bj;
                                 const DATA8 bitrepl[2] = {0x0, 0xff};

                                 tmpbuf = alloca(w);
                                 for (i = 0; i < h; i++)
                                   {
                                      int dx, dy;
                                      int in_x, in_w, end;
                                      
                                      in_x = 0;
                                      in_w = 0;
                                      dx = chr_x;
                                      dy = y - (chr_y - i - y);

				      tp = tmpbuf;
				      dp = data + (i * fg->glyph_out->bitmap.pitch);
				      for (bi = 0; bi < w; bi += 8)
					{
					  bits = *dp;
					  if ((w - bi) < 8) end = w - bi;
					  else end = 8;
					  for (bj = 0; bj < end; bj++)
					    {
					      *tp = bitrepl[(bits >> (7 - bj)) & 0x1];
					      tp++;
					    }
					  dp++;
					}
				      if ((dx < (ext_x + ext_w)) &&
					  (dy >= (ext_y)) &&
					  (dy < (ext_y + ext_h)))
					{
					  if (dx + w > (ext_x + ext_w))
					    in_w += (dx + w) - (ext_x + ext_w);
					  if (dx < ext_x)
					    {
					      in_w += ext_x - dx;
					      in_x = ext_x - dx;
					      dx = ext_x;
					    }
					  if (in_w < w)
					    {
					      func(NULL, tmpbuf + in_x, dc->col.col,
						   im + (dy * im_w) + dx, w - in_w);
					    }
                                        }
                                   }
                              }
                         }
                    }
               }
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_FONT
             pixman_image_unref(font_mask_image);
# endif
#endif
          }
        else
          break;
     }
}

EAPI void
evas_common_font_draw_prepare(Evas_Text_Props *text_props)
{
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();

   fi = text_props->font_instance;
   if (!fi) return;

   if (!text_props->changed && text_props->generation == fi->generation && text_props->bin)
     return ;

   if (!text_props->bin) text_props->bin = eina_binbuf_new();
   else eina_binbuf_reset(text_props->bin);

   evas_common_font_int_reload(fi);

   if (fi->src->current_size != fi->size && text_props->generation == fi->generation)
     {
        evas_common_font_source_reload(fi->src);
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

   EVAS_FONT_WALK_TEXT_START()
     {
        Evas_Glyph glyph;
        RGBA_Font_Glyph *fg;
        FT_UInt idx;

        if (!EVAS_FONT_WALK_IS_VISIBLE) continue;
        idx = EVAS_FONT_WALK_INDEX;

        fg = evas_common_font_int_cache_glyph_get(fi, idx);
        if (!fg) continue;
        if (!fg->glyph_out) evas_common_font_int_cache_glyph_render(fg);

        glyph.fg = fg;
        glyph.coord.x = EVAS_FONT_WALK_PEN_X + EVAS_FONT_WALK_X_OFF + EVAS_FONT_WALK_X_BEAR;
        glyph.coord.y = EVAS_FONT_WALK_PEN_Y + EVAS_FONT_WALK_Y_OFF + EVAS_FONT_WALK_Y_BEAR;
        glyph.coord.w = fg->glyph_out->bitmap.width;
        glyph.coord.h = fg->glyph_out->bitmap.rows;
        glyph.j = fg->glyph_out->bitmap.pitch;
        glyph.idx = idx;
        glyph.data = fg->glyph_out->bitmap.buffer;

        eina_binbuf_append_length(text_props->bin, (void*) &glyph, sizeof (Evas_Glyph));
     }
   EVAS_FONT_WALK_TEXT_END();

   text_props->generation = fi->generation;
}

EAPI void
evas_common_font_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, const Evas_Text_Props *text_props)
{
   int ext_x, ext_y, ext_w, ext_h;
   int im_w, im_h;
   RGBA_Gfx_Func func;
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int c, cx, cy, cw, ch;
   int i;

   im_w = dst->cache_entry.w;
   im_h = dst->cache_entry.h;

   ext_x = 0; ext_y = 0; ext_w = im_w; ext_h = im_h;
   if (dc->clip.use)
     {
	ext_x = dc->clip.x;
	ext_y = dc->clip.y;
	ext_w = dc->clip.w;
	ext_h = dc->clip.h;
	if (ext_x < 0)
	  {
	     ext_w += ext_x;
	     ext_x = 0;
	  }
	if (ext_y < 0)
	  {
	     ext_h += ext_y;
	     ext_y = 0;
	  }
	if ((ext_x + ext_w) > im_w)
	  ext_w = im_w - ext_x;
	if ((ext_y + ext_h) > im_h)
	  ext_h = im_h - ext_y;
     }
   if (ext_w <= 0) return;
   if (ext_h <= 0) return;

//   evas_common_font_size_use(fn);
   func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst, 1, dc->render_op);

   if (!dc->cutout.rects)
     {
        evas_common_font_draw_internal(dst, dc, x, y, text_props,
                                       func, ext_x, ext_y, ext_w, ext_h,
                                       im_w, im_h);
     }
   else
     {
        c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
        evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
        /* our clip is 0 size.. abort */
        if ((dc->clip.w > 0) && (dc->clip.h > 0))
          {
             rects = evas_common_draw_context_apply_cutouts(dc);
             for (i = 0; i < rects->active; ++i)
               {
                  r = rects->rects + i;
                  evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
                  evas_common_font_draw_internal(dst, dc, x, y, text_props,
                                                 func, r->x, r->y, r->w, r->h,
                                                 im_w, im_h);
               }
             evas_common_draw_context_apply_clear_cutouts(rects);
          }
        dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
     }
}
