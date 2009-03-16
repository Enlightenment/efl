/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_blend_private.h"

static RGBA_Gfx_Func     op_copy_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_copy_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_copy_init(void);
static void op_copy_shutdown(void);

static RGBA_Gfx_Func op_copy_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);

static RGBA_Gfx_Pt_Func op_copy_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_mask_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);

static RGBA_Gfx_Compositor  _composite_copy = { "copy",
 op_copy_init, op_copy_shutdown,
 op_copy_pixel_span_get, op_copy_color_span_get,
 op_copy_pixel_color_span_get, op_copy_mask_color_span_get,
 op_copy_pixel_mask_span_get,
 op_copy_pixel_pt_get, op_copy_color_pt_get,
 op_copy_pixel_color_pt_get, op_copy_mask_color_pt_get,
 op_copy_pixel_mask_pt_get
 };

RGBA_Gfx_Compositor  *
evas_common_gfx_compositor_copy_get(void)
{
   return &(_composite_copy);
}


static RGBA_Gfx_Func     op_copy_rel_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_copy_rel_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_copy_rel_init(void);
static void op_copy_rel_shutdown(void);

static RGBA_Gfx_Func op_copy_rel_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_rel_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_rel_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_rel_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_copy_rel_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);

static RGBA_Gfx_Pt_Func op_copy_rel_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_rel_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_rel_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_rel_mask_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_copy_rel_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);

static RGBA_Gfx_Compositor  _composite_copy_rel = { "copy_rel",
 op_copy_rel_init, op_copy_rel_shutdown,
 op_copy_rel_pixel_span_get, op_copy_rel_color_span_get,
 op_copy_rel_pixel_color_span_get, op_copy_rel_mask_color_span_get,
 op_copy_rel_pixel_mask_span_get,
 op_copy_rel_pixel_pt_get, op_copy_color_pt_get,
 op_copy_rel_pixel_color_pt_get, op_copy_rel_mask_color_pt_get,
 op_copy_rel_pixel_mask_pt_get
 };

RGBA_Gfx_Compositor  *
evas_common_gfx_compositor_copy_rel_get(void)
{
   return &(_composite_copy_rel);
}


# include "./evas_op_copy/op_copy_pixel_.c"
# include "./evas_op_copy/op_copy_color_.c"
# include "./evas_op_copy/op_copy_pixel_color_.c"
# include "./evas_op_copy/op_copy_pixel_mask_.c"
# include "./evas_op_copy/op_copy_mask_color_.c"
//# include "./evas_op_copy/op_copy_pixel_mask_color_.c"

# include "./evas_op_copy/op_copy_pixel_i386.c"
# include "./evas_op_copy/op_copy_color_i386.c"
# include "./evas_op_copy/op_copy_pixel_color_i386.c"
# include "./evas_op_copy/op_copy_pixel_mask_i386.c"
# include "./evas_op_copy/op_copy_mask_color_i386.c"
//# include "./evas_op_copy/op_copy_pixel_mask_color_i386.c"

# include "./evas_op_copy/op_copy_pixel_neon.c"
# include "./evas_op_copy/op_copy_color_neon.c"
# include "./evas_op_copy/op_copy_pixel_color_neon.c"
# include "./evas_op_copy/op_copy_pixel_mask_neon.c"
# include "./evas_op_copy/op_copy_mask_color_neon.c"
//# include "./evas_op_copy/op_copy_pixel_mask_color_neon.c"


static void
op_copy_init(void)
{
   memset(op_copy_span_funcs, 0, sizeof(op_copy_span_funcs));
   memset(op_copy_pt_funcs, 0, sizeof(op_copy_pt_funcs));
#ifdef BUILD_MMX
   init_copy_pixel_span_funcs_mmx();
   init_copy_pixel_color_span_funcs_mmx();
   init_copy_pixel_mask_span_funcs_mmx();
   init_copy_color_span_funcs_mmx();
   init_copy_mask_color_span_funcs_mmx();

   init_copy_pixel_pt_funcs_mmx();
   init_copy_pixel_color_pt_funcs_mmx();
   init_copy_pixel_mask_pt_funcs_mmx();
   init_copy_color_pt_funcs_mmx();
   init_copy_mask_color_pt_funcs_mmx();
#endif
#ifdef BUILD_C
   init_copy_pixel_span_funcs_c();
   init_copy_pixel_color_span_funcs_c();
   init_copy_pixel_mask_span_funcs_c();
   init_copy_color_span_funcs_c();
   init_copy_mask_color_span_funcs_c();

   init_copy_pixel_pt_funcs_c();
   init_copy_pixel_color_pt_funcs_c();
   init_copy_pixel_mask_pt_funcs_c();
   init_copy_color_pt_funcs_c();
   init_copy_mask_color_pt_funcs_c();
#endif
#ifdef BUILD_NEON
   init_copy_pixel_span_funcs_neon();
   init_copy_pixel_color_span_funcs_neon();
   init_copy_pixel_mask_span_funcs_neon();
   init_copy_color_span_funcs_neon();
   init_copy_mask_color_span_funcs_neon();

//   init_copy_pixel_pt_funcs_neon();
//   init_copy_pixel_color_pt_funcs_neon();
//   init_copy_pixel_mask_pt_funcs_neon();
   init_copy_color_pt_funcs_neon();
//   init_copy_mask_color_pt_funcs_neon();
#endif
}

