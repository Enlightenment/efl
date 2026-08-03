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

void eina_test_cpu(TCase *tc)
{
   tcase_add_test(tc, eina_cpu_avx2_implies_sse3);
}
