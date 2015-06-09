#include "evas_common_private.h"

static RGBA_Gfx_Func     op_sub_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_sub_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_sub_init(void);
static void op_sub_shutdown(void);

static RGBA_Gfx_Func op_sub_pixel_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_pixel_color_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_mask_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_pixel_mask_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, Eina_Bool dst_alpha, int pixels);

static RGBA_Gfx_Pt_Func op_sub_pixel_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_color_pt_get(DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_pixel_color_pt_get(Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_mask_color_pt_get(DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_pixel_mask_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha);

static RGBA_Gfx_Compositor  _composite_sub = { "sub",
 op_sub_init, op_sub_shutdown,
 op_sub_pixel_span_get, op_sub_color_span_get,
 op_sub_pixel_color_span_get, op_sub_mask_color_span_get,
 op_sub_pixel_mask_span_get,
 op_sub_pixel_pt_get, op_sub_color_pt_get,
 op_sub_pixel_color_pt_get, op_sub_mask_color_pt_get,
 op_sub_pixel_mask_pt_get
 };

RGBA_Gfx_Compositor  *
evas_common_gfx_compositor_sub_get(void)
{
   return &(_composite_sub);
}

static RGBA_Gfx_Func     op_sub_rel_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_sub_rel_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_sub_rel_init(void);
static void op_sub_rel_shutdown(void);

static RGBA_Gfx_Func op_sub_rel_pixel_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_rel_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_rel_pixel_color_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_rel_mask_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels);
static RGBA_Gfx_Func op_sub_rel_pixel_mask_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels);

static RGBA_Gfx_Pt_Func op_sub_rel_pixel_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_rel_color_pt_get(DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_rel_pixel_color_pt_get(Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_rel_mask_color_pt_get(DATA32 col, Eina_Bool dst_alpha);
static RGBA_Gfx_Pt_Func op_sub_rel_pixel_mask_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha);

static RGBA_Gfx_Compositor  _composite_sub_rel = { "sub_rel",
 op_sub_rel_init, op_sub_rel_shutdown,
 op_sub_rel_pixel_span_get, op_sub_rel_color_span_get,
 op_sub_rel_pixel_color_span_get, op_sub_rel_mask_color_span_get,
 op_sub_rel_pixel_mask_span_get,
 op_sub_rel_pixel_pt_get, op_sub_rel_color_pt_get,
 op_sub_rel_pixel_color_pt_get, op_sub_rel_mask_color_pt_get,
 op_sub_rel_pixel_mask_pt_get
 };

RGBA_Gfx_Compositor  *
evas_common_gfx_compositor_sub_rel_get(void)
{
   return &(_composite_sub_rel);
}


# include "./evas_op_sub/op_sub_pixel_.c"
# include "./evas_op_sub/op_sub_color_.c"
# include "./evas_op_sub/op_sub_pixel_color_.c"
# include "./evas_op_sub/op_sub_pixel_mask_.c"
# include "./evas_op_sub/op_sub_mask_color_.c"
//# include "./evas_op_sub/op_sub_pixel_mask_color_.c"

# include "./evas_op_sub/op_sub_pixel_i386.c"
# include "./evas_op_sub/op_sub_color_i386.c"
# include "./evas_op_sub/op_sub_pixel_color_i386.c"
# include "./evas_op_sub/op_sub_pixel_mask_i386.c"
# include "./evas_op_sub/op_sub_mask_color_i386.c"
//# include "./evas_op_sub/op_sub_pixel_mask_color_i386.c"

static void
op_sub_init(void)
{
   memset(op_sub_span_funcs, 0, sizeof(op_sub_span_funcs));
   memset(op_sub_pt_funcs, 0, sizeof(op_sub_pt_funcs));
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
        init_sub_pixel_span_funcs_mmx();
        init_sub_pixel_color_span_funcs_mmx();
        init_sub_pixel_mask_span_funcs_mmx();
        init_sub_color_span_funcs_mmx();
        init_sub_mask_color_span_funcs_mmx();

        init_sub_pixel_pt_funcs_mmx();
        init_sub_pixel_color_pt_funcs_mmx();
        init_sub_pixel_mask_pt_funcs_mmx();
        init_sub_color_pt_funcs_mmx();
        init_sub_mask_color_pt_funcs_mmx();
     }
#endif
   init_sub_pixel_span_funcs_c();
   init_sub_pixel_color_span_funcs_c();
   init_sub_rel_pixel_mask_span_funcs_c();
   init_sub_color_span_funcs_c();
   init_sub_mask_color_span_funcs_c();

   init_sub_pixel_pt_funcs_c();
   init_sub_pixel_color_pt_funcs_c();
   init_sub_rel_pixel_mask_pt_funcs_c();
   init_sub_color_pt_funcs_c();
   init_sub_mask_color_pt_funcs_c();
}

