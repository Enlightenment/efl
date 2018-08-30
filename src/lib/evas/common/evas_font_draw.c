#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_blend_private.h"

#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

#include "evas_font_ot.h"
#include "draw.h"

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
_evas_font_image_new(RGBA_Font_Glyph *fg, int alpha, Evas_Colorspace cspace)
{
   DATA32 *image_data;
   int src_w, src_h;

   if (!fg) return NULL;

   image_data = (DATA32 *)fg->glyph_out->bitmap.buffer;
   src_w = fg->glyph_out->bitmap.width;
   src_h = fg->glyph_out->bitmap.rows;

   return evas_cache_image_data(evas_common_image_cache_get(), src_w, src_h, image_data, alpha, cspace);
}

static void
_evas_font_image_free(void *image)
{
   evas_cache_image_drop(image);
}

static void
_evas_font_image_draw(void *context, void *surface, void *image, RGBA_Font_Glyph *fg, int x, int y, int w, int h, int smooth)
{
   RGBA_Image *im;
   int src_w, src_h;

   if (!image || !fg) return;
   im = image;
   src_w = fg->glyph_out->bitmap.width;
   src_h = fg->glyph_out->bitmap.rows;

#ifdef BUILD_PIPE_RENDER
   if ((eina_cpu_count() > 1))
     {
        evas_common_rgba_image_scalecache_prepare((Image_Entry *)(im),
                                                  surface, context, smooth,
                                                  0, 0, src_w, src_h,
                                                  x, y, w, h);

        evas_common_pipe_image_draw(im, surface, context, smooth,
                                    0, 0, src_w, src_h,
                                    x, y, w, h);
     }
   else
#endif
     {
        evas_common_rgba_image_scalecache_prepare
          (&im->cache_entry, surface, context, smooth,
           0, 0, src_w, src_h,
           x, y, w, h);
        evas_common_rgba_image_scalecache_do
          (&im->cache_entry, surface, context, smooth,
           0, 0, src_w, src_h,
           x, y, w, h);

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

        if (FT_HAS_FIXED_SIZES(fg->fi->src->ft.face))
          {
             if ((fg->fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR) &&
                 FT_HAS_COLOR(fg->fi->src->ft.face))
               {
                  w *= fg->fi->scale_factor;
                  h *= fg->fi->scale_factor;
               }
          }

        if ((!fg->ext_dat) && (dc->font_ext.func.gl_new))
          {
             /* extension calls */
             fg->ext_dat = dc->font_ext.func.gl_new(dc->font_ext.data, fg);
             fg->ext_dat_free = dc->font_ext.func.gl_free;
          }

        if ((!fg->ext_dat) && FT_HAS_COLOR(fg->fi->src->ft.face))
          {
             if (dc->font_ext.func.gl_image_new)
               {
                  /* extension calls */
                  fg->ext_dat = dc->font_ext.func.gl_image_new
                    (dc->font_ext.data, fg, EINA_TRUE, EVAS_COLORSPACE_ARGB8888);
                  fg->ext_dat_free = dc->font_ext.func.gl_image_free;
               }
             else
               {
                  fg->ext_dat = _evas_font_image_new(fg, EINA_TRUE, EVAS_COLORSPACE_ARGB8888);
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
                                                   chr_x, y - (chr_y - y), w, h);
                       else
                         // TODO: scale with evas_font_compress_draw.c...
                         evas_common_font_glyph_draw(fg, dc, dst, im_w,
                                                     chr_x, y - (chr_y - y), w, h,
                                                     ext_x, ext_y,
                                                     ext_w, ext_h);
                    }
                  else if ((fg->ext_dat) && FT_HAS_COLOR(fg->fi->src->ft.face))
                    {
                       if (dc->font_ext.func.gl_image_draw)
                         dc->font_ext.func.gl_image_draw
                           (dc->font_ext.data, fg->ext_dat,
                            chr_x, y - (chr_y - y), w, h, EINA_TRUE);
                       else
                         _evas_font_image_draw
                           (dc, dst, fg->ext_dat, fg,
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

// this draws a compressed font glyph and decompresses on the fly as it
// draws, saving memory bandwidth and providing speedups
EAPI void
evas_common_font_glyph_draw(RGBA_Font_Glyph *fg,
                            RGBA_Draw_Context *dc,
                            RGBA_Image *dst_image, int dst_pitch,
                            int dx, int dy, int dw, int dh, int cx, int cy, int cw, int ch)
{
   RGBA_Font_Glyph_Out *fgo = fg->glyph_out;
   int x, y, w, h, x1, x2, y1, y2, i, *iptr;
   DATA32 *dst = dst_image->image.data;
   DATA32 coltab[16], col;
   DATA16 mtab[16], v;

   // FIXME: Use dw, dh for scaling glyphs...
   (void) dw;
   (void) dh;
   x = dx;
   y = dy;
   w = fgo->bitmap.width; h = fgo->bitmap.rows;
   // skip if totally clipped out
   if ((y >= (cy + ch)) || ((y + h) <= cy) ||
       (x >= (cx + cw)) || ((x + w) <= cx)) return;
   // figure y1/y2 limit range
   y1 = 0; y2 = h;
   if ((y + y1) < cy) y1 = cy - y;
   if ((y + y2) > (cy + ch)) y2 = cy + ch - y;
   // figure x1/x2 limit range
   x1 = 0; x2 = w;
   if ((x + x1) < cx) x1 = cx - x;
   if ((x + x2) > (cx + cw)) x2 = cx + cw - x;
   col = dc->col.col;
   if (dst_image->cache_entry.space == EVAS_COLORSPACE_GRY8)
     {
        // FIXME: Font draw not optimized for Alpha targets! SLOW!
        // This is not pretty :)

        DATA8 *src8, *dst8;
        Draw_Func_Alpha func;
        int row;

        if (EINA_UNLIKELY(x < 0))
          {
             x1 += (-x);
             x = 0;
             if ((x2 - x1) <= 0) return;
          }
        if (EINA_UNLIKELY(y < 0))
          {
             y1 += (-y);
             y = 0;
             if ((y2 - y1) <= 0) return;
          }

        dst8 = dst_image->image.data8 + x + (y * dst_pitch);
        func = efl_draw_alpha_func_get(dc->render_op, EINA_FALSE);
        src8 = evas_common_font_glyph_uncompress(fg, NULL, NULL);
        if (!src8) return;

        for (row = y1; row < y2; row++)
          {
             DATA8 *d = dst8 + ((row - y1) * dst_pitch);
             DATA8 *s = src8 + (row * w) + x1;
             func(d, s, x2 - x1);
          }
        free(src8);
     }
   else if (dc->clip.mask)
     {
        RGBA_Gfx_Func func;
        DATA8 *src8, *mask;
        DATA32 *buf, *ptr, *buf_ptr;
        RGBA_Image *im = dc->clip.mask;
        int row;

        buf = alloca(sizeof(DATA32) * w * h);

        // Adjust clipping info
        if (EINA_UNLIKELY((x + x1) < dc->clip.mask_x))
          x1 = dc->clip.mask_x - x;
        if (EINA_UNLIKELY((y + y1) < dc->clip.mask_y))
          y1 = dc->clip.mask_y - y;
        if (EINA_UNLIKELY((x + x2) > (int)(x + x1 + im->cache_entry.w)))
          x2 = x1 + im->cache_entry.w;
        if (EINA_UNLIKELY((y + y2) > (int)(y + y1 + im->cache_entry.h)))
          y2 = y1 + im->cache_entry.h;

        // Step 1: alpha glyph drawing
        src8 = evas_common_font_glyph_uncompress(fg, NULL, NULL);
        if (!src8) return;

        // Step 2: color blending to buffer
        func = evas_common_gfx_func_composite_mask_color_span_get(col, dst_image->cache_entry.flags.alpha, 1, EVAS_RENDER_COPY);
        for (row = y1; row < y2; row++)
          {
             buf_ptr = buf + (row * w) + x1;
             DATA8 *s = src8 + (row * w) + x1;
             func(NULL, s, col, buf_ptr, x2 - x1);
          }
        free(src8);

        // Step 3: masking to destination
        func = evas_common_gfx_func_composite_pixel_mask_span_get(im->cache_entry.flags.alpha, im->cache_entry.flags.alpha_sparse, dst_image->cache_entry.flags.alpha, dst_pitch, dc->render_op);
        for (row = y1; row < y2; row++)
          {
             mask = im->image.data8
                + (y + row - dc->clip.mask_y) * im->cache_entry.w
                + (x + x1 - dc->clip.mask_x);

             ptr = dst + (x + x1) + ((y + row) * dst_pitch);
             buf_ptr = buf + (row * w) + x1;
             func(buf_ptr, mask, 0, ptr, w);
          }
     }
   else
     {
        // build fast multiply + mask color tables to avoid compute. this works
        // because of our very limited 4bit range of alpha values
        for (i = 0; i <= 0xf; i++)
          {
             v = (i << 4) | i;
             coltab[i] = MUL_SYM(v, col);
             mtab[i] = 256 - (coltab[i] >> 24);
          }
#ifdef BUILD_MMX
        if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
          {
#define MMX 1
#include "evas_font_compress_draw.c"
#undef MMX
          }
        else
#endif

#ifdef BUILD_NEON
        if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
          {
#define NEON 1
#include "evas_font_compress_draw.c"
#undef NEON
          }
        else
#endif

          // Plain C
          {
#include "evas_font_compress_draw.c"
          }
     }
}

