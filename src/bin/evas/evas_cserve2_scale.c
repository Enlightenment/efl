#include "evas_common_private.h"
#include "evas_private.h"

void
cserve2_scale_init(void)
{
   evas_common_cpu_init();
   evas_common_blend_init();
   evas_common_image_init();
   evas_common_convert_init();
   evas_common_scale_init();
}

void
cserve2_scale_shutdown(void)
{
   evas_common_image_shutdown();
}

static inline void
_cserve2_rgba_image_set(RGBA_Image *im, void *data, int w, int h, int alpha)
{
   memset(im, 0, sizeof *im);

   im->cache_entry.w = w;
   im->cache_entry.h = h;
   im->cache_entry.space = EVAS_COLORSPACE_ARGB8888;
   im->cache_entry.flags.alpha = alpha;
   im->image.data = data;
   im->cache_entry.allocated.w = w;
   im->cache_entry.allocated.h = h;
}

void
cserve2_rgba_image_scale_do(void *src_data, int src_full_w, int src_full_h,
                            void *dst_data,
                            int src_x, int src_y, int src_w, int src_h,
                            int dst_x, int dst_y, int dst_w, int dst_h,
                            int alpha, int smooth)
{
   RGBA_Image src, dst;
   RGBA_Draw_Context ct;

   _cserve2_rgba_image_set(&src, src_data, src_full_w, src_full_h, alpha);
   _cserve2_rgba_image_set(&dst, dst_data, dst_w, dst_h, alpha);
   dst.flags = RGBA_IMAGE_NOTHING;

   memset(&ct, 0, sizeof(ct));
   ct.sli.h = 1;
   ct.render_op = _EVAS_RENDER_COPY;

   if (smooth)
     evas_common_scale_rgba_in_to_out_clip_smooth(&src, &dst, &ct,
                                                  src_x, src_y, src_w, src_h,
                                                  dst_x, dst_y, dst_w, dst_h);
   else
     evas_common_scale_rgba_in_to_out_clip_sample(&src, &dst, &ct,
                                                  src_x, src_y, src_w, src_h,
                                                  dst_x, dst_y, dst_w, dst_h);
}
