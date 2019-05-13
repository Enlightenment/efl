#include "evas_common_private.h"
#include "evas_blend_private.h"

#include "Ecore.h"

static Eina_Bool scale_rgba_in_to_out_clip_sample_internal(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

typedef struct _Evas_Scale_Thread Evas_Scale_Thread;
typedef struct _Evas_Scale_Msg Evas_Scale_Msg;

struct _Evas_Scale_Msg
{
   Eina_Thread_Queue_Msg head;
   Evas_Scale_Thread *task;
};

struct _Evas_Scale_Thread
{
   RGBA_Image *mask8;
   DATA32 **row_ptr;
   DATA32 *dptr;
   int *lin_ptr;

   RGBA_Gfx_Func func;
   RGBA_Gfx_Func func2;

   int dst_clip_x;
   int dst_clip_y;
   int dst_clip_h;
   int dst_clip_w;
   int dst_w;

   int mask_x;
   int mask_y;

   unsigned int mul_col;
};

static Eina_Bool use_thread = EINA_FALSE;
static Eina_Thread scaling_thread;
static Eina_Thread_Queue *thread_queue = NULL;
static Eina_Thread_Queue *main_queue = NULL;

EAPI Eina_Bool
evas_common_scale_rgba_in_to_out_clip_sample(RGBA_Image *src, RGBA_Image *dst,
                                             RGBA_Draw_Context *dc,
                                             int src_region_x, int src_region_y,
                                             int src_region_w, int src_region_h,
                                             int dst_region_x, int dst_region_y,
                                             int dst_region_w, int dst_region_h)
{
   return evas_common_scale_rgba_in_to_out_clip_cb
     (src, dst, dc,
      src_region_x, src_region_y, src_region_w, src_region_h,
      dst_region_x, dst_region_y, dst_region_w, dst_region_h,
      scale_rgba_in_to_out_clip_sample_internal);
}

EAPI void
evas_common_scale_rgba_in_to_out_clip_sample_do(const Cutout_Rects *reuse,
                                                const Eina_Rectangle *clip,
                                                RGBA_Image *src, RGBA_Image *dst,
                                                RGBA_Draw_Context *dc,
                                                int src_region_x, int src_region_y,
                                                int src_region_w, int src_region_h,
                                                int dst_region_x, int dst_region_y,
                                                int dst_region_w, int dst_region_h)
{
   Eina_Rectangle area;
   Cutout_Rect *r;
   int i;

   if (!reuse)
     {
        evas_common_draw_context_clip_clip(dc, clip->x, clip->y, clip->w, clip->h);
        scale_rgba_in_to_out_clip_sample_internal(src, dst, dc,
                                                  src_region_x, src_region_y,
                                                  src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y,
                                                  dst_region_w, dst_region_h);
        return;
     }

   for (i = 0; i < reuse->active; ++i)
     {
        r = reuse->rects + i;

        EINA_RECTANGLE_SET(&area, r->x, r->y, r->w, r->h);
        if (!eina_rectangle_intersection(&area, clip)) continue ;
        evas_common_draw_context_set_clip(dc, area.x, area.y, area.w, area.h);
        scale_rgba_in_to_out_clip_sample_internal(src, dst, dc,
                                                  src_region_x, src_region_y,
                                                  src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y,
                                                  dst_region_w, dst_region_h);
     }
}

static void
_evas_common_scale_rgba_sample_scale_nomask(int y,
                                            int dst_clip_w, int dst_clip_h, int dst_w,
                                            DATA32 **row_ptr, int *lin_ptr,
                                            DATA32 *dptr, RGBA_Gfx_Func func, unsigned int mul_col)
{
   DATA32 *buf, *dst_ptr;
   int x;

   /* a scanline buffer */
   buf = alloca(dst_clip_w * sizeof(DATA32));

   dptr = dptr + dst_w * y;
   for (; y < dst_clip_h; y++)
     {
        dst_ptr = buf;
        for (x = 0; x < dst_clip_w; x++)
          {
             DATA32 *ptr;

             ptr = row_ptr[y] + lin_ptr[x];
             *dst_ptr = *ptr;
             dst_ptr++;
          }

        /* * blend here [clip_w *] buf -> dptr * */
        func(buf, NULL, mul_col, dptr, dst_clip_w);

        dptr += dst_w;
     }
}

static void
_evas_common_scale_rgba_sample_scale_mask(int y,
                                          int dst_clip_x, int dst_clip_y,
                                          int dst_clip_w, int dst_clip_h, int dst_w,
                                          int mask_x, int mask_y,
                                          DATA32 **row_ptr, int *lin_ptr, RGBA_Image *mask_ie,
                                          DATA32 *dptr, RGBA_Gfx_Func func, RGBA_Gfx_Func func2,
                                          unsigned int mul_col)
{
   DATA32 *buf, *dst_ptr;
   int x;

   /* a scanline buffer */
   buf = alloca(dst_clip_w * sizeof(DATA32));

   /* clamp/map to mask geometry */
   if (EINA_UNLIKELY(dst_clip_x < mask_x))
     dst_clip_x = mask_x;
   if (EINA_UNLIKELY(dst_clip_y < mask_y))
     dst_clip_y = mask_y;
   if (EINA_UNLIKELY(dst_clip_x + dst_clip_w > mask_x + (int)mask_ie->cache_entry.w))
     dst_clip_w = mask_x + mask_ie->cache_entry.w - dst_clip_x;
   if (EINA_UNLIKELY(dst_clip_y + dst_clip_h > mask_y + (int)mask_ie->cache_entry.h))
     dst_clip_h = mask_y + mask_ie->cache_entry.h - dst_clip_y;

   dptr = dptr + dst_w * y;
   for (; y < dst_clip_h; y++)
     {
        DATA8 *mask;

        dst_ptr = buf;
        mask = mask_ie->image.data8
          + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
          + (dst_clip_x - mask_x);

        for (x = 0; x < dst_clip_w; x++)
          {
             DATA32 *ptr;

             ptr = row_ptr[y] + lin_ptr[x];
             *dst_ptr = *ptr;
             dst_ptr++;
          }

        /* * blend here [clip_w *] buf -> dptr * */
        if (mul_col != 0xFFFFFFFF) func2(buf, NULL, mul_col, buf, dst_clip_w);
        func(buf, mask, 0, dptr, dst_clip_w);

        dptr += dst_w;
     }
}

EAPI void
evas_common_scale_rgba_sample_draw(RGBA_Image *src, RGBA_Image *dst, int dst_clip_x, int dst_clip_y, int dst_clip_w, int dst_clip_h, DATA32 mul_col, int render_op, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h, RGBA_Image *mask_ie, int mask_x, int mask_y)
{
   int      x, y;
   int     *lin_ptr;
   DATA32  *buf, *dptr;
   DATA32 **row_ptr;
   DATA32  *ptr, *dst_ptr, *src_data, *dst_data;
   DATA8   *mask;
   int      src_w, src_h, dst_w, dst_h;
   RGBA_Gfx_Func func, func2 = NULL;

   if ((!src->image.data) || (!dst->image.data)) return;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h,
                         0, 0, dst->cache_entry.w, dst->cache_entry.h))) return;
   if (!(RECTS_INTERSECT(src_region_x, src_region_y, src_region_w, src_region_h,
                         0, 0, src->cache_entry.w, src->cache_entry.h))) return;

   if ((src_region_w <= 0) || (src_region_h <= 0) ||
       (dst_region_w <= 0) || (dst_region_h <= 0)) return;

   src_w = src->cache_entry.w;
   if (src_region_x >= src_w) return;

   src_h = src->cache_entry.h;
   if (src_region_y >= src_h) return;

   dst_w = dst->cache_entry.w;
   dst_h = dst->cache_entry.h;

   src_data = src->image.data;
   dst_data = dst->image.data;

   /* sanitise clip x */
   if (dst_clip_x < 0)
     {
        dst_clip_w += dst_clip_x;
        dst_clip_x = 0;
     }

   if ((dst_clip_x + dst_clip_w) > dst_w)
     dst_clip_w = dst_w - dst_clip_x;

   if (dst_clip_x < dst_region_x)
     {
        dst_clip_w += dst_clip_x - dst_region_x;
        dst_clip_x = dst_region_x;
     }

   if (dst_clip_x >= dst_w) return;

   if ((dst_clip_x + dst_clip_w) > (dst_region_x + dst_region_w))
     dst_clip_w = dst_region_x + dst_region_w - dst_clip_x;

   if (dst_clip_w <= 0) return;

   /* sanitise clip y */
   if (dst_clip_y < 0)
     {
        dst_clip_h += dst_clip_y;
        dst_clip_y = 0;
     }

   if ((dst_clip_y + dst_clip_h) > dst_h)
     dst_clip_h = dst_h - dst_clip_y;

   if (dst_clip_y < dst_region_y)
     {
        dst_clip_h += dst_clip_y - dst_region_y;
        dst_clip_y = dst_region_y;
     }

   if (dst_clip_y >= dst_h) return;

   if ((dst_clip_y + dst_clip_h) > (dst_region_y + dst_region_h))
     dst_clip_h = dst_region_y + dst_region_h - dst_clip_y;

   if (dst_clip_h <= 0) return;

   /* sanitise region x */
   if (src_region_x < 0)
     {
        dst_region_x -= (src_region_x * dst_region_w) / src_region_w;
        dst_region_w += (src_region_x * dst_region_w) / src_region_w;
        src_region_w += src_region_x;
        src_region_x = 0;

        if (dst_clip_x < dst_region_x)
          {
             dst_clip_w += (dst_clip_x - dst_region_x);
             dst_clip_x = dst_region_x;
          }
     }

   if ((dst_clip_x + dst_clip_w) > dst_w)
     dst_clip_w = dst_w - dst_clip_x;

   if (dst_clip_w <= 0) return;

   if ((src_region_x + src_region_w) > src_w)
     {
        dst_region_w = (dst_region_w * (src_w - src_region_x)) / (src_region_w);
        src_region_w = src_w - src_region_x;
     }

   if ((dst_region_w <= 0) || (src_region_w <= 0)) return;

   /* sanitise region y */
   if (src_region_y < 0)
     {
        dst_region_y -= (src_region_y * dst_region_h) / src_region_h;
        dst_region_h += (src_region_y * dst_region_h) / src_region_h;
        src_region_h += src_region_y;
        src_region_y = 0;

        if (dst_clip_y < dst_region_y)
          {
             dst_clip_h += (dst_clip_y - dst_region_y);
             dst_clip_y = dst_region_y;
          }
     }

   if ((dst_clip_y + dst_clip_h) > dst_h)
     dst_clip_h = dst_h - dst_clip_y;

   if (dst_clip_h <= 0) return;

   if ((src_region_y + src_region_h) > src_h)
     {
        dst_region_h = (dst_region_h * (src_h - src_region_y)) / (src_region_h);
        src_region_h = src_h - src_region_y;
     }

   if ((dst_region_h <= 0) || (src_region_h <= 0)) return;

   /* figure out dst jump */
   //dst_jump = dst_w - dst_clip_w;

   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_w);

   if (!mask_ie)
     {
         if (mul_col != 0xffffffff)
           func = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, mul_col, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
         else
           func = evas_common_gfx_func_composite_pixel_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
     }
   else
     {
        if (mul_col != 0xffffffff)
          {
             func = evas_common_gfx_func_composite_pixel_mask_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
             func2 = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, mul_col, dst->cache_entry.flags.alpha, dst_clip_w, EVAS_RENDER_COPY);
          }
        else
          func = evas_common_gfx_func_composite_pixel_mask_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);

        /* clamp/map to mask geometry */
        if (EINA_UNLIKELY(dst_clip_x < mask_x))
          dst_clip_x = mask_x;
        if (EINA_UNLIKELY(dst_clip_y < mask_y))
          dst_clip_y = mask_y;
        if (EINA_UNLIKELY(dst_clip_x + dst_clip_w > mask_x + (int)mask_ie->cache_entry.w))
          dst_clip_w = mask_x + mask_ie->cache_entry.w - dst_clip_x;
        if (EINA_UNLIKELY(dst_clip_y + dst_clip_h > mask_y + (int)mask_ie->cache_entry.h))
          dst_clip_h = mask_y + mask_ie->cache_entry.h - dst_clip_y;
     }

   if ((dst_region_w == src_region_w) && (dst_region_h == src_region_h))
     {
        ptr = src_data + (((dst_clip_y - dst_region_y) + src_region_y) * src_w) + ((dst_clip_x - dst_region_x) + src_region_x);

        /* image masking */
        if (mask_ie)
          {
             if (mul_col != 0xffffffff)
               buf = alloca(dst_clip_w * sizeof(DATA32));

             for (y = 0; y < dst_clip_h; y++)
               {
                  mask = mask_ie->image.data8
                     + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                     + (dst_clip_x - mask_x);

                  /* * blend here [clip_w *] ptr -> dst_ptr * */
                  if (mul_col != 0xffffffff)
                    {
                       func2(ptr, NULL, mul_col, buf, dst_clip_w);
                       func(buf, mask, 0, dst_ptr, dst_clip_w);
                    }
                  else
                    func(ptr, mask, 0, dst_ptr, dst_clip_w);

                  ptr += src_w;
                  dst_ptr += dst_w;
               }
          }
        else
          {
             for (y = 0; y < dst_clip_h; y++)
               {
                  /* * blend here [clip_w *] ptr -> dst_ptr * */
                  func(ptr, NULL, mul_col, dst_ptr, dst_clip_w);

                  ptr += src_w;
                  dst_ptr += dst_w;
               }
          }
     }
   else
     {
        /* allocate scale lookup tables */
        lin_ptr = alloca(dst_clip_w * sizeof(int));
        row_ptr = alloca(dst_clip_h * sizeof(DATA32 *));

        /* fill scale tables */
        for (x = 0; x < dst_clip_w; x++)
          lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
        for (y = 0; y < dst_clip_h; y++)
          row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / dst_region_h)
                                    + src_region_y) * src_w);

        /* scale to dst */
        dptr = dst_ptr;

        /* a scanline buffer */
        buf = alloca(dst_clip_w * sizeof(DATA32));

        /* image masking */
        if (mask_ie)
          {
             for (y = 0; y < dst_clip_h; y++)
               {
                  dst_ptr = buf;
                  mask = mask_ie->image.data8
                     + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                     + (dst_clip_x - mask_x);

                  for (x = 0; x < dst_clip_w; x++)
                    {
                       ptr = row_ptr[y] + lin_ptr[x];
                       *dst_ptr = *ptr;
                       dst_ptr++;
                    }

                  /* * blend here [clip_w *] buf -> dptr * */
                  if (mul_col != 0xffffffff)
                    func2(buf, NULL, mul_col, buf, dst_clip_w);
                  func(buf, mask, 0, dptr, dst_clip_w);

                  dptr += dst_w;
               }
          }
        else
          {
             for (y = 0; y < dst_clip_h; y++)
               {
                  dst_ptr = buf;

                  for (x = 0; x < dst_clip_w; x++)
                    {
                       ptr = row_ptr[y] + lin_ptr[x];
                       *dst_ptr = *ptr;
                       dst_ptr++;
                    }

                  /* * blend here [clip_w *] buf -> dptr * */
                  func(buf, NULL, mul_col, dptr, dst_clip_w);

                  dptr += dst_w;
               }
          }
     }
}

