/* Differential test: C reference vs NEON kernels in the evas span/point op tables.
 *
 * The op tables are indexed by CPU, so after init both [CPU_C] and [SIMD_TIER]
 * slots are live in the same process. We can therefore run the two variants
 * over identical buffers and compare, with no second build and no mocking.
 *
 * The copy tables are static, so this TU includes both op main_ files rather
 * than linking them; that keeps the production tree untouched.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Which vector tier this build compares against the C reference. The build
 * compiles this file once per tier; everything below is tier-agnostic. */
#ifndef SIMD_TIER
# error "SIMD_TIER must be defined by the build (e.g. -DSIMD_TIER=CPU_AVX2)"
#endif
#ifndef SIMD_NAME
# error "SIMD_NAME must be defined by the build (e.g. -DSIMD_NAME=\"avx2\")"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "evas_common_private.h"
#include "evas_blend_private.h"

/* Check if the current tier has a corresponding BUILD flag. Each tier can only
 * run if its implementation is compiled in. This must come AFTER the includes
 * above, which pull in evas_blend_ops.h and define CPU_NEON, CPU_AVX2, etc. */
#if (SIMD_TIER == CPU_NEON) && !defined(BUILD_NEON)
# define SIMD_TIER_UNAVAILABLE 1
#elif (SIMD_TIER == CPU_AVX2) && !defined(BUILD_AVX2)
# define SIMD_TIER_UNAVAILABLE 1
#endif

/* The op TUs only need these symbols from the rest of evas. Force NEON on so
 * both the C and the NEON init paths populate their slots regardless of any
 * EVAS_NEON_DISABLE setting in the environment. */
EAPI int
evas_common_cpu_has_feature(unsigned int feature EINA_UNUSED)
{
   return 1;
}

EAPI int
evas_common_cpu_has_neon_for(unsigned int part EINA_UNUSED)
{
   return 1;
}

#include "evas_op_blend_main_.c"
#include "evas_op_copy_main_.c"

/*----------------------------------------------------------------------------
 * buffers
 *
 * Every buffer is over-allocated and fully compared afterwards, so a kernel
 * that writes past the span it was given is caught as a difference just like a
 * kernel that computes the wrong pixel. That out-of-span write is the failure
 * mode that shows up as "weird artifacts" rather than as an obvious error.
 *--------------------------------------------------------------------------*/

#define MAX_LEN      1024
#define PAD_PIX      16                       /* pixels of slack each side */
#define BUF_PIX      (MAX_LEN + (2 * PAD_PIX))
#define BUF_BYTES    (BUF_PIX * (int)sizeof(DATA32))
#define GUARD_BYTE   0xA5

typedef struct
{
   void   *raw;
   DATA32 *pix;   /* raw + PAD_PIX pixels; span starts at pix[off] */
} Buf;

static void
buf_new(Buf *b)
{
   if (posix_memalign(&b->raw, 64, BUF_BYTES) != 0)
     {
        fprintf(stderr, "out of memory\n");
        exit(2);
     }
   b->pix = (DATA32 *)b->raw + PAD_PIX;
}

/*----------------------------------------------------------------------------
 * deterministic data generation
 *
 * Each op slot declares what it may assume about its inputs (SP_AN means the
 * source alphas really are 255, SC_AA means the colour really is a*0x01010101,
 * ...). Feeding data that violates those contracts would make C and NEON
 * disagree legitimately, so generators are driven by the slot's flags.
 *--------------------------------------------------------------------------*/

static unsigned int rng_state;

static unsigned int
rnd(void)
{
   /* xorshift32 - reproducible across platforms for a given seed */
   unsigned int x = rng_state;
   x ^= x << 13;
   x ^= x >> 17;
   x ^= x << 5;
   rng_state = x;
   return x;
}

/* Values that break naive fixed-point blends: the 255/256 boundary, the
 * rounding midpoint, and both saturation ends. */
