#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

static jmp_buf detect_buf;
static int cpu_feature_mask = 0;

static void evas_common_cpu_catch_ill(int sig);

static void
evas_common_cpu_catch_ill(int sig)
{
   longjmp(detect_buf, 1);
}

#if ( \
	 defined __i386__ || \
	 defined __386__ || \
	 defined __X86__ || \
	 defined _M_IX86 || \
	 defined i386)
void
evas_common_cpu_mmx_test(void)
{
#ifdef BUILD_MMX
   pxor_r2r(mm4, mm4);
#endif   
}

void
evas_common_cpu_mmx2_test(void)
{
#ifdef BUILD_MMX
   char data[16];

   data[0] = 0;
   mmx_r2m(movntq, mm0, data);
   data[0] = 0;
#endif   
}

void
evas_common_cpu_sse_test(void)
{
#ifdef BUILD_SSE 
   int blah[2500];
   
   prefetch(blah + 1200);
#endif     
}
#endif /* __i386__ */

#ifdef __POWERPC__
#ifdef __VEC__
void
evas_common_cpu_altivec_test(void)
{
   vector unsigned int zero;
   
   zero = vec_splat_u32(0);
}
#endif /* __VEC__ */
#endif /* __POWERPC__ */

#ifdef __SPARC__
void
evas_common_cpu_vis_test(void)
{
}
#endif /* __SPARC__ */

int
evas_common_cpu_feature_test(void (*feature)(void))
{
   int enabled = 1;
   struct sigaction act, oact;

   act.sa_handler = evas_common_cpu_catch_ill;
   act.sa_flags = SA_RESTART;
   sigemptyset(&act.sa_mask);
   sigaction(SIGILL, &act, &oact);
   if (setjmp(detect_buf))
     {
	sigaction(SIGILL, &oact, NULL);
	return 0;
     }
   feature();
   sigaction(SIGILL, &oact, NULL);
   return enabled;
}

void
evas_common_cpu_init(void)
{
   static int called = 0;

   if (called) return;
   called = 1;
#if ( \
	 defined __i386__ || \
	 defined __386__ || \
	 defined __X86__ || \
	 defined _M_IX86 || \
	 defined i386)
#ifdef BUILD_MMX
   cpu_feature_mask |= CPU_FEATURE_MMX * 
     evas_common_cpu_feature_test(evas_common_cpu_mmx_test);
   evas_common_cpu_end_opt();
   cpu_feature_mask |= CPU_FEATURE_MMX2 * 
     evas_common_cpu_feature_test(evas_common_cpu_mmx2_test);
   evas_common_cpu_end_opt();
#ifdef BUILD_SSE
   cpu_feature_mask |= CPU_FEATURE_SSE * 
     evas_common_cpu_feature_test(evas_common_cpu_sse_test);
   evas_common_cpu_end_opt();
#endif /* BUILD_SSE */
#endif /* BUILD_MMX */
#endif /* __i386__ */
#ifdef __POWERPC__
#ifdef __VEC__
   cpu_feature_mask |= CPU_FEATURE_ALTIVEC *
     evas_common_cpu_feature_test(evas_common_cpu_altivec_test);
   evas_common_cpu_end_opt();
#endif /* __VEC__ */
#endif /* __POWERPC__ */
#ifdef __SPARC__
   cpu_feature_mask |= CPU_FEATURE_VIS *
     evas_common_cpu_feature_test(evas_common_cpu_vis_test);
   evas_common_cpu_end_opt();
#endif /* __SPARC__ */
}

int
evas_common_cpu_has_feature(unsigned int feature)
{
   return (cpu_feature_mask & feature);
}

int
evas_common_cpu_have_cpuid(void)
{
#ifdef BUILD_MMX
   unsigned int have_cpu_id;
   
   have_cpu_id = 0;
   have_cpuid(have_cpu_id);
   return have_cpu_id;
#else
   return 0;
#endif
}

void
evas_common_cpu_can_do(int *mmx, int *sse, int *sse2)
{
   static int do_mmx = 0, do_sse = 0, do_sse2 = 0, done = 0;

   if (!done)
     {
# ifdef HARD_CODED_P3
	cpu_feature_mask |= CPU_FEATURE_MMX;
	cpu_feature_mask |= CPU_FEATURE_SSE;
#endif
#ifdef HARD_CODED_P2
	cpu_feature_mask |= CPU_FEATURE_MMX;
#endif
	if (cpu_feature_mask & CPU_FEATURE_MMX) do_mmx = 1;
	if (cpu_feature_mask & CPU_FEATURE_MMX2) do_mmx = 2;
	if (cpu_feature_mask & CPU_FEATURE_SSE) do_sse = 1;
     }
//   printf("%i %i %i\n", do_mmx, do_sse, do_sse2);
   *mmx = do_mmx;
   *sse = do_sse;
   *sse2 = do_sse2;
   done = 1;
}

#ifdef BUILD_MMX
void
evas_common_cpu_end_opt(void)
{
   if (cpu_feature_mask & 
       (CPU_FEATURE_MMX | CPU_FEATURE_MMX2))
     {
	emms();
     }
}
#else
void
evas_common_cpu_end_opt(void)
{
}
#endif
