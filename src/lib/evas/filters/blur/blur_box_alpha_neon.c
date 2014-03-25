#ifdef BUILD_NEON

static inline void
_box_blur_alpha_horiz_step_neon(const DATA8* restrict const srcdata,
                                DATA8* restrict const dstdata,
                                const int* restrict const radii,
                                const int len,
                                const int loops)
{
   // TODO: implement optimized code here and remove the following line:
   _box_blur_alpha_horiz_step(srcdata, dstdata, radii, len, loops);
}

static inline void
_box_blur_alpha_vert_step_neon(const DATA8* restrict const srcdata,
                               DATA8* restrict const dstdata,
                               const int* restrict const radii,
                               const int len,
                               const int loops)
{
   // TODO: implement optimized code here and remove the following line:
   _box_blur_alpha_vert_step(srcdata, dstdata, radii, len, loops);
}

#endif
