#include "evas_common_private.h"

static RGBA_Gfx_Func     op_mul_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_mul_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_mul_init(void);
static void op_mul_shutdown(void);

static RGBA_Gfx_Func op_mul_pixel_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_mul_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_mul_pixel_color_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_mul_mask_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_mul_pixel_mask_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels);

static RGBA_Gfx_Pt_Func op_mul_pixel_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_mul_color_pt_get(DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_mul_pixel_color_pt_get(Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_mul_mask_color_pt_get(DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_mul_pixel_mask_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha);

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
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
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
     }
#endif
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
   cpu = CPU_C;
   func = op_mul_span_funcs[s][m][c][d][cpu];
   return func;
}

static RGBA_Gfx_Func
op_mul_pixel_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_alpha)
     {
	s = SP;
     }
   if (dst_alpha)
	d = DP;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_pixel_color_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_alpha)
     {
	s = SP;
     }
   if ((col >> 24) < 255)
     {
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_mask_color_span_get(DATA32 col, Eina_Bool dst_alpha EINA_UNUSED, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return mul_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_mul_pixel_mask_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, Eina_Bool dst_alpha EINA_UNUSED, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (src_alpha)
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
   cpu = CPU_C;
   func = op_mul_pt_funcs[s][m][c][d][cpu];
   return func;
}

static RGBA_Gfx_Pt_Func
op_mul_pixel_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_alpha)
     {
	s = SP;
     }
   if (dst_alpha)
	d = DP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_color_pt_get(DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_pixel_color_pt_get(Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_alpha)
     {
	s = SP;
     }
   if ((col >> 24) < 255)
     {
	c = SC;
     }
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_mask_color_pt_get(DATA32 col, Eina_Bool dst_alpha EINA_UNUSED)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_mul_pixel_mask_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha EINA_UNUSED)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (src_alpha)
	s = SP;
   return mul_gfx_pt_func_cpu(s, m, c, d);
}