static void
op_sub_shutdown(void)
{
}

static RGBA_Gfx_Func
sub_gfx_span_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_sub_span_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_sub_span_funcs[s][m][c][d][cpu];
   return func;
}

static RGBA_Gfx_Func
op_sub_pixel_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_pixel_color_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_mask_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_pixel_mask_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
sub_gfx_pt_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Pt_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_sub_pt_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_sub_pt_funcs[s][m][c][d][cpu];
   return func;
}

static RGBA_Gfx_Pt_Func
op_sub_pixel_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_color_pt_get(DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_pixel_color_pt_get(Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_mask_color_pt_get(DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_pixel_mask_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_gfx_pt_func_cpu(s, m, c, d);
}



static void
op_sub_rel_init(void)
{
   memset(op_sub_rel_span_funcs, 0, sizeof(op_sub_rel_span_funcs));
   memset(op_sub_rel_pt_funcs, 0, sizeof(op_sub_rel_pt_funcs));
#ifdef BUILD_MMX
   init_sub_rel_pixel_span_funcs_mmx();
   init_sub_rel_pixel_color_span_funcs_mmx();
   init_sub_rel_pixel_mask_span_funcs_mmx();
   init_sub_rel_color_span_funcs_mmx();
   init_sub_rel_mask_color_span_funcs_mmx();

   init_sub_rel_pixel_pt_funcs_mmx();
   init_sub_rel_pixel_color_pt_funcs_mmx();
   init_sub_rel_pixel_mask_pt_funcs_mmx();
   init_sub_rel_color_pt_funcs_mmx();
   init_sub_rel_mask_color_pt_funcs_mmx();
#endif
   init_sub_rel_pixel_span_funcs_c();
   init_sub_rel_pixel_color_span_funcs_c();
   init_sub_rel_pixel_mask_span_funcs_c();
   init_sub_rel_color_span_funcs_c();
   init_sub_rel_mask_color_span_funcs_c();

   init_sub_rel_pixel_pt_funcs_c();
   init_sub_rel_pixel_color_pt_funcs_c();
   init_sub_rel_pixel_mask_pt_funcs_c();
   init_sub_rel_color_pt_funcs_c();
   init_sub_rel_mask_color_pt_funcs_c();
}

static void
op_sub_rel_shutdown(void)
{
}

static RGBA_Gfx_Func
sub_rel_gfx_span_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_sub_rel_span_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_sub_rel_span_funcs[s][m][c][d][cpu];
   return func;
}

static RGBA_Gfx_Func
op_sub_rel_pixel_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_rel_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_rel_pixel_color_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_rel_mask_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_sub_rel_pixel_mask_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha EINA_UNUSED, Eina_Bool dst_alpha, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
sub_rel_gfx_pt_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Pt_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_sub_rel_pt_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_sub_rel_pt_funcs[s][m][c][d][cpu];
   return func;
}

static RGBA_Gfx_Pt_Func
op_sub_rel_pixel_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_rel_color_pt_get(DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_rel_pixel_color_pt_get(Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_rel_mask_color_pt_get(DATA32 col, Eina_Bool dst_alpha)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_sub_rel_pixel_mask_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src_alpha)
	s = SP;
   if (dst_alpha)
	d = DP;
   return sub_rel_gfx_pt_func_cpu(s, m, c, d);
}