static Eina_Bool
scale_rgba_in_to_out_clip_sample_internal(RGBA_Image *src, RGBA_Image *dst,
                                         RGBA_Draw_Context *dc,
                                         int src_region_x, int src_region_y,
                                         int src_region_w, int src_region_h,
                                         int dst_region_x, int dst_region_y,
                                         int dst_region_w, int dst_region_h)
{
   int      x, y;
   int     *lin_ptr;
   DATA32  *buf = NULL, *dptr;
   DATA32 **row_ptr;
   DATA32  *ptr, *dst_ptr, *src_data, *dst_data;
   DATA8   *mask;
   int      dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h;
   int      src_w, src_h, dst_w, dst_h, mask_x, mask_y;
   RGBA_Gfx_Func func, func2 = NULL;
   RGBA_Image *mask_ie = dc->clip.mask;

   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return EINA_FALSE;
   if (!(RECTS_INTERSECT(src_region_x, src_region_y, src_region_w, src_region_h, 0, 0, src->cache_entry.w, src->cache_entry.h)))
     return EINA_FALSE;

   src_w = src->cache_entry.w;
   src_h = src->cache_entry.h;
   dst_w = dst->cache_entry.w;
   dst_h = dst->cache_entry.h;

   src_data = src->image.data;
   dst_data = dst->image.data;

   mask_x = dc->clip.mask_x;
   mask_y = dc->clip.mask_y;

   if (dc->clip.use)
     {
        dst_clip_x = dc->clip.x;
        dst_clip_y = dc->clip.y;
        dst_clip_w = dc->clip.w;
        dst_clip_h = dc->clip.h;
        if (dst_clip_x < 0)
          {
             dst_clip_w += dst_clip_x;
             dst_clip_x = 0;
          }
        if (dst_clip_y < 0)
          {
             dst_clip_h += dst_clip_y;
             dst_clip_y = 0;
          }
        if ((dst_clip_x + dst_clip_w) > dst_w)
          dst_clip_w = dst_w - dst_clip_x;
        if ((dst_clip_y + dst_clip_h) > dst_h)
          dst_clip_h = dst_h - dst_clip_y;
     }
   else
     {
        dst_clip_x = 0;
        dst_clip_y = 0;
        dst_clip_w = dst_w;
        dst_clip_h = dst_h;
     }

   if (dst_clip_x < dst_region_x)
     {
        dst_clip_w += dst_clip_x - dst_region_x;
        dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > (dst_region_x + dst_region_w))
     dst_clip_w = dst_region_x + dst_region_w - dst_clip_x;
   if (dst_clip_y < dst_region_y)
     {
        dst_clip_h += dst_clip_y - dst_region_y;
        dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > (dst_region_y + dst_region_h))
     dst_clip_h = dst_region_y + dst_region_h - dst_clip_y;

   if ((src_region_w <= 0) || (src_region_h <= 0) ||
       (dst_region_w <= 0) || (dst_region_h <= 0) ||
       (dst_clip_w <= 0) || (dst_clip_h <= 0))
     return EINA_FALSE;

   /* sanitise x */
   if (src_region_x < 0)
     {
        dst_region_x -= (src_region_x * dst_region_w) / src_region_w;
        dst_region_w += (src_region_x * dst_region_w) / src_region_w;
        src_region_w += src_region_x;
        src_region_x = 0;
     }
   if (src_region_x >= src_w) return EINA_FALSE;
   if ((src_region_x + src_region_w) > src_w)
     {
        dst_region_w = (dst_region_w * (src_w - src_region_x)) / (src_region_w);
        src_region_w = src_w - src_region_x;
     }
   if (dst_region_w <= 0) return EINA_FALSE;
   if (src_region_w <= 0) return EINA_FALSE;
   if (dst_clip_x >= dst_w) return EINA_FALSE;
   if (dst_clip_x < dst_region_x)
     {
        dst_clip_w += (dst_clip_x - dst_region_x);
        dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > dst_w)
     {
        dst_clip_w = dst_w - dst_clip_x;
     }
   if (dst_clip_w <= 0) return EINA_FALSE;

   /* sanitise y */
   if (src_region_y < 0)
     {
        dst_region_y -= (src_region_y * dst_region_h) / src_region_h;
        dst_region_h += (src_region_y * dst_region_h) / src_region_h;
        src_region_h += src_region_y;
        src_region_y = 0;
     }
   if (src_region_y >= src_h) return EINA_FALSE;
   if ((src_region_y + src_region_h) > src_h)
     {
        dst_region_h = (dst_region_h * (src_h - src_region_y)) / (src_region_h);
        src_region_h = src_h - src_region_y;
     }
   if (dst_region_h <= 0) return EINA_FALSE;
   if (src_region_h <= 0) return EINA_FALSE;
   if (dst_clip_y >= dst_h) return EINA_FALSE;
   if (dst_clip_y < dst_region_y)
     {
        dst_clip_h += (dst_clip_y - dst_region_y);
        dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > dst_h)
     {
        dst_clip_h = dst_h - dst_clip_y;
     }
   if (dst_clip_h <= 0) return EINA_FALSE;

   /* figure out dst jump */
   //dst_jump = dst_w - dst_clip_w;

   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_w);

   if (!mask_ie)
     {
        if (dc->mul.use)
          func = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dc->mul.col, dst->cache_entry.flags.alpha, dst_clip_w, dc->render_op);
        else
          func = evas_common_gfx_func_composite_pixel_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, dc->render_op);
     }
   else
     {
        func = evas_common_gfx_func_composite_pixel_mask_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, dc->render_op);
        if (dc->mul.use)
          func2 = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dc->mul.col, dst->cache_entry.flags.alpha, dst_clip_w, EVAS_RENDER_COPY);
        /* clamp/map to mask geometry */
        if (EINA_UNLIKELY(dst_clip_x < mask_x))
          dst_clip_x = mask_x;
        if (EINA_UNLIKELY(dst_clip_y < mask_y))
          dst_clip_y = mask_y;
        if (EINA_UNLIKELY(dst_clip_x + dst_clip_w > mask_x + (int)mask_ie->cache_entry.w))
          dst_clip_w = mask_x + mask_ie->cache_entry.w - dst_clip_x;
        if (EINA_UNLIKELY(dst_clip_y + dst_clip_h > mask_y + (int)mask_ie->cache_entry.h))
          dst_clip_h = mask_y + mask_ie->cache_entry.h - dst_clip_y;
     }

   if ((dst_region_w == src_region_w) && (dst_region_h == src_region_h))
     {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_IMAGE_SCALE_SAMPLE
        if ((src->pixman.im) && (dst->pixman.im) && (!dc->clip.mask) &&
            ((!dc->mul.use) || ((dc->mul.use) && (dc->mul.col == 0xffffffff))) &&
            ((dc->render_op == _EVAS_RENDER_COPY) ||
             (dc->render_op == _EVAS_RENDER_BLEND)))
          {
             pixman_op_t op = PIXMAN_OP_SRC; // _EVAS_RENDER_COPY
             if (dc->render_op == _EVAS_RENDER_BLEND)
               op = PIXMAN_OP_OVER;

             pixman_image_composite(op,
                                    src->pixman.im, NULL,
                                    dst->pixman.im,
                                    (dst_clip_x - dst_region_x) + src_region_x,
                                    (dst_clip_y - dst_region_y) + src_region_y,
                                    0, 0,
                                    dst_clip_x, dst_clip_y,
                                    dst_clip_w, dst_clip_h);
          }
        else
# endif
#endif
          {
             ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;

             /* image masking */
             if (mask_ie)
               {
                  if (dc->mul.use)
                    buf = alloca(dst_clip_w * sizeof(DATA32));

                  for (y = 0; y < dst_clip_h; y++)
                    {
                       mask = mask_ie->image.data8
                          + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                          + (dst_clip_x - mask_x);

                       /* * blend here [clip_w *] ptr -> dst_ptr * */
                       if (dc->mul.use)
                         {
                            func2(ptr, NULL, dc->mul.col, buf, dst_clip_w);
                            func(buf, mask, 0, dst_ptr, dst_clip_w);
                         }
                       else
                         func(ptr, mask, 0, dst_ptr, dst_clip_w);

                       ptr += src_w;
                       dst_ptr += dst_w;
                    }
               }
             else
               {
                  for (y = 0; y < dst_clip_h; y++)
                    {
                       /* * blend here [clip_w *] ptr -> dst_ptr * */
                       func(ptr, NULL, dc->mul.col, dst_ptr, dst_clip_w);

                       ptr += src_w;
                       dst_ptr += dst_w;
                    }
               }
          }
     }
   else
     {
        /* allocate scale lookup tables */
        lin_ptr = alloca(dst_clip_w * sizeof(int));
        row_ptr = alloca(dst_clip_h * sizeof(DATA32 *));

        /* fill scale tables */
        for (x = 0; x < dst_clip_w; x++)
          lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
        for (y = 0; y < dst_clip_h; y++)
          row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / dst_region_h)
                                    + src_region_y) * src_w);
        /* scale to dst */
        dptr = dst_ptr;
