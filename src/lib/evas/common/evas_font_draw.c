#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_blend_private.h"

#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

#include "evas_font_ot.h"

#ifdef EVAS_CSERVE2
#include "../cserve2/evas_cs2_private.h"
#endif

struct _Evas_Glyph
{
   RGBA_Font_Glyph *fg;
   int x, y;
   FT_UInt idx;
};

EAPI void
evas_common_font_draw_init(void)
{
}

static void *
_evas_font_image_new_from_data(int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     {
        Evas_Cache2 *cache = evas_common_image_cache2_get();
        return evas_cache2_image_data(cache, w, h, image_data, alpha, cspace);
     }
#endif
   return evas_cache_image_data(evas_common_image_cache_get(), w, h, image_data, alpha, cspace);
}

static void
_evas_font_image_free(void *image)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(image))
     {
        evas_cache2_image_close(image);
        return;
     }
#endif
   evas_cache_image_drop(image);
}

static void
_evas_font_image_draw(void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   RGBA_Image *im;

   if (!image) return;
   im = image;

#ifdef BUILD_PIPE_RENDER
   if ((eina_cpu_count() > 1))
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_load_data(&im->cache_entry);
#endif
        evas_common_rgba_image_scalecache_prepare((Image_Entry *)(im),
                                                  surface, context, smooth,
                                                  src_x, src_y, src_w, src_h,
                                                  dst_x, dst_y, dst_w, dst_h);

        evas_common_pipe_image_draw(im, surface, context, smooth,
                                    src_x, src_y, src_w, src_h,
                                    dst_x, dst_y, dst_w, dst_h);
     }
   else
#endif
     {
        evas_common_rgba_image_scalecache_prepare
          (&im->cache_entry, surface, context, smooth,
           src_x, src_y, src_w, src_h,
           dst_x, dst_y, dst_w, dst_h);
        evas_common_rgba_image_scalecache_do
          (&im->cache_entry, surface, context, smooth,
           src_x, src_y, src_w, src_h,
           dst_x, dst_y, dst_w, dst_h);

        evas_common_cpu_end_opt();
     }
}

/*
 * BiDi handling: We receive the shaped string + other props from text_props,
 * we need to reorder it so we'll have the visual string (the way we draw)
 * and then for kerning we have to switch the order of the kerning query (as the prev
 * is on the right, and not on the left).
 */
EAPI Eina_Bool
evas_common_font_rgba_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y,
                           Evas_Glyph_Array *glyphs, RGBA_Gfx_Func func EINA_UNUSED, int ext_x, int ext_y, int ext_w,
                           int ext_h, int im_w, int im_h EINA_UNUSED)
{
   Evas_Glyph *glyph;

   if (!glyphs) return EINA_FALSE;
   if (!glyphs->array) return EINA_FALSE;

   EINA_INARRAY_FOREACH(glyphs->array, glyph)
     {
        RGBA_Font_Glyph *fg;
        int chr_x, chr_y, w, h;

        fg = glyph->fg;
        w = fg->glyph_out->bitmap.width;
        h = fg->glyph_out->bitmap.rows;

        if ((!fg->ext_dat) && (dc->font_ext.func.gl_new))
          {
             /* extension calls */
             fg->ext_dat = dc->font_ext.func.gl_new(dc->font_ext.data, fg);
             fg->ext_dat_free = dc->font_ext.func.gl_free;
          }

        if ((!fg->ext_dat) && FT_HAS_COLOR(fg->fi->src->ft.face))
          {
             if (dc->font_ext.func.gl_image_new_from_data)
               {
                  /* extension calls */
                  fg->ext_dat = dc->font_ext.func.gl_image_new_from_data
                    (dc->font_ext.data, (unsigned int)w, (unsigned int)h,
                     (DATA32 *)fg->glyph_out->bitmap.buffer, EINA_TRUE,
                     EVAS_COLORSPACE_ARGB8888);
                  fg->ext_dat_free = dc->font_ext.func.gl_image_free;
               }
             else
               {
                  fg->ext_dat = _evas_font_image_new_from_data
                    (w, h, (DATA32 *)fg->glyph_out->bitmap.buffer,
                     EINA_TRUE, EVAS_COLORSPACE_ARGB8888);
                  fg->ext_dat_free = _evas_font_image_free;
               }
          }

        chr_x = x + glyph->x;
        chr_y = y + glyph->y;
        if (chr_x < (ext_x + ext_w))
          {
             if ((w > 0) && ((chr_x + w) > ext_x))
               {
                  if (fg->glyph_out->rle)
                    {
                       if ((fg->ext_dat) && (dc->font_ext.func.gl_draw))
                         dc->font_ext.func.gl_draw(dc->font_ext.data, dst,
                                                   dc, fg,
                                                   chr_x, y - (chr_y - y));
                       else
                         evas_common_font_glyph_draw(fg, dc, dst, im_w,
                                                     chr_x, y - (chr_y - y),
                                                     ext_x, ext_y,
                                                     ext_w, ext_h);
                    }
                  else if ((fg->ext_dat) && FT_HAS_COLOR(fg->fi->src->ft.face))
                    {
                       if (dc->font_ext.func.gl_image_draw)
                         dc->font_ext.func.gl_image_draw
                           (dc->font_ext.data, fg->ext_dat, 0, 0, w, h,
                            chr_x, y - (chr_y - y), w, h, EINA_TRUE);
                       else
                         _evas_font_image_draw
                           (dc, dst, fg->ext_dat, 0, 0, w, h,
                            chr_x, y - (chr_y - y), w, h, EINA_TRUE);
                    }
               }
          }
        else
          break;
     }

   return EINA_TRUE;
}

