#ifndef EVAS_BLEND_OPS_H
#define EVAS_BLEND_OPS_H

#define BLEND_RGBA_RGBA_C(src, dst) \
{ \
   DATA32 __tmp; \
   DATA8 __a; \
\
   if (A_VAL(src)) /* hmmm - do we need this? */ \
     { \
	__a = _evas_pow_lut[A_VAL(src)][A_VAL(dst)]; \
	BLEND_COLOR(A_VAL(src), A_VAL(dst), \
		    255, A_VAL(dst), \
		    __tmp); \
	BLEND_COLOR(__a, R_VAL(dst), \
		    R_VAL(src), R_VAL(dst), \
		    __tmp); \
	BLEND_COLOR(__a, G_VAL(dst), \
		    G_VAL(src), G_VAL(dst), \
		    __tmp); \
	BLEND_COLOR(__a, B_VAL(dst), \
		    B_VAL(src), B_VAL(dst), \
		    __tmp); \
     } \
}

#define BLEND_RGBA_RGB_C(src, dst) \
{ \
   DATA32 __tmp; \
\
   if (A_VAL(src)) /* hmmm - do we need this? */ \
     { \
	BLEND_COLOR(A_VAL(src), R_VAL(dst), \
		    R_VAL(src), R_VAL(dst), \
		    __tmp); \
	BLEND_COLOR(A_VAL(src), G_VAL(dst), \
		    G_VAL(src), G_VAL(dst), \
		    __tmp); \
	BLEND_COLOR(A_VAL(src), B_VAL(dst), \
		    B_VAL(src), B_VAL(dst), \
		    __tmp); \
     } \
}

#define BLEND_RGB_RGBA_C(src, dst) \
{ \
   *dst = *src | PIXEL_SOLID_ALPHA; \
}

#define BLEND_RGB_RGB_C(src, dst) \
{ \
   *dst = *src; \
}






#define BLEND_RGBA_RGBA_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   DATA32 __tmp; \
\
   movd_r2m(src_mmx, __tmp); \
   BLEND_RGBA_RGBA_C(&__tmp, dst); \
}

#define BLEND_RGBA_RGBA_MMX_Z_C1(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   BLEND_RGBA_RGBA_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1); \
}

#define BLEND_RGBA_RGBA_MMX_Z(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   BLEND_RGBA_RGBA_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1); \
}





#define BLEND_RGBA_RGB_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   movd_m2r(dst[0], mmx1); \
\
   movq_r2r(src_mmx, mmx2); \
   punpcklbw_r2r(mmx2, mmx2); \
   punpckhwd_r2r(mmx2, mmx2); \
   punpckhdq_r2r(mmx2, mmx2); \
   psrlw_i2r(1, mmx2); \
\
   psrlq_i2r(16, mmx2); \
\
   pxor_r2r(mmxz, mmxz); \
   punpcklbw_r2r(mmxz, src_mmx); \
   punpcklbw_r2r(mmxz, mmx1); \
\
   psubw_r2r(mmx1, src_mmx); \
   psllw_i2r(1, src_mmx); \
   movq_m2r(*_evas_const_c1, mmxc1); \
   paddw_r2r(mmxc1, src_mmx); \
   pmulhw_r2r(mmx2, src_mmx); \
   paddw_r2r(src_mmx, mmx1); \
\
   packuswb_r2r(mmxz, mmx1); \
   movd_r2m(mmx1, dst[0]); \
}

#define BLEND_RGBA_RGB_MMX_Z_C1(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   movd_m2r(dst[0], mmx1); \
\
   movq_r2r(src_mmx, mmx2); \
   punpcklbw_r2r(mmx2, mmx2); \
   punpckhwd_r2r(mmx2, mmx2); \
   punpckhdq_r2r(mmx2, mmx2); \
   psrlw_i2r(1, mmx2); \
\
   psrlq_i2r(16, mmx2); \
\
   punpcklbw_r2r(mmxz, src_mmx); \
   punpcklbw_r2r(mmxz, mmx1); \
\
   psubw_r2r(mmx1, src_mmx); \
   psllw_i2r(1, src_mmx); \
   paddw_r2r(mmxc1, src_mmx); \
   pmulhw_r2r(mmx2, src_mmx); \
   paddw_r2r(src_mmx, mmx1); \
\
   packuswb_r2r(mmxz, mmx1); \
   movd_r2m(mmx1, dst[0]); \
}

#define BLEND_RGBA_RGB_MMX_Z(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   movd_m2r(dst[0], mmx1); \
\
   movq_r2r(src_mmx, mmx2); \
   punpcklbw_r2r(mmx2, mmx2); \
   punpckhwd_r2r(mmx2, mmx2); \
   punpckhdq_r2r(mmx2, mmx2); \
   psrlw_i2r(1, mmx2); \
\
   psrlq_i2r(16, mmx2); \
\
   punpcklbw_r2r(mmxz, src_mmx); \
   punpcklbw_r2r(mmxz, mmx1); \
\
   psubw_r2r(mmx1, src_mmx); \
   psllw_i2r(1, src_mmx); \
   movq_m2r(*_evas_const_c1, mmxc1); \
   paddw_r2r(mmxc1, src_mmx); \
   pmulhw_r2r(mmx2, src_mmx); \
   paddw_r2r(src_mmx, mmx1); \
\
   packuswb_r2r(mmxz, mmx1); \
   movd_r2m(mmx1, dst[0]); \
}





#define BLEND_RGB_RGBA_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   DATA32 __tmp; \
\
   movd_r2m(src_mmx, __tmp); \
   BLEND_RGB_RGBA_C(&__tmp, dst); \
}

#define BLEND_RGB_RGBA_MMX_Z_C1(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   BLEND_RGB_RGBA_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1); \
}

#define BLEND_RGB_RGBA_MMX_Z(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   BLEND_RGB_RGBA_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1); \
}





#define BLEND_RGB_RGB_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   movd_r2m(src_mmx, dst[0]); \
}

#define BLEND_RGB_RGB_MMX_Z_C1(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   BLEND_RGB_RGB_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1); \
}

#define BLEND_RGB_RGB_MMX_Z(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1) \
{ \
   BLEND_RGB_RGB_MMX(src_mmx, dst, mmx1, mmx2, mmxz, mmxc1); \
}

#endif
