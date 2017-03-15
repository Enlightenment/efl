#ifdef BUILD_NEON

static inline void
_box_blur_alpha_horiz_step_neon(const uint8_t* restrict src, int src_stride,
                                uint8_t* restrict dst, int dst_stride,
                                const int* restrict const radii,
                                Eina_Rectangle region)
{
   // TODO: implement optimized code here and remove the following line:
   _box_blur_alpha_horiz_step(src, src_stride, dst, dst_stride, radii, region);
}

static inline void
_box_blur_alpha_vert_step_neon(const uint8_t* restrict src, int src_stride,
                               uint8_t* restrict dst, int dst_stride,
                               const int* restrict const radii,
                               Eina_Rectangle region)
{
   // TODO: implement optimized code here and remove the following line:
   _box_blur_alpha_vert_step(src, src_stride, dst, dst_stride, radii, region);
}

#endif
