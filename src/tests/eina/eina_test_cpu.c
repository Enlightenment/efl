#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_cpu_avx2_implies_sse3)
{
   Eina_Cpu_Features f = eina_cpu_features_get();

   /* AVX2 is a strict superset: a CPU reporting AVX2 without SSE3 means the
    * detection read the wrong CPUID leaf or register. */
   if (f & EINA_CPU_AVX2)
     {
        fail_if(!(f & EINA_CPU_SSE3));
        fail_if(!(f & EINA_CPU_SSE2));
     }
   /* The bit must be distinct from every other feature bit. */
   fail_if(EINA_CPU_AVX2 & (EINA_CPU_SSE3 | EINA_CPU_SSE41 | EINA_CPU_SSE42 |
                            EINA_CPU_SVE  | EINA_CPU_NEON  | EINA_CPU_ALTIVEC));
}
EFL_END_TEST

static Eina_Cpu_Features
_cpu_features_with_disable(const char *disable)
{
   Eina_Cpu_Features f;

   /* The suite fixture holds the only eina_init(), so this cycle runs the
    * feature detection again. */
   ck_assert_int_eq(eina_shutdown(), 0);
   setenv("EINA_CPU_DISABLE", disable, 1);
   eina_init();
   f = eina_cpu_features_get();
   unsetenv("EINA_CPU_DISABLE");
   eina_shutdown();
   eina_init();
   return f;
}

EFL_START_TEST(eina_cpu_disable_masks_listed_features)
{
   Eina_Cpu_Features detected = eina_cpu_features_get();
   Eina_Cpu_Features f;

   f = _cpu_features_with_disable("sse3");
   ck_assert_int_eq(f, detected & ~EINA_CPU_SSE3);

   f = _cpu_features_with_disable("avx2,neon,sse41");
   ck_assert_int_eq(f, detected & ~(EINA_CPU_AVX2 | EINA_CPU_NEON | EINA_CPU_SSE41));

   f = _cpu_features_with_disable("all");
   ck_assert_int_eq(f, 0);

   /* A name that is a prefix of another must not match it, and an unknown
    * name is ignored rather than taking everything else with it. */
   f = _cpu_features_with_disable("sse,bogus");
   ck_assert_int_eq(f, detected & ~EINA_CPU_SSE);

   /* Re-initializing without the variable restores the detection. */
   ck_assert_int_eq(eina_cpu_features_get(), detected);
}
EFL_END_TEST

void eina_test_cpu(TCase *tc)
{
   tcase_add_test(tc, eina_cpu_avx2_implies_sse3);
   tcase_add_test(tc, eina_cpu_disable_masks_listed_features);
}
