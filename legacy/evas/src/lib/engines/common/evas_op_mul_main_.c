#include "evas_common.h"

static RGBA_Gfx_Func     op_mul_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_mul_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_mul_init(void);
static void op_mul_shutdown(void);

static RGBA_Gfx_Func op_mul_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_mul_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_mul_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_mul_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_mul_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);

static RGBA_Gfx_Pt_Func op_mul_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_mul_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_mul_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_mul_mask_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_mul_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);

static RGBA_Gfx_Compositor  _composite_mul = { "mul",
 op_mul_init, op_mul_shutdown,
 op_mul_pixel_span_get, op_mul_color_span_get,
 op_mul_pixel_color_span_get, op_mul_mask_color_span_get,
 op_mul_pixel_mask_span_get,
 op_mul_pixel_pt_get, op_mul_color_pt_get,
 op_mul_pixel_color_pt_get, op_mul_mask_color_pt_get,
 op_mul_pixel_mask_pt_get
 };

RGBA_Gfx_Compositor  *
evas_common_gfx_compositor_mul_get(void)
{
   return &(_composite_mul);
}


# include "./evas_op_mul/op_mul_pixel_.c"
# include "./evas_op_mul/op_mul_color_.c"
# include "./evas_op_mul/op_mul_pixel_color_.c"
# include "./evas_op_mul/op_mul_pixel_mask_.c"
# include "./evas_op_mul/op_mul_mask_color_.c"
//# include "./evas_op_mul/op_mul_pixel_mask_color_.c"

# include "./evas_op_mul/op_mul_pixel_i386.c"
# include "./evas_op_mul/op_mul_color_i386.c"
# include "./evas_op_mul/op_mul_pixel_color_i386.c"
# include "./evas_op_mul/op_mul_pixel_mask_i386.c"
# include "./evas_op_mul/op_mul_mask_color_i386.c"
// # include "./evas_op_mul/op_mul_pixel_mask_color_i386.c"

static void
op_mul_init(void)
{
   memset(op_mul_span_funcs, 0, sizeof(op_mul_span_funcs));
   memset(op_mul_pt_funcs, 0, sizeof(op_mul_pt_funcs));
#ifdef BUILD_MMX
   init_mul_pixel_span_funcs_mmx();
   init_mul_pixel_color_span_funcs_mmx();
   init_mul_pixel_mask_span_funcs_mmx();
   init_mul_color_span_funcs_mmx();
   init_mul_mask_color_span_funcs_mmx();

   init_mul_pixel_pt_funcs_mmx();
   init_mul_pixel_color_pt_funcs_mmx();
   init_mul_pixel_mask_pt_funcs_mmx();
   init_mul_color_pt_funcs_mmx();
   init_mul_mask_color_pt_funcs_mmx();
#endif
#ifdef BUILD_C
   init_mul_pixel_span_funcs_c();
   init_mul_pixel_color_span_funcs_c();
   init_mul_pixel_mask_span_funcs_c();
   init_mul_color_span_funcs_c();
   init_mul_mask_color_span_funcs_c();

   init_mul_pixel_pt_funcs_c();
   init_mul_pixel_color_pt_funcs_c();
   init_mul_pixel_mask_pt_funcs_c();
   init_mul_color_pt_funcs_c();
   init_mul_mask_color_pt_funcs_c();
#endif
}

static void
op_mul_shutdown(void)
{
}

static RGBA_Gfx_Func
mul_gfx_span_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Func func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_mul_span_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
#ifdef BUILD_C
   cpu = CPU_C;
   func = op_mul_span_funcs[s][m][c][d][cpu];
   if (func) return func;
#endif
   return func;
}

static RGBA_Gfx_Func
op_mul_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
     {
	dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_color_span_get(DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if (src && src->cache_entry.flags.alpha)
	s = SP;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
mul_gfx_pt_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Pt_Func func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
    {
      cpu = CPU_MMX;
      func = op_mul_pt_funcs[s][m][c][d][cpu];
      if (func) return func;
    }
#endif
#ifdef BUILD_C
   cpu = CPU_C;
   func = op_mul_pt_funcs[s][m][c][d][cpu];
   if (func) return func;
#endif
   return func;
}

static RGBA_Gfx_Pt_Func
op_mul_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_flags.alpha)
     {
	dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_flags.alpha)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_mask_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if (src_flags.alpha)
	s = SP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}