#ifdef DIRECT_SCALE
        if ((!src->cache_entry.flags.alpha) &&
            (!dst->cache_entry.flags.alpha) &&
            (!dc->mul.use) &&
            (!dc->clip.mask))
          {
             for (y = 0; y < dst_clip_h; y++)
               {

                  dst_ptr = dptr;
                  for (x = 0; x < dst_clip_w; x++)
                    {
                       ptr = row_ptr[y] + lin_ptr[x];
                       *dst_ptr = *ptr;
                       dst_ptr++;
                    }

                  dptr += dst_w;
               }
          }
        else
#endif
          {
             unsigned int mul_col;

             mul_col = dc->mul.use ? dc->mul.col : 0xFFFFFFFF;

             /* do we have enough data to start some additional thread ? */
             if (use_thread && dst_clip_h > 32 && dst_clip_w * dst_clip_h > 4096)
               {
                  /* Yes, we do ! */
                  Evas_Scale_Msg *msg;
                  void *ref;
                  Evas_Scale_Thread local;

                  local.mask8 = dc->clip.mask;
                  local.row_ptr = row_ptr;
                  local.dptr = dptr;
                  local.lin_ptr = lin_ptr;
                  local.func = func;
                  local.func2 = func2;
                  local.dst_clip_x = dst_clip_x;
                  local.dst_clip_y = dst_clip_y;
                  local.dst_clip_h = dst_clip_h;
                  local.dst_clip_w = dst_clip_w;
                  local.dst_w = dst_w;
                  local.mask_x = mask_x;
                  local.mask_y = mask_y;
                  local.mul_col = mul_col;

                  msg = eina_thread_queue_send(thread_queue, sizeof (Evas_Scale_Msg), &ref);
                  msg->task = &local;
                  eina_thread_queue_send_done(thread_queue, ref);

                  /* image masking */
                  if (dc->clip.mask)
                    {
                       _evas_common_scale_rgba_sample_scale_mask(0,
                                                                 dst_clip_x, dst_clip_y,
                                                                 dst_clip_w, dst_clip_h >> 1, dst_w,
                                                                 dc->clip.mask_x, dc->clip.mask_y,
                                                                 row_ptr, lin_ptr, dc->clip.mask,
                                                                 dptr, func, func2, mul_col);

                    }
                  else
                    {
                       _evas_common_scale_rgba_sample_scale_nomask(0,
                                                                   dst_clip_w, dst_clip_h >> 1, dst_w,
                                                                   row_ptr, lin_ptr,
                                                                   dptr, func, mul_col);
                    }

                  msg = eina_thread_queue_wait(main_queue, &ref);
                  if (msg) eina_thread_queue_wait_done(main_queue, ref);
               }
             else
               {
                  /* No we don't ! */

                  /* image masking */
                  if (dc->clip.mask)
                    {
                       _evas_common_scale_rgba_sample_scale_mask(0,
                                                                 dst_clip_x, dst_clip_y,
                                                                 dst_clip_w, dst_clip_h, dst_w,
                                                                 dc->clip.mask_x, dc->clip.mask_y,
                                                                 row_ptr, lin_ptr, dc->clip.mask,
                                                                 dptr, func, func2, mul_col);

                    }
                  else
                    {
                       _evas_common_scale_rgba_sample_scale_nomask(0,
                                                                   dst_clip_w, dst_clip_h, dst_w,
                                                                   row_ptr, lin_ptr,
                                                                   dptr, func, mul_col);
                    }
               }
          }
     }

   return EINA_TRUE;
}

