#include "evas_common_private.h"
#ifdef BUILD_NEON
# ifdef BUILD_NEON_INTRINSICS
#  include <arm_neon.h>
# endif
#endif

static int cpu_feature_mask = 0;

#ifdef BUILD_ALTIVEC
# ifdef __POWERPC__
#  ifdef __VEC__
#   define NEED_FEATURE_TEST
#  endif
# endif
#endif

#ifdef __SPARC__
#   define NEED_FEATURE_TEST
#endif

#if defined(__ARM_ARCH__)
# ifdef BUILD_NEON
#  define NEED_FEATURE_TEST
# endif
#endif

#if defined(__aarch64__)
# ifdef BUILD_NEON
#  define NEED_FEATURE_TEST
# endif
#endif

#ifdef NEED_FEATURE_TEST
# ifdef HAVE_SIGLONGJMP
#  include <signal.h>
#  include <setjmp.h>
#  include <errno.h>

static sigjmp_buf detect_buf;
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
# endif

# ifdef BUILD_ALTIVEC
void
evas_common_cpu_altivec_test(void)
{
#  ifdef __POWERPC__
#   ifdef __VEC__
   vector unsigned int zero;

   zero = vec_splat_u32(0);
#   endif /* __VEC__ */
#  endif /* __POWERPC__ */
}
# endif /* BUILD_ALTIVEC */

void
evas_common_cpu_neon_test(void)
{
//#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 70)
# ifdef BUILD_NEON
#  ifdef BUILD_NEON_INTRINSICS
   volatile uint32x4_t temp = vdupq_n_u32(0x1);
   vaddq_u32(temp, temp);
#  else
   asm volatile (
		".fpu neon	     \n\t"
                 "vqadd.u8 d0, d1, d0\n"
                 : /* Out */
                 : /* In */
                 : /* Clobbered */
                 "d0", "d1"
                 );
#  endif
# endif
//#endif
}

void
evas_common_cpu_vis_test(void)
{
# ifdef __SPARC__
# endif /* __SPARC__ */
}
#endif /* NEED_FEATURE_TEST */


#if defined(BUILD_MMX) || defined(BUILD_SSE3)
# define NEED_CPU_CHECK
#endif

#ifdef NEED_FEATURE_TEST
# ifndef HAVE_SIGLONGJMP
#  undef NEED_CPU_CHECK
#  define NEED_CPU_CHECK
# endif
#endif

#ifdef NEED_CPU_CHECK
static Eina_Bool
_cpu_check(Eina_Cpu_Features f)
{
   Eina_Cpu_Features features;

   features = eina_cpu_features_get();
   return (features & f) == f;
}
#endif

#if defined(__aarch64__)
void
evas_common_cpu_sve_test(void)
{
   volatile int result = 123;
   asm("movz %w[res], #10" : [res] "=r" (result));
}
#endif

#ifdef NEED_FEATURE_TEST
int
evas_common_cpu_feature_test(void (*feature)(void))
{
# ifdef HAVE_SIGLONGJMP
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
# else
#  ifdef BUILD_MMX
   if (feature == evas_common_cpu_mmx_test)
     return _cpu_check(EINA_CPU_MMX);
   /* no mmx2 support in eina */
   if (feature == evas_common_cpu_sse_test)
     return _cpu_check(EINA_CPU_SSE);
#  endif
#  ifdef BUILD_SSE3
   if (feature == evas_common_cpu_sse3_test)
     return _cpu_check(EINA_CPU_SSE3);
#  endif
#  ifdef BUILD_ALTIVEC
   if (feature == evas_common_cpu_altivec_test)
     return _cpu_check(CPU_FEATURE_ALTIVEC);
#  endif
#  ifdef BUILD_NEON
   if (feature == evas_common_cpu_neon_test)
     return _cpu_check(EINA_CPU_NEON);
#  endif
#  if defined(__aarch64__)
   if (feature == evas_common_cpu_sve_test)
     return _cpu_check(EINA_CPU_SVE);
#  endif
   return 0;
# endif
}
#endif

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
     cpu_feature_mask |= _cpu_check(EINA_CPU_MMX) * CPU_FEATURE_MMX;
   if (getenv("EVAS_CPU_NO_MMX2"))
     cpu_feature_mask &= ~CPU_FEATURE_MMX2;
   else /* It seems "MMX2" is actually part of SSE (and 3DNow)? */
     cpu_feature_mask |= _cpu_check(EINA_CPU_SSE) * CPU_FEATURE_MMX2;
   if (getenv("EVAS_CPU_NO_SSE"))
     cpu_feature_mask &= ~CPU_FEATURE_SSE;
   else
     cpu_feature_mask |= _cpu_check(EINA_CPU_SSE) * CPU_FEATURE_SSE;
# ifdef BUILD_SSE3
   if (getenv("EVAS_CPU_NO_SSE3"))
     cpu_feature_mask &= ~CPU_FEATURE_SSE3;
   else
     cpu_feature_mask |= _cpu_check(EINA_CPU_SSE3) * CPU_FEATURE_SSE3;
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
#  if defined(HAVE_SYS_AUXV_H) && defined(HAVE_ASM_HWCAP_H) && defined(__arm__) && defined(__linux__)
        cpu_feature_mask |= CPU_FEATURE_NEON *
          !!(eina_cpu_features_get() & EINA_CPU_NEON);
#  else
        cpu_feature_mask |= CPU_FEATURE_NEON *
          evas_common_cpu_feature_test(evas_common_cpu_neon_test);
        evas_common_cpu_end_opt();
#  endif
     }
# endif
#endif

#if defined(__aarch64__)
   if (getenv("EVAS_CPU_NO_NEON"))
     cpu_feature_mask &= ~CPU_FEATURE_NEON;
   else
     cpu_feature_mask |= CPU_FEATURE_NEON;
#endif

#if defined(__aarch64__)
   if (getenv("EVAS_CPU_NO_SVE"))
     cpu_feature_mask &= ~CPU_FEATURE_SVE;
   else
     {
# if defined(HAVE_SYS_AUXV_H) && defined(HAVE_ASM_HWCAP_H) && defined(__arm__) && defined(__linux__)
        cpu_feature_mask |= CPU_FEATURE_SVE *
          !!(eina_cpu_features_get() & EINA_CPU_SVE);
# else
        cpu_feature_mask |= CPU_FEATURE_SVE *
          evas_common_cpu_feature_test(evas_common_cpu_sve_test);
        evas_common_cpu_end_opt();
# endif
     }
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