static const DATA8 corners[] = { 0, 1, 2, 127, 128, 129, 253, 254, 255 };
#define NCORNERS ((int)(sizeof(corners) / sizeof(corners[0])))

typedef enum
{
   PAT_PREMUL,   /* realistic: premultiplied ARGB, rgb <= a */
   PAT_RAW,      /* rgb unconstrained: exercises saturating paths */
   PAT_CORNERS,  /* cycles the corner values above */
   PAT_ZERO,
   PAT_OPAQUE,   /* 0xffffffff */
   PAT_RAMP,     /* alpha sweeps 0..255 across the span */
   PAT_LAST
} Pattern;

static const char *pat_names[] =
{ "premul", "raw", "corners", "zero", "opaque", "ramp" };

static DATA32
gen_pixel(Pattern pat, int i, int alpha_flag)
{
   DATA8 a, r, g, b;

   /* Alpha first, honouring what the slot is allowed to assume, so the
    * premultiplied clamp below sees the alpha the kernel will actually act on */
   switch (pat)
     {
      case PAT_ZERO:    a = 0; break;
      case PAT_OPAQUE:  a = 255; break;
      case PAT_CORNERS: a = corners[i % NCORNERS]; break;
      case PAT_RAMP:
        a = (DATA8)((i * 255) / (MAX_LEN > 1 ? MAX_LEN - 1 : 1));
        break;
      default:          a = (DATA8)rnd(); break;
     }

   if (alpha_flag == SP_AN) a = 255;
   else if ((alpha_flag == SP_AS) && (rnd() & 3))
     a = (rnd() & 1) ? 255 : 0;   /* sparse: mostly wholly opaque or clear */

   switch (pat)
     {
      case PAT_ZERO:    r = g = b = 0; break;
      case PAT_OPAQUE:  r = g = b = 255; break;
      case PAT_CORNERS:
        r = corners[(i + 3) % NCORNERS];
        g = corners[(i + 5) % NCORNERS];
        b = corners[(i + 7) % NCORNERS];
        break;
      case PAT_RAMP:
        r = (DATA8)(a / 2); g = (DATA8)(a / 3); b = a;
        break;
      default:
        r = (DATA8)rnd(); g = (DATA8)rnd(); b = (DATA8)rnd();
        break;
     }

   /* Evas pixels are premultiplied, so rgb <= a always holds in real data and
    * kernels are entitled to rely on it. PAT_RAW deliberately breaks the
    * invariant; its differences are reported separately and are not failures. */
   if (pat != PAT_RAW)
     {
        if (r > a) r = a;
        if (g > a) g = a;
        if (b > a) b = a;
     }

   return ((DATA32)a << 24) | ((DATA32)r << 16) | ((DATA32)g << 8) | b;
}

static DATA8
gen_mask(Pattern pat, int i, int m_flag)
{
   switch (m_flag)
     {
      case SM_AT:  /* trivial: only 0 or 255 */
        return (rnd() & 1) ? 255 : 0;
      case SM_AS:  /* sparse: mostly trivial, occasionally not */
        if ((rnd() & 3) == 0) return (DATA8)rnd();
        return (rnd() & 1) ? 255 : 0;
      case SM:
      default:
        if (pat == PAT_CORNERS) return corners[(i + 2) % NCORNERS];
        if (pat == PAT_ZERO)    return 0;
        if (pat == PAT_OPAQUE)  return 255;
        if (pat == PAT_RAMP)    return (DATA8)((i * 255) / (MAX_LEN > 1 ? MAX_LEN - 1 : 1));
        return (DATA8)rnd();
     }
}