static void
op_copy_shutdown(void)
{
}

static RGBA_Gfx_Func
copy_gfx_span_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
    {
      cpu = CPU_MMX;
      func = op_copy_span_funcs[s][m][c][d][cpu];
      if (func) return func;
    }
#endif
#ifdef BUILD_NEON
   if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
    {
      cpu = CPU_NEON;
      func = op_copy_span_funcs[s][m][c][d][cpu];
      if (func) return func;
    }
#endif
#ifdef BUILD_C
   cpu = CPU_C;
   func = op_copy_span_funcs[s][m][c][d][cpu];
   if (func) return func;
#endif
   return func;
}

static RGBA_Gfx_Func
op_copy_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
     {
	dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_color_span_get(DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
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
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if ((col >> 24) < 255)
	c = SC;
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return copy_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if (src && src->cache_entry.flags.alpha)
	s = SP;
   return copy_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
copy_gfx_pt_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Pt_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
    {
      cpu = CPU_MMX;
      func = op_copy_pt_funcs[s][m][c][d][cpu];
      if (func) return func;
    }
#endif
#ifdef BUILD_C
   cpu = CPU_C;
   func = op_copy_pt_funcs[s][m][c][d][cpu];
   if (func) return func;
#endif
   return func;
}

static RGBA_Gfx_Pt_Func
op_copy_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_flags.alpha)
     {
	dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst)
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
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_mask_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if ((col >> 24) < 255)
	c = SC;
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return copy_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if (src_flags.alpha)
	s = SP;
   return copy_gfx_pt_func_cpu(s, m, c, d);
}


static void
op_copy_rel_init(void)
{
   memset(op_copy_rel_span_funcs, 0, sizeof(op_copy_rel_span_funcs));
   memset(op_copy_rel_pt_funcs, 0, sizeof(op_copy_rel_pt_funcs));
#ifdef BUILD_MMX
   init_copy_rel_pixel_span_funcs_mmx();
   init_copy_rel_pixel_color_span_funcs_mmx();
   init_copy_rel_pixel_mask_span_funcs_mmx();
   init_copy_rel_color_span_funcs_mmx();
   init_copy_rel_mask_color_span_funcs_mmx();

   init_copy_rel_pixel_pt_funcs_mmx();
   init_copy_rel_pixel_color_pt_funcs_mmx();
   init_copy_rel_pixel_mask_pt_funcs_mmx();
   init_copy_rel_color_pt_funcs_mmx();
   init_copy_rel_mask_color_pt_funcs_mmx();
#endif
#ifdef BUILD_C
   init_copy_rel_pixel_span_funcs_c();
   init_copy_rel_pixel_color_span_funcs_c();
   init_copy_rel_pixel_mask_span_funcs_c();
   init_copy_rel_color_span_funcs_c();
   init_copy_rel_mask_color_span_funcs_c();

   init_copy_rel_pixel_pt_funcs_c();
   init_copy_rel_pixel_color_pt_funcs_c();
   init_copy_rel_pixel_mask_pt_funcs_c();
   init_copy_rel_color_pt_funcs_c();
   init_copy_rel_mask_color_pt_funcs_c();
#endif
}

static void
op_copy_rel_shutdown(void)
{
}

static RGBA_Gfx_Func
copy_rel_gfx_span_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Func func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
    {
      cpu = CPU_MMX;
      func = op_copy_rel_span_funcs[s][m][c][d][cpu];
      if (func) return func;
    }
#endif
#ifdef BUILD_C
   cpu = CPU_C;
   func = op_copy_rel_span_funcs[s][m][c][d][cpu];
   if (func) return func;
#endif
   return func;
}

static RGBA_Gfx_Func
op_copy_rel_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_rel_color_span_get(DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_rel_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
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
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_rel_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if ((col >> 24) < 255)
	c = SC;
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return copy_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_copy_rel_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels __UNUSED__)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if (src && src->cache_entry.flags.alpha)
	s = SP;
   return copy_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
copy_rel_gfx_pt_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Pt_Func func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
    {
      cpu = CPU_MMX;
      func = op_copy_rel_pt_funcs[s][m][c][d][cpu];
      if (func) return func;
    }
#endif
#ifdef BUILD_C
   cpu = CPU_C;
   func = op_copy_rel_pt_funcs[s][m][c][d][cpu];
   if (func) return func;
#endif
   return func;
}

static RGBA_Gfx_Pt_Func
op_copy_rel_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_flags.alpha)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	s = SP;
     }
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_rel_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
     {
	if (dst)
	   dst->cache_entry.flags.alpha = 1;
	c = SC;
     }
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_rel_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst)
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
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return copy_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_rel_mask_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if ((col >> 24) < 255)
	c = SC;
   if (col == ((col >> 24) * 0x01010101))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   return copy_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_copy_rel_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP;

   if (dst)
	dst->cache_entry.flags.alpha = 1;
   if (src_flags.alpha)
	s = SP;
   return copy_rel_gfx_pt_func_cpu(s, m, c, d);
}
