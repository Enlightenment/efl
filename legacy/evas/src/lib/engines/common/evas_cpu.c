#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

void
evas_common_cpu_init(void)
{
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
#ifdef BUILD_MMX
#ifndef HARD_CODED_P3
#ifndef HARD_CODED_P2
   static int do_mmx = 0, do_sse = 0, do_sse2 = 0, done = 0;
   unsigned int have_cpu_id;
#endif
#endif
#endif
   
#ifndef BUILD_MMX
   *mmx = 0;
   *sse = 0;
   *sse2 = 0;
   return;
#else   
# ifdef HARD_CODED_P3
   *mmx = 1;
   *sse = 1;
   *sse2 = 0;
   return;
# else
#  ifdef HARD_CODED_P2
   *mmx = 1;
   *sse = 0;
   *sse2 = 0;
   return;
#  else
   if (done)
     {
	*mmx = do_mmx;
	*sse = do_sse;
	*sse2 = do_sse2;
	return;
     }
   
#   ifdef BUILD_MMX
   have_cpu_id = evas_common_cpu_have_cpuid();
   if (have_cpu_id)
     {
	unsigned int cpu_id;
	
	cpu_id = 0;
	get_cpuid(cpu_id);
	if (cpu_id & CPUID_MMX) do_mmx = 1;
	if (cpu_id & CPUID_SSE) do_sse = 1;
	if (cpu_id & CPUID_SSE2) do_sse2 = 1;
     }
#   endif  
   *mmx = do_mmx;
   *sse = do_sse;
   *sse2 = do_sse2;
   done = 1;
#  endif
# endif  
#endif   
}

#ifdef BUILD_MMX
void
evas_common_cpu_end_opt(void)
{
   emms();
}
#else
void
evas_common_cpu_end_opt(void)
{
}
#endif
