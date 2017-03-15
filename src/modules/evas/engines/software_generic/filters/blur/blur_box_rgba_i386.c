#ifdef BUILD_MMX

static inline void
_box_blur_rgba_horiz_step_mmx(const uint32_t* restrict src, int src_stride,
                              uint32_t* restrict dst, int dst_stride,
                              const int* restrict const radii,
                              Eina_Rectangle region)
{
   // TODO: implement optimized code here and remove the following line:
   _box_blur_rgba_horiz_step(src, src_stride, dst, dst_stride, radii, region);
}

static inline void
_box_blur_rgba_vert_step_mmx(const uint32_t* restrict src, int src_stride,
                             uint32_t* restrict dst, int dst_stride,
                             const int* restrict const radii,
                             Eina_Rectangle region)
{
   // TODO: implement optimized code here and remove the following line:
   _box_blur_rgba_vert_step(src, src_stride, dst, dst_stride, radii, region);
}

#endif