static DATA32
gen_color(int c_flag, int idx)
{
   DATA8 a;

   switch (c_flag)
     {
      case SC_N:   /* colour is 0xffffffff */
        return 0xffffffff;
      case SC_AA:  /* colour is a*0x01010101 */
        a = corners[idx % NCORNERS];
        return (DATA32)a * 0x01010101U;
      case SC_AN:  /* colour is 0xffrrggbb */
        return 0xff000000U | (rnd() & 0x00ffffffU);
      case SC:
      default:
        {
           DATA32 col;
           a = corners[idx % NCORNERS];
           col = ((DATA32)a << 24)
                 | ((DATA32)(a ? rnd() % (a + 1) : 0) << 16)
                 | ((DATA32)(a ? rnd() % (a + 1) : 0) << 8)
                 | ((DATA32)(a ? rnd() % (a + 1) : 0));
           return col;
        }
     }
}

/*--------------------------------------------------------------------------*/

static const char *sp_names[] = { "SP_N", "SP", "SP_AN", "SP_AS" };
static const char *sm_names[] = { "SM_N", "SM", "SM_AT", "SM_AS" };
static const char *sc_names[] = { "SC_N", "SC", "SC_AN", "SC_AA" };
static const char *dp_names[] = { "DP", "DP_AN" };

/* Lengths around the 8-pixel vector width, where tail handling lives, plus
 * spans as wide as a real scanline - some kernels block larger than 8 and only
 * misbehave well past the short lengths. */
static const int lens[] =
{
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
   23, 24, 25, 31, 32, 33, 39, 40, 41, 47, 48, 49, 63, 64, 65, 71, 72,
   127, 128, 129, 255, 256, 257, 512, 720, 1024
};
#define NLENS ((int)(sizeof(lens) / sizeof(lens[0])))

/* (src, dst, mask) pixel/byte offsets - catches assumed alignment */
static const int offs[][3] =
{
   {0,0,0}, {1,0,0}, {0,1,0}, {1,1,0}, {2,3,0}, {3,2,1}, {4,4,2},
   {5,1,3}, {7,7,7}, {1,7,5}, {6,2,4}, {2,6,6}
};
#define NOFFS ((int)(sizeof(offs) / sizeof(offs[0])))

typedef struct
{
   unsigned long long cases;
   unsigned long long span_diff;      /* differing pixels inside the span */
   unsigned long long raw_diff;       /* ditto, but from invalid PAT_RAW input */
   unsigned long long oob_simd;       /* NEON wrote outside its span */
   unsigned long long oob_c;          /* C wrote outside its span */
   unsigned long long src_clobber;    /* an implementation modified src/mask */
   int                max_delta;      /* worst per-channel difference */
   int                raw_max_delta;
} Stats;

/* Each failure kind gets its own print budget, so a flood of one-off rounding
 * differences cannot hide a single buffer overrun reported later. */
typedef enum
{
   CAT_DIFF, CAT_RAW, CAT_OOB, CAT_CLOBBER, CAT_PT, CAT_LAST
} Category;

static int verbose = 0;
static int max_report = 12;
static int reported[CAT_LAST];

static int
chan_delta(DATA32 a, DATA32 b)
{
   int worst = 0, i;

   for (i = 0; i < 4; i++)
     {
        int va = (int)((a >> (i * 8)) & 0xff);
        int vb = (int)((b >> (i * 8)) & 0xff);
        int d = va > vb ? va - vb : vb - va;
        if (d > worst) worst = d;
     }
   return worst;
}

static void
report(Category cat, const char *table, int s, int m, int c, int d, int len,
       const int *off, Pattern pat, const char *what,
       int idx, DATA32 got, DATA32 want, DATA32 sv, DATA8 mv, DATA32 col)
{
   if (reported[cat] >= max_report) return;
   reported[cat]++;

   printf("  %s[%s][%s][%s][%s] len=%d off=(s%d,d%d,m%d) pat=%s: %s\n",
          table, sp_names[s], sm_names[m], sc_names[c], dp_names[d],
          len, off[0], off[1], off[2], pat_names[pat], what);
   printf("      pixel %d: " SIMD_NAME "=%08x c=%08x  (src=%08x mask=%02x col=%08x)\n",
          idx, got, want, sv, mv, col);
   if (reported[cat] == max_report)
     printf("  ... further reports of this kind suppressed (--max-report to raise)\n");
}