static void *
_evas_common_scale_sample_thread(void *data EINA_UNUSED,
                                 Eina_Thread t EINA_UNUSED)
{
   Evas_Scale_Msg *msg;
   Evas_Scale_Thread *todo = NULL;

   eina_thread_name_set(eina_thread_self(), "Evas-scale-sam");
   do
     {
        void *ref;

        todo = NULL;

        msg = eina_thread_queue_wait(thread_queue, &ref);
        if (msg)
          {
             int h;

             todo = msg->task;
             eina_thread_queue_wait_done(thread_queue, ref);

             if (!todo) goto end;

             h = todo->dst_clip_h >> 1;

             if (todo->mask8)
               _evas_common_scale_rgba_sample_scale_mask(h,
                                                         todo->dst_clip_x, todo->dst_clip_y,
                                                         todo->dst_clip_w, todo->dst_clip_h,
                                                         todo->dst_w,
                                                         todo->mask_x, todo->mask_y,
                                                         todo->row_ptr, todo->lin_ptr, todo->mask8,
                                                         todo->dptr, todo->func, todo->func2,
                                                         todo->mul_col);
             else
               _evas_common_scale_rgba_sample_scale_nomask(h,
                                                           todo->dst_clip_w, todo->dst_clip_h,
                                                           todo->dst_w,
                                                           todo->row_ptr, todo->lin_ptr,
                                                           todo->dptr, todo->func, todo->mul_col);
          }

     end:
        msg = eina_thread_queue_send(main_queue, sizeof (Evas_Scale_Msg), &ref);
        msg->task = NULL;
        eina_thread_queue_send_done(main_queue, ref);
     }
   while (todo);

   return NULL;
}

