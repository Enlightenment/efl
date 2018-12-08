#include "evas_common_private.h"

static int cpu_feature_mask = 0;

static Eina_Bool
_cpu_check(Eina_Cpu_Features f)
{
   Eina_Cpu_Features features = eina_cpu_features_get();
   return (features & f) == f;
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
   if (getenv("EVAS_CPU_NO_ALTIVEC"))
     cpu_feature_mask &= ~CPU_FEATURE_ALTIVEC;
   else
     cpu_feature_mask |= _cpu_check(CPU_FEATURE_ALTIVEC) * CPU_FEATURE_ALTIVEC;
#endif /* BUILD_ALTIVEC */

#if defined(__ARM_ARCH__)
# ifdef BUILD_NEON
   if (getenv("EVAS_CPU_NO_NEON"))
     cpu_feature_mask &= ~CPU_FEATURE_NEON;
   else
     cpu_feature_mask |= _cpu_check(EINA_CPU_NEON) * CPU_FEATURE_NEON;
# endif
#endif

#if defined(__aarch64__)
# ifdef BUILD_NEON
   if (getenv("EVAS_CPU_NO_NEON"))
     cpu_feature_mask &= ~CPU_FEATURE_NEON;
   else
     cpu_feature_mask |= CPU_FEATURE_NEON;
# endif
   if (getenv("EVAS_CPU_NO_SVE"))
     cpu_feature_mask &= ~CPU_FEATURE_SVE;
   else
     cpu_feature_mask |= _cpu_check(EINA_CPU_SVE) * CPU_FEATURE_SVE;
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