/* Run one span function pair over one configuration. */
static void
run_span_case(const char *table, RGBA_Gfx_Func fc, RGBA_Gfx_Func fn,
              int s, int m, int c, int d, int len, const int *off,
              Pattern pat, Stats *st)
{
   static Buf src, msk, src_o, msk_o, dst_c, dst_n, dst_o;
   static int inited = 0;
   int soff = off[0], doff = off[1], moff = off[2];
   DATA32 col;
   int i;

   if (!inited)
     {
        buf_new(&src); buf_new(&msk);
        buf_new(&src_o); buf_new(&msk_o);
        buf_new(&dst_c); buf_new(&dst_n); buf_new(&dst_o);
        inited = 1;
     }

   /* fill every byte, guards included, so stray writes are visible */
   memset(src.raw, GUARD_BYTE, BUF_BYTES);
   memset(msk.raw, GUARD_BYTE, BUF_BYTES);
   memset(dst_o.raw, GUARD_BYTE, BUF_BYTES);

   for (i = 0; i < len; i++)
     {
        src.pix[soff + i] = gen_pixel(pat, i, s);
        ((DATA8 *)msk.pix)[moff + i] = gen_mask(pat, i, m);
        dst_o.pix[doff + i] = gen_pixel(pat, i + 11, (d == DP_AN) ? SP_AN : SP);
     }
   col = gen_color(c, len);

   memcpy(dst_c.raw, dst_o.raw, BUF_BYTES);
   memcpy(dst_n.raw, dst_o.raw, BUF_BYTES);
   memcpy(src_o.raw, src.raw, BUF_BYTES);
   memcpy(msk_o.raw, msk.raw, BUF_BYTES);

   fc(src.pix + soff, (DATA8 *)msk.pix + moff, col, dst_c.pix + doff, len);

   /* src and mask are inputs: neither variant may touch them, and restoring
    * before the second call keeps a C-side clobber from poisoning NEON's run */
   if (memcmp(src.raw, src_o.raw, BUF_BYTES) ||
       memcmp(msk.raw, msk_o.raw, BUF_BYTES))
     {
        report(CAT_CLOBBER, table, s, m, c, d, len, off, pat,
               "C modified its source or mask", 0, 0, 0, 0, 0, col);
        st->src_clobber++;
        memcpy(src.raw, src_o.raw, BUF_BYTES);
        memcpy(msk.raw, msk_o.raw, BUF_BYTES);
     }

   fn(src.pix + soff, (DATA8 *)msk.pix + moff, col, dst_n.pix + doff, len);

   if (memcmp(src.raw, src_o.raw, BUF_BYTES) ||
       memcmp(msk.raw, msk_o.raw, BUF_BYTES))
     {
        report(CAT_CLOBBER, table, s, m, c, d, len, off, pat,
               SIMD_NAME " modified its source or mask", 0, 0, 0, 0, 0, col);
        st->src_clobber++;
        memcpy(src.raw, src_o.raw, BUF_BYTES);
        memcpy(msk.raw, msk_o.raw, BUF_BYTES);
     }

   st->cases++;

   /* 1. out-of-span writes, either implementation */
   for (i = 0; i < BUF_PIX; i++)
     {
        int in_span = (i >= PAD_PIX + doff) && (i < PAD_PIX + doff + len);
        DATA32 orig = ((DATA32 *)dst_o.raw)[i];

        if (in_span) continue;
        if (((DATA32 *)dst_n.raw)[i] != orig)
          {
             report(CAT_OOB, table, s, m, c, d, len, off, pat,
                    SIMD_NAME " wrote outside its span", i - PAD_PIX - doff,
                    ((DATA32 *)dst_n.raw)[i], orig, 0, 0, col);
             st->oob_simd++;
          }
        if (((DATA32 *)dst_c.raw)[i] != orig)
          {
             report(CAT_OOB, table, s, m, c, d, len, off, pat,
                    "C wrote outside its span", i - PAD_PIX - doff,
                    ((DATA32 *)dst_c.raw)[i], orig, 0, 0, col);
             st->oob_c++;
          }
     }

   /* 2. in-span comparison */
   for (i = 0; i < len; i++)
     {
        DATA32 vn = dst_n.pix[doff + i];
        DATA32 vc = dst_c.pix[doff + i];
        int delta;

        if (vn == vc) continue;

        delta = chan_delta(vn, vc);

        if (pat == PAT_RAW)
          {
             /* input violates the premultiplied invariant: informational only */
             st->raw_diff++;
             if (delta > st->raw_max_delta) st->raw_max_delta = delta;
             if (verbose)
               report(CAT_RAW, table, s, m, c, d, len, off, pat,
                      "difference on non-premultiplied input", i, vn, vc,
                      src.pix[soff + i], ((DATA8 *)msk.pix)[moff + i], col);
             continue;
          }

        if (delta > st->max_delta) st->max_delta = delta;
        st->span_diff++;

        if (verbose || delta > 1)
          report(CAT_DIFF, table, s, m, c, d, len, off, pat,
                 delta > 1 ? "mismatch beyond rounding" : "rounding difference",
                 i, vn, vc, src.pix[soff + i],
                 ((DATA8 *)msk.pix)[moff + i], col);
     }
}