static void
evas_common_scale_sample_fork_reset(void *data EINA_UNUSED)
{
   eina_thread_queue_free(thread_queue);
   eina_thread_queue_free(main_queue);

   thread_queue = eina_thread_queue_new();
   main_queue = eina_thread_queue_new();

   if (!eina_thread_create(&scaling_thread, EINA_THREAD_NORMAL, -1,
                           _evas_common_scale_sample_thread, NULL))
     {
        CRI("We failed to recreate the upscaling thread.");
        use_thread = EINA_FALSE;
     }
}

EAPI void
evas_common_scale_sample_init(void)
{
   if (eina_cpu_count() <= 2) return ;

//Eina_Thread_Queue doesn't work on WIN32.
#ifdef _WIN32
   return;
#endif

   ecore_fork_reset_callback_add(evas_common_scale_sample_fork_reset, NULL);

   thread_queue = eina_thread_queue_new();
   if (EINA_UNLIKELY(!thread_queue))
     {
        ERR("Failed to create thread queue");
        goto cleanup;
     }
   main_queue = eina_thread_queue_new();
   if (EINA_UNLIKELY(!thread_queue))
     {
        ERR("Failed to create thread queue");
        goto cleanup;
     }

   if (!eina_thread_create(&scaling_thread, EINA_THREAD_NORMAL, -1,
                           _evas_common_scale_sample_thread, NULL))
     {
        CRI("We failed to create the upscaling thread.");
        goto cleanup;
     }

   use_thread = EINA_TRUE;
   return;

cleanup:
   if (thread_queue) eina_thread_queue_free(thread_queue);
   if (main_queue) eina_thread_queue_free(main_queue);
}

EAPI void
evas_common_scale_sample_shutdown(void)
{
   Evas_Scale_Msg *msg;
   void *ref;

   if (!use_thread) return ;

   ecore_fork_reset_callback_del(evas_common_scale_sample_fork_reset, NULL);

   msg = eina_thread_queue_send(thread_queue, sizeof (Evas_Scale_Msg), &ref);
   msg->task = NULL;
   eina_thread_queue_send_done(thread_queue, ref);

   /* Here is the thread commiting succide*/

   msg = eina_thread_queue_wait(main_queue, &ref);
   if (msg) eina_thread_queue_wait_done(main_queue, ref);

   eina_thread_join(scaling_thread);

   eina_thread_queue_free(thread_queue);
   eina_thread_queue_free(main_queue);
}
