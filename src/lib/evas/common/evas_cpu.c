#include "evas_common_private.h"
#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif
#ifdef BUILD_NEON
#ifdef BUILD_NEON_INTRINSICS
#include <arm_neon.h>
#endif
#endif
#if defined BUILD_SSE3
#include <immintrin.h>
#endif

#if defined (HAVE_STRUCT_SIGACTION) && defined (HAVE_SIGLONGJMP)
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

static sigjmp_buf detect_buf;
#endif

static int cpu_feature_mask = 0;

#if defined (HAVE_STRUCT_SIGACTION) && defined (HAVE_SIGLONGJMP)
static void evas_common_cpu_catch_ill(int sig);
static void evas_common_cpu_catch_segv(int sig);

static void
evas_common_cpu_catch_ill(int sig EINA_UNUSED)
{
   siglongjmp(detect_buf, 1);
}

static void
evas_common_cpu_catch_segv(int sig EINA_UNUSED)
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
#ifdef BUILD_MMX
   int blah[16];

   movntq_r2m(mm0, blah);
#endif
}

void evas_common_op_sse3_test(void);

void
evas_common_cpu_sse3_test(void)
{
#ifdef BUILD_SSE3
   evas_common_op_sse3_test();
#endif
}

#ifdef BUILD_ALTIVEC
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
#endif /* BUILD_ALTIVEC */

void
evas_common_cpu_neon_test(void)
{
//#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 70)
#ifdef BUILD_NEON
#ifdef BUILD_NEON_INTRINSICS
   volatile uint32x4_t temp = vdupq_n_u32(0x1);
#else
   asm volatile (
		".fpu neon	     \n\t"
                 "vqadd.u8 d0, d1, d0\n"
                 : /* Out */
                 : /* In */
                 : /* Clobbered */
                 "d0", "d1"
                 );
#endif
#endif
//#endif
}

void
evas_common_cpu_vis_test(void)
{
#ifdef __SPARC__
#endif /* __SPARC__ */
}

static Eina_Bool
_cpu_check(Eina_Cpu_Features f)
{
   Eina_Cpu_Features features;

   features = eina_cpu_features_get();
   return (features & f) == f;
}

int
evas_common_cpu_feature_test(void (*feature)(void))
{
#if defined (HAVE_STRUCT_SIGACTION) && defined (HAVE_SIGLONGJMP)
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
   if (feature == evas_common_cpu_mmx_test)
     return _cpu_check(EINA_CPU_MMX);
   /* no mmx2 support in eina */
   if (feature == evas_common_cpu_sse_test)
     return _cpu_check(EINA_CPU_SSE);
   if (feature == evas_common_cpu_sse3_test)
     return _cpu_check(EINA_CPU_SSE3);
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
   if (getenv("EVAS_CPU_NO_MMX"))
     cpu_feature_mask &= ~CPU_FEATURE_MMX;
   else
     {
        cpu_feature_mask |= CPU_FEATURE_MMX *
          evas_common_cpu_feature_test(evas_common_cpu_mmx_test);
        evas_common_cpu_end_opt();
     }
   if (getenv("EVAS_CPU_NO_MMX2"))
     cpu_feature_mask &= ~CPU_FEATURE_MMX2;
   else
     {
        cpu_feature_mask |= CPU_FEATURE_MMX2 *
          evas_common_cpu_feature_test(evas_common_cpu_mmx2_test);
        evas_common_cpu_end_opt();
     }
   if (getenv("EVAS_CPU_NO_SSE"))
     cpu_feature_mask &= ~CPU_FEATURE_SSE;
   else
     {
        cpu_feature_mask |= CPU_FEATURE_SSE *
          evas_common_cpu_feature_test(evas_common_cpu_sse_test);
        evas_common_cpu_end_opt();
     }
# ifdef BUILD_SSE3
   if (getenv("EVAS_CPU_NO_SSE3"))
     cpu_feature_mask &= ~CPU_FEATURE_SSE3; 
   else
     {
        cpu_feature_mask |= CPU_FEATURE_SSE3 *
          evas_common_cpu_feature_test(evas_common_cpu_sse3_test); 
        evas_common_cpu_end_opt();
     }
# endif /* BUILD_SSE3 */
#endif /* BUILD_MMX */
#ifdef BUILD_ALTIVEC
# ifdef __POWERPC__
#  ifdef __VEC__
   if (getenv("EVAS_CPU_NO_ALTIVEC"))
     cpu_feature_mask &= ~CPU_FEATURE_ALTIVEC;
   else
     {
        cpu_feature_mask |= CPU_FEATURE_ALTIVEC *
          evas_common_cpu_feature_test(evas_common_cpu_altivec_test);
        evas_common_cpu_end_opt();
     }
#  endif /* __VEC__ */
# endif /* __POWERPC__ */
#endif /* BUILD_ALTIVEC */
#ifdef __SPARC__
   if (getenv("EVAS_CPU_NO_VIS"))
     cpu_feature_mask &= ~CPU_FEATURE_VIS;
   else
     {
        cpu_feature_mask |= CPU_FEATURE_VIS *
          evas_common_cpu_feature_test(evas_common_cpu_vis_test);
        evas_common_cpu_end_opt();
     }
#endif /* __SPARC__ */
#if defined(__ARM_ARCH__)
# ifdef BUILD_NEON
   if (getenv("EVAS_CPU_NO_NEON"))
     cpu_feature_mask &= ~CPU_FEATURE_NEON;
   else
     {
        /* On linux eina_cpu sets this up with getauxval() */
#if defined(HAVE_SYS_AUXV_H) && defined(HAVE_ASM_HWCAP_H) && defined(__arm__) && defined(__linux__)
        cpu_feature_mask |= CPU_FEATURE_NEON *
          !!(eina_cpu_features_get() & EINA_CPU_NEON);
#else
        cpu_feature_mask |= CPU_FEATURE_NEON *
          evas_common_cpu_feature_test(evas_common_cpu_neon_test);
        evas_common_cpu_end_opt();
#endif
     }
# endif
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
        if (cpu_feature_mask & CPU_FEATURE_MMX) do_mmx = 1;
        if (cpu_feature_mask & CPU_FEATURE_MMX2) do_sse = 1;
        if (cpu_feature_mask & CPU_FEATURE_SSE) do_sse = 1;
        done = 1;
     }

   *mmx = do_mmx;
   *sse = do_sse;
   *sse2 = do_sse2;
}

#ifdef BUILD_MMX
EAPI void
evas_common_cpu_end_opt(void)
{
   if (cpu_feature_mask & (CPU_FEATURE_MMX | CPU_FEATURE_MMX2))
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