static void
walk_span_table(const char *table,
                RGBA_Gfx_Func (*t)[SM_LAST][SC_LAST][DP_LAST][CPU_LAST],
                int iterations, Stats *total)
{
   int s, m, c, d, li, oi, p, it;
   int pairs = 0, simd_only = 0;

   for (s = 0; s < SP_LAST; s++)
     for (m = 0; m < SM_LAST; m++)
       for (c = 0; c < SC_LAST; c++)
         for (d = 0; d < DP_LAST; d++)
           {
              RGBA_Gfx_Func fc = t[s][m][c][d][CPU_C];
              RGBA_Gfx_Func fn = t[s][m][c][d][SIMD_TIER];
              Stats st;

              if (!fn) continue;
              if (!fc)
                {
                   printf("  %s[%s][%s][%s][%s]: " SIMD_NAME " slot with no C reference\n",
                          table, sp_names[s], sm_names[m], sc_names[c], dp_names[d]);
                   simd_only++;
                   continue;
                }
              pairs++;

              memset(&st, 0, sizeof(st));
              for (it = 0; it < iterations; it++)
                for (li = 0; li < NLENS; li++)
                  for (oi = 0; oi < NOFFS; oi++)
                    for (p = 0; p < PAT_LAST; p++)
                      run_span_case(table, fc, fn, s, m, c, d,
                                    lens[li], offs[oi], (Pattern)p, &st);

              total->cases += st.cases;
              total->span_diff += st.span_diff;
              total->raw_diff += st.raw_diff;
              if (st.raw_max_delta > total->raw_max_delta)
                total->raw_max_delta = st.raw_max_delta;
              total->oob_simd += st.oob_simd;
              total->oob_c += st.oob_c;
              total->src_clobber += st.src_clobber;
              if (st.max_delta > total->max_delta) total->max_delta = st.max_delta;

              if (st.span_diff || st.oob_simd || st.oob_c || st.src_clobber)
                printf("  %-10s[%-5s][%-5s][%-5s][%-5s] diff=%llu oob_simd=%llu oob_c=%llu clobber=%llu maxdelta=%d\n",
                       table, sp_names[s], sm_names[m], sc_names[c], dp_names[d],
                       st.span_diff, st.oob_simd, st.oob_c, st.src_clobber,
                       st.max_delta);
              /* Worth naming even though the input is out of contract: map and
               * scale interpolation really do emit transparent-but-coloured
               * pixels, so a slot that diverges here can still show artifacts. */
              else if (st.raw_diff)
                printf("  %-10s[%-5s][%-5s][%-5s][%-5s] non-premultiplied-input diff=%llu maxdelta=%d\n",
                       table, sp_names[s], sm_names[m], sc_names[c], dp_names[d],
                       st.raw_diff, st.raw_max_delta);
           }

   printf("%s: %d C/" SIMD_NAME " pairs compared", table, pairs);
   if (simd_only) printf(", %d " SIMD_NAME "-only slots", simd_only);
   printf("\n");
}

