#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_blend_private.h"

#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

#include "evas_font_ot.h"

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
EAPI Eina_Bool
evas_common_font_rgba_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y,
                           Evas_Glyph_Array *glyphs, RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w,
                           int ext_h, int im_w, int im_h EINA_UNUSED)
{
   DATA32 *im;
   Evas_Glyph *glyph;

   if (!glyphs) return EINA_FALSE;
   if (!glyphs->array) return EINA_FALSE;

   im = dst->image.data;

   EINA_INARRAY_FOREACH(glyphs->array, glyph)
     {
        RGBA_Font_Glyph *fg;
        int chr_x, chr_y;

        fg = glyph->fg;

	/* FIXME: Why was that moved out of prepare ? This increase cache miss. */
        glyph->coord.w = fg->glyph_out->bitmap.width;
        glyph->coord.h = fg->glyph_out->bitmap.rows;
        glyph->j = fg->glyph_out->bitmap.pitch;
        glyph->data = fg->glyph_out->bitmap.buffer;

        if (dc->font_ext.func.gl_new)
          {
             /* extension calls */
             fg->ext_dat = dc->font_ext.func.gl_new(dc->font_ext.data, fg);
             fg->ext_dat_free = dc->font_ext.func.gl_free;
          }

        chr_x = x + glyph->coord.x;
        chr_y = y + glyph->coord.y;

        if (chr_x < (ext_x + ext_w))
          {
             DATA8 *data;
             int i, j, w, h;

             data = glyph->data;
             j = glyph->j;
             w = glyph->coord.w;
             if (j < w) j = w;
             h = glyph->coord.h;

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

             if (!font_mask_image) return EINA_FALSE;
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

   return EINA_TRUE;
}

void
evas_common_font_glyphs_ref(Evas_Glyph_Array *array)
{
   array->refcount++;
}

void
evas_common_font_glyphs_unref(Evas_Glyph_Array *array)
{
   if (--array->refcount) return;

   eina_inarray_free(array->array);
   evas_common_font_int_unref(array->fi);
   free(array);
}

void
evas_common_font_fonts_ref(Evas_Font_Array *array)
{
   array->refcount++;
}

void
evas_common_font_fonts_unref(Evas_Font_Array *array)
{
   if (--array->refcount) return;

   eina_inarray_free(array->array);
   free(array);
}

EAPI void
evas_common_font_draw_prepare(Evas_Text_Props *text_props)
{
   RGBA_Font_Int *fi;
   RGBA_Font_Glyph *fg;
   Eina_Inarray *glyphs;
   size_t unit = 32;
   Eina_Bool reused_glyphs;
   EVAS_FONT_WALK_TEXT_INIT();

   fi = text_props->font_instance;
   if (!fi) return;

   if (!text_props->changed && text_props->generation == fi->generation && text_props->glyphs)
     return;

   if (text_props->len < unit) unit = text_props->len;
   if (text_props->glyphs && text_props->glyphs->refcount == 1)
     {
        glyphs = text_props->glyphs->array;
        glyphs->len = 0;
        reused_glyphs = EINA_TRUE;
     }
   else
     {
        glyphs = eina_inarray_new(sizeof(Evas_Glyph), unit);
        reused_glyphs = EINA_FALSE;
     }
   evas_common_font_int_reload(fi);

   if (fi->src->current_size != fi->size)
     {
        evas_common_font_source_reload(fi->src);
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

   EVAS_FONT_WALK_TEXT_START()
     {
        Evas_Glyph *glyph;
        FT_UInt idx;

        if (!EVAS_FONT_WALK_IS_VISIBLE) continue;
        idx = EVAS_FONT_WALK_INDEX;

        fg = evas_common_font_int_cache_glyph_get(fi, idx);
        if (!fg) continue;
        if (!fg->glyph_out) evas_common_font_int_cache_glyph_render(fg);
	
	glyph = eina_inarray_grow(glyphs, 1);
	if (!glyph) goto error;

        glyph->fg = fg;
        glyph->idx = idx;
        glyph->coord.x = EVAS_FONT_WALK_PEN_X + EVAS_FONT_WALK_X_OFF + EVAS_FONT_WALK_X_BEAR;
        glyph->coord.y = EVAS_FONT_WALK_PEN_Y + EVAS_FONT_WALK_Y_OFF + EVAS_FONT_WALK_Y_BEAR;
     }
   EVAS_FONT_WALK_TEXT_END();

   if (!reused_glyphs)
     {
        if (text_props->glyphs) evas_common_font_glyphs_unref(text_props->glyphs);

        text_props->glyphs = malloc(sizeof(*text_props->glyphs));
        if (!text_props->glyphs) goto error;
        text_props->glyphs->refcount = 1;
        text_props->glyphs->array = glyphs;
        text_props->glyphs->fi = fi;
        fi->references++;
     }

   /* check if there's a request queue in fi, if so ask cserve2 to render
    * those glyphs
    */

   text_props->generation = fi->generation;
   text_props->changed = EINA_FALSE;

   return;

error:
   if (fg) free(fg);
   eina_inarray_free(glyphs);
}

EAPI Eina_Bool
evas_common_font_draw_cb(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs, Evas_Common_Font_Draw_Cb cb)
{
   static Cutout_Rects *rects = NULL;
   int ext_x, ext_y, ext_w, ext_h;
   int im_w, im_h;
   RGBA_Gfx_Func func;
   Cutout_Rect  *r;
   int c, cx, cy, cw, ch;
   int i;

   if (!glyphs) return EINA_FALSE;

   im_w = dst->cache_entry.w;
   im_h = dst->cache_entry.h;

//   evas_common_font_size_use(fn);
   func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst, 1, dc->render_op);

   if (!dc->cutout.rects)
     {
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
        if (ext_w <= 0) return EINA_FALSE;
        if (ext_h <= 0) return EINA_FALSE;

        return cb(dst, dc, x, y, glyphs,
                  func, ext_x, ext_y, ext_w, ext_h,
                  im_w, im_h);
     }
   else
     {
        Eina_Bool ret = EINA_FALSE;
        c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
        evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
        /* our clip is 0 size.. abort */
        if ((dc->clip.w > 0) && (dc->clip.h > 0))
          {
             rects = evas_common_draw_context_apply_cutouts(dc, rects);
             for (i = 0; i < rects->active; ++i)
               {
                  r = rects->rects + i;
                  evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
                  ret |= cb(dst, dc, x, y, glyphs,
                            func, r->x, r->y, r->w, r->h,
                            im_w, im_h);
               }
          }
        dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;

        return ret;
     }
}

EAPI void
evas_common_font_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs)
{
   evas_common_font_draw_cb(dst, dc, x, y, glyphs,
                            evas_common_font_rgba_draw);
}

EAPI void
evas_common_font_draw_do(const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Gfx_Func func,
                         RGBA_Image *dst, RGBA_Draw_Context *dc,
                         int x, int y, const Evas_Text_Props *text_props)
{
   Eina_Rectangle area;
   Cutout_Rect *r;
   int i;
   int im_w, im_h;

   im_w = dst->cache_entry.w;
   im_h = dst->cache_entry.h;

   if (!reuse)
     {
        evas_common_draw_context_clip_clip(dc,
                                           clip->x, clip->y,
                                           clip->w, clip->h);
        evas_common_font_rgba_draw(dst, dc, x, y, text_props->glyphs,
                                   func,
                                   dc->clip.x, dc->clip.y,
                                   dc->clip.w, dc->clip.h,
                                   im_w, im_h);
        return;
     }

   for (i = 0; i < reuse->active; ++i)
     {
        r = reuse->rects + i;

        EINA_RECTANGLE_SET(&area, r->x, r->y, r->w - 1, r->h - 1);
        if (!eina_rectangle_intersection(&area, clip)) continue ;
        evas_common_draw_context_set_clip(dc, area.x, area.y, area.w, area.h);
        evas_common_font_rgba_draw(dst, dc, x, y, text_props->glyphs,
                                   func, area.x, area.y, area.w, area.h,
                                   im_w, im_h);
     }
}

EAPI Eina_Bool
evas_common_font_draw_prepare_cutout(Cutout_Rects *reuse, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Gfx_Func *func)
{
   int im_w, im_h;

   im_w = dst->cache_entry.w;
   im_h = dst->cache_entry.h;

   *func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst, 1, dc->render_op);

   evas_common_draw_context_clip_clip(dc, 0, 0, im_w, im_h);
   if (dc->clip.w <= 0) return EINA_FALSE;
   if (dc->clip.h <= 0) return EINA_FALSE;

   if (dc->cutout.rects)
     {
        reuse = evas_common_draw_context_apply_cutouts(dc, reuse);
     }

   return EINA_TRUE;
}