void
evas_common_font_glyphs_ref(Evas_Glyph_Array *array)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && !array->refcount)
     {
        Eina_Iterator *iter;
        Evas_Glyph *glyph;

        iter = eina_inarray_iterator_new(array->array);
        EINA_ITERATOR_FOREACH(iter, glyph)
          evas_cserve2_font_glyph_ref(glyph->fg->glyph_out, EINA_TRUE);
        eina_iterator_free(iter);
     }
#endif

   array->refcount++;
}

void
evas_common_font_glyphs_unref(Evas_Glyph_Array *array)
{
   if (--array->refcount) return;

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     {
        Eina_Iterator *iter;
        Evas_Glyph *glyph;

        iter = eina_inarray_iterator_new(array->array);
        EINA_ITERATOR_FOREACH(iter, glyph)
          evas_cserve2_font_glyph_ref(glyph->fg->glyph_out, EINA_FALSE);
        eina_iterator_free(iter);
     }
#endif

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
   RGBA_Font_Glyph *fg = NULL;
   Eina_Inarray *glyphs;
   size_t unit = 32;
   Eina_Bool reused_glyphs;
   EVAS_FONT_WALK_TEXT_INIT();

   fi = text_props->font_instance;
   if (!fi) return;

   if ((!text_props->changed) &&
       (text_props->generation == fi->generation) &&
       text_props->glyphs)
     return;

   if (text_props->len < unit) unit = text_props->len;
   if (text_props->glyphs && text_props->glyphs->refcount == 1)
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          {
             Eina_Iterator *iter;
             Evas_Glyph *glyph;

             iter = eina_inarray_iterator_new(text_props->glyphs->array);
             EINA_ITERATOR_FOREACH(iter, glyph)
               evas_cserve2_font_glyph_ref(glyph->fg->glyph_out, EINA_FALSE);
             eina_iterator_free(iter);
          }
#endif
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
        if (!evas_common_font_int_cache_glyph_render(fg))
          {
             fg = NULL;
             goto error;
          }

#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cserve2_font_glyph_ref(fg->glyph_out, EINA_TRUE);
#endif

        glyph = eina_inarray_grow(glyphs, 1);
        if (!glyph) goto error;

        glyph->fg = fg;
        glyph->idx = idx;
        glyph->x = EVAS_FONT_WALK_PEN_X + EVAS_FONT_WALK_X_OFF + EVAS_FONT_WALK_X_BEAR;
        glyph->y = EVAS_FONT_WALK_PEN_Y + EVAS_FONT_WALK_Y_OFF + EVAS_FONT_WALK_Y_BEAR;
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
   eina_inarray_free(glyphs);
}

EAPI Eina_Bool
evas_common_font_draw_cb(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs, Evas_Common_Font_Draw_Cb cb)
{
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
   func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst->cache_entry.flags.alpha, 1, dc->render_op);

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
             dc->cache.rects = evas_common_draw_context_apply_cutouts(dc, dc->cache.rects);
             for (i = 0; i < dc->cache.rects->active; ++i)
               {
                  r = dc->cache.rects->rects + i;
                  evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
                  ret |= cb(dst, dc, x, y, glyphs,
                            func, r->x, r->y, r->w, r->h,
                            im_w, im_h);
               }
             evas_common_draw_context_cache_update(dc);
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
evas_common_font_draw_prepare_cutout(Cutout_Rects **reuse, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Gfx_Func *func)
{
   int im_w, im_h;

   im_w = dst->cache_entry.w;
   im_h = dst->cache_entry.h;

   *func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst->cache_entry.flags.alpha, 1, dc->render_op);

   evas_common_draw_context_clip_clip(dc, 0, 0, im_w, im_h);
   if (dc->clip.w <= 0) return EINA_FALSE;
   if (dc->clip.h <= 0) return EINA_FALSE;

   if (dc->cutout.rects)
     {
        *reuse = evas_common_draw_context_apply_cutouts(dc, *reuse);
     }

   return EINA_TRUE;
}