/*--------------------------------------------------------------------------*/

static void
walk_pt_table(const char *table,
              RGBA_Gfx_Pt_Func (*t)[SM_LAST][SC_LAST][DP_LAST][CPU_LAST],
              Stats *total)
{
   int s, m, c, d, i, j, k;
   int pairs = 0;

   for (s = 0; s < SP_LAST; s++)
     for (m = 0; m < SM_LAST; m++)
       for (c = 0; c < SC_LAST; c++)
         for (d = 0; d < DP_LAST; d++)
           {
              RGBA_Gfx_Pt_Func fc = t[s][m][c][d][CPU_C];
              RGBA_Gfx_Pt_Func fn = t[s][m][c][d][SIMD_TIER];
              unsigned long long diff = 0;
              int maxd = 0;

              if (!fn || !fc) continue;
              pairs++;

              for (i = 0; i < NCORNERS; i++)
                for (j = 0; j < NCORNERS; j++)
                  for (k = 0; k < NCORNERS; k++)
                    {
                       DATA8 sa = (s == SP_AN) ? 255 : corners[i];
                       DATA8 da = (d == DP_AN) ? 255 : corners[k];
                       /* keep both operands premultiplied (rgb <= a) */
                       DATA32 sv = ((DATA32)sa << 24)
                                   | ((DATA32)MIN(corners[j], sa) << 16)
                                   | ((DATA32)MIN(corners[k], sa) << 8)
                                   | MIN(corners[i], sa);
                       DATA8 mv = corners[j];
                       DATA32 col = gen_color(c, k);
                       DATA32 dorig = ((DATA32)da << 24)
                                      | ((DATA32)MIN(corners[i], da) << 16)
                                      | ((DATA32)MIN(corners[j], da) << 8)
                                      | MIN(corners[k], da);
                       DATA32 dc, dn;
                       int delta;

                       dc = dn = dorig;
                       fc(sv, mv, col, &dc);
                       fn(sv, mv, col, &dn);

                       if (dc == dn) continue;
                       delta = chan_delta(dn, dc);
                       if (delta > maxd) maxd = delta;
                       diff++;
                       total->span_diff++;
                       if (delta > total->max_delta) total->max_delta = delta;
                       if (verbose || delta > 1)
                         {
                            static const int zoff[3] = {0,0,0};
                            report(CAT_PT, table, s, m, c, d, 1, zoff, PAT_CORNERS,
                                   delta > 1 ? "pt mismatch beyond rounding"
                                             : "pt rounding difference",
                                   0, dn, dc, sv, mv, col);
                         }
                    }

              if (diff)
                printf("  %-13s[%-5s][%-5s][%-5s][%-5s] diff=%llu maxdelta=%d\n",
                       table, sp_names[s], sm_names[m], sc_names[c], dp_names[d],
                       diff, maxd);
           }

   printf("%s: %d C/" SIMD_NAME " pairs compared\n", table, pairs);
}

/*----------------------------------------------------------------------------
 * microbenchmark
 *
 * Kernel level timing, isolated from the rest of evas. Reports the best of
 * several trials rather than the mean: on a loaded machine the minimum is the
 * measurement least polluted by other work, and these kernels are entirely
 * deterministic so the fastest observed run is the honest one.
 *--------------------------------------------------------------------------*/

