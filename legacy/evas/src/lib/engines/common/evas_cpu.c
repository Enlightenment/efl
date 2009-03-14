/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"

#if defined BUILD_MMX || defined BUILD_SSE
#include "evas_mmx.h"
#endif

#ifndef _WIN32
#include <signal.h>
#include <setjmp.h>

static sigjmp_buf detect_buf;
#endif

static int cpu_feature_mask = 0;

#ifndef _WIN32
static void evas_common_cpu_catch_ill(int sig);
static void evas_common_cpu_catch_segv(int sig);

static void
evas_common_cpu_catch_ill(int sig __UNUSED__)
{
   siglongjmp(detect_buf, 1);
}

static void
evas_common_cpu_catch_segv(int sig __UNUSED__)
{
   siglongjmp(detect_buf, 1);
}
#endif

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
   int blah[16];

   movntq_r2m(mm0, blah);
#endif
}

void
evas_common_cpu_altivec_test(void)
{
#ifdef __POWERPC__
#ifdef __VEC__
   vector unsigned int zero;

   zero = vec_splat_u32(0);
#endif /* __VEC__ */
#endif /* __POWERPC__ */
}

void
evas_common_cpu_neon_test(void)
{
#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 70)
#ifdef BUILD_NEON
   asm volatile (
                 "vqadd.u8 d0, d1, d0\n"
                 );
#endif
#endif
}

void
evas_common_cpu_vis_test(void)
{
#ifdef __SPARC__
#endif /* __SPARC__ */
}

int
evas_common_cpu_feature_test(void (*feature)(void))
{
#ifndef _WIN32
   int enabled = 1;
   struct sigaction act, oact, oact2;

   act.sa_handler = evas_common_cpu_catch_ill;
   act.sa_flags = SA_RESTART;
   sigemptyset(&act.sa_mask);
   sigaction(SIGILL, &act, &oact);

   act.sa_handler = evas_common_cpu_catch_segv;
   act.sa_flags = SA_RESTART;
   sigemptyset(&act.sa_mask);
   sigaction(SIGSEGV, &act, &oact2);

   if (sigsetjmp(detect_buf, 1))
     {
	sigaction(SIGILL, &oact, NULL);
	sigaction(SIGSEGV, &oact2, NULL);
	return 0;
     }

   feature();

   sigaction(SIGILL, &oact, NULL);
   sigaction(SIGSEGV, &oact2, NULL);
   return enabled;
#else
   return 0;
#endif
}

EAPI void
evas_common_cpu_init(void)
{
   static int called = 0;

   if (called) return;
   called = 1;
#ifdef BUILD_MMX
   cpu_feature_mask |= CPU_FEATURE_MMX *
     evas_common_cpu_feature_test(evas_common_cpu_mmx_test);
   evas_common_cpu_end_opt();
   if (getenv("EVAS_CPU_NO_MMX"))
     cpu_feature_mask &= ~CPU_FEATURE_MMX;
   cpu_feature_mask |= CPU_FEATURE_MMX2 *
     evas_common_cpu_feature_test(evas_common_cpu_mmx2_test);
   evas_common_cpu_end_opt();
   if (getenv("EVAS_CPU_NO_MMX2"))
     cpu_feature_mask &= ~CPU_FEATURE_MMX2;
#ifdef BUILD_SSE
   cpu_feature_mask |= CPU_FEATURE_SSE *
     evas_common_cpu_feature_test(evas_common_cpu_sse_test);
   evas_common_cpu_end_opt();
   if (getenv("EVAS_CPU_NO_SSE"))
     cpu_feature_mask &= ~CPU_FEATURE_SSE;
#endif /* BUILD_SSE */
#endif /* BUILD_MMX */
#ifdef __POWERPC__
#ifdef __VEC__
   cpu_feature_mask |= CPU_FEATURE_ALTIVEC *
     evas_common_cpu_feature_test(evas_common_cpu_altivec_test);
   evas_common_cpu_end_opt();
   if (getenv("EVAS_CPU_NO_ALTIVEC"))
     cpu_feature_mask &= ~CPU_FEATURE_ALTIVEC;
#endif /* __VEC__ */
#endif /* __POWERPC__ */
#ifdef __SPARC__
   cpu_feature_mask |= CPU_FEATURE_VIS *
     evas_common_cpu_feature_test(evas_common_cpu_vis_test);
   evas_common_cpu_end_opt();
   if (getenv("EVAS_CPU_NO_VIS"))
     cpu_feature_mask &= ~CPU_FEATURE_VIS;
#endif /* __SPARC__ */
#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 70)
#ifdef BUILD_NEON
   cpu_feature_mask |= CPU_FEATURE_NEON *
     evas_common_cpu_feature_test(evas_common_cpu_neon_test);
   evas_common_cpu_end_opt();
#endif
#endif
}

int
evas_common_cpu_has_feature(unsigned int feature)
{
   return (cpu_feature_mask & feature);
}

int
evas_common_cpu_have_cpuid(void)
{
   return 0;
/*
#ifdef BUILD_MMX
   unsigned int have_cpu_id;

   have_cpu_id = 0;
   have_cpuid(have_cpu_id);
   return have_cpu_id;
#else
   return 0;
#endif
 */
}

EAPI void
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
	if (cpu_feature_mask & CPU_FEATURE_MMX2) do_sse = 1;
	if (cpu_feature_mask & CPU_FEATURE_SSE) do_sse = 1;
     }
//   printf("%i %i %i\n", do_mmx, do_sse, do_sse2);
   *mmx = do_mmx;
   *sse = do_sse;
   *sse2 = do_sse2;
   done = 1;
}

#ifdef BUILD_MMX
EAPI void
evas_common_cpu_end_opt(void)
{
   if (cpu_feature_mask &
       (CPU_FEATURE_MMX | CPU_FEATURE_MMX2))
     {
	emms();
     }
}
#else
EAPI void
evas_common_cpu_end_opt(void)
{
}
#endif

EAPI int
evas_common_cpu_count(void)
{
#ifdef BUILD_PTHREAD
   cpu_set_t cpu;
   int i;
   static int cpus = 0;

   if (cpus != 0) return cpus;

   sched_getaffinity(getpid(), sizeof(cpu), &cpu);
   for (i = 0; i < TH_MAX; i++)
     {
	if (CPU_ISSET(i, &cpu)) cpus = i + 1;
	else break;
     }
   return cpus;
#else
   return 1;
#endif
}