static double
now_sec(void)
{
   struct timespec ts;

   clock_gettime(CLOCK_MONOTONIC, &ts);
   return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static double
time_one(RGBA_Gfx_Func f, DATA32 *s, DATA8 *m, DATA32 col, DATA32 *d, int len,
         int iters)
{
   double t0 = now_sec();
   int i;

   for (i = 0; i < iters; i++) f(s, m, col, d, len);
   return now_sec() - t0;
}

/* Measure the two implementations alternately inside one trial loop, after a
 * warm up pass. Timing them in separate batches makes the result depend on
 * whatever else the machine happened to be doing during each batch, which on a
 * loaded box is enough to invert the comparison. */
static void
bench_pair(RGBA_Gfx_Func fc, RGBA_Gfx_Func fn, DATA32 *s, DATA8 *m, DATA32 col,
           DATA32 *d, int len, int iters, int trials,
           double *out_c, double *out_n)
{
   double bc = 1e30, bn = 1e30;
   int t;

   time_one(fc, s, m, col, d, len, iters);
   time_one(fn, s, m, col, d, len, iters);

   for (t = 0; t < trials; t++)
     {
        double dt;

        dt = time_one(fc, s, m, col, d, len, iters);
        if (dt < bc) bc = dt;
        dt = time_one(fn, s, m, col, d, len, iters);
        if (dt < bn) bn = dt;
     }
   *out_c = bc / (double)iters;
   *out_n = bn / (double)iters;
}

static void
bench_span_table(const char *table,
                 RGBA_Gfx_Func (*t)[SM_LAST][SC_LAST][DP_LAST][CPU_LAST],
                 int len, int iters, int trials)
{
   static Buf src, msk, dst;
   static int inited = 0;
   int s, m, c, d, i;

   if (!inited)
     {
        buf_new(&src); buf_new(&msk); buf_new(&dst);
        inited = 1;
     }

   for (s = 0; s < SP_LAST; s++)
     for (m = 0; m < SM_LAST; m++)
       for (c = 0; c < SC_LAST; c++)
         for (d = 0; d < DP_LAST; d++)
           {
              RGBA_Gfx_Func fc = t[s][m][c][d][CPU_C];
              RGBA_Gfx_Func fn = t[s][m][c][d][SIMD_TIER];
              double tc, tn;
              DATA32 col;

              if (!fc || !fn) continue;

              rng_state = 12345;   /* identical data for every slot */
              for (i = 0; i < len; i++)
                {
                   src.pix[i] = gen_pixel(PAT_PREMUL, i, s);
                   ((DATA8 *)msk.pix)[i] = gen_mask(PAT_PREMUL, i, m);
                   dst.pix[i] = gen_pixel(PAT_PREMUL, i + 11,
                                          (d == DP_AN) ? SP_AN : SP);
                }
              col = gen_color(c, len);

              bench_pair(fc, fn, src.pix, (DATA8 *)msk.pix, col, dst.pix, len,
                         iters, trials, &tc, &tn);

              printf("  %-10s[%-5s][%-5s][%-5s][%-5s] C %7.1f Mpx/s  " SIMD_NAME " %7.1f Mpx/s  %5.2fx\n",
                     table, sp_names[s], sm_names[m], sc_names[c], dp_names[d],
                     len / tc / 1e6, len / tn / 1e6, tc / tn);
           }
}

/*--------------------------------------------------------------------------*/

int
main(int argc, char **argv)
{
   Stats total;
   int iterations = 1;
   unsigned int seed = 1;
   int bench = 0, bench_len = 1024, bench_iters = 200, bench_trials = 9;
   int i;

   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) verbose = 1;
        else if (!strcmp(argv[i], "--bench")) bench = 1;
        else if (!strncmp(argv[i], "--seed=", 7)) seed = (unsigned)atoi(argv[i] + 7);
        else if (!strncmp(argv[i], "--iterations=", 13)) iterations = atoi(argv[i] + 13);
        else if (!strncmp(argv[i], "--max-report=", 13)) max_report = atoi(argv[i] + 13);
        else if (!strncmp(argv[i], "--bench-len=", 12)) bench_len = atoi(argv[i] + 12);
        else if (!strncmp(argv[i], "--bench-trials=", 15)) bench_trials = atoi(argv[i] + 15);
        else
          {
             printf("usage: %s [-v] [--seed=N] [--iterations=N] [--max-report=N]\n"
                    "       %s --bench [--bench-len=N] [--bench-trials=N]\n",
                    argv[0], argv[0]);
             return 2;
          }
     }
   if (bench_len < 1 || bench_len > MAX_LEN) bench_len = MAX_LEN;
   if (!seed) seed = 1;
   rng_state = seed;

#ifdef SIMD_TIER_UNAVAILABLE
   printf("built without " SIMD_NAME " support - nothing to compare\n");
   return 77;   /* meson/automake "skipped" */
#else
   printf("evas op table C vs " SIMD_NAME " differential test (seed=%u iterations=%d)\n\n",
          seed, iterations);

   memset(&total, 0, sizeof(total));

   evas_common_gfx_compositor_blend_get()->init();
   evas_common_gfx_compositor_blend_rel_get()->init();
   evas_common_gfx_compositor_copy_get()->init();
   evas_common_gfx_compositor_copy_rel_get()->init();

   if (bench)
     {
        printf("span kernel throughput, %d px spans, best of %d\n\n",
               bench_len, bench_trials);
        bench_span_table("blend", op_blend_span_funcs, bench_len, bench_iters, bench_trials);
        bench_span_table("blend_rel", op_blend_rel_span_funcs, bench_len, bench_iters, bench_trials);
        bench_span_table("copy", op_copy_span_funcs, bench_len, bench_iters, bench_trials);
        bench_span_table("copy_rel", op_copy_rel_span_funcs, bench_len, bench_iters, bench_trials);
        return 0;
     }

   walk_span_table("blend", op_blend_span_funcs, iterations, &total);
   walk_span_table("blend_rel", op_blend_rel_span_funcs, iterations, &total);
   walk_span_table("copy", op_copy_span_funcs, iterations, &total);
   walk_span_table("copy_rel", op_copy_rel_span_funcs, iterations, &total);

   walk_pt_table("blend_pt", op_blend_pt_funcs, &total);
   walk_pt_table("blend_rel_pt", op_blend_rel_pt_funcs, &total);
   walk_pt_table("copy_pt", op_copy_pt_funcs, &total);
   walk_pt_table("copy_rel_pt", op_copy_rel_pt_funcs, &total);

   printf("\n--- summary ---\n");
   printf("cases run          : %llu\n", total.cases);
   printf("differing pixels   : %llu\n", total.span_diff);
   printf("worst channel delta: %d\n", total.max_delta);
   printf("  (non-premultiplied input, informational: %llu diffs, worst delta %d)\n",
          total.raw_diff, total.raw_max_delta);
   printf(SIMD_NAME " out-of-span   : %llu\n", total.oob_simd);
   printf("C out-of-span      : %llu\n", total.oob_c);
   printf("input clobbered    : %llu\n", total.src_clobber);

   if (total.oob_simd || total.oob_c || total.src_clobber)
     {
        printf("RESULT: FAIL (buffer overrun or input clobbered)\n");
        return 1;
     }
   if (total.max_delta > 1)
     {
        printf("RESULT: FAIL (differences beyond rounding)\n");
        return 1;
     }
   if (total.span_diff)
     {
        printf("RESULT: PASS with rounding differences (max 1 per channel)\n");
        return 0;
     }
   printf("RESULT: PASS (bit exact)\n");
   return 0;
#endif
}
