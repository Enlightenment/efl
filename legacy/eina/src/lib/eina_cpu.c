/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_cpu.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
/* FIXME this ifdefs should be replaced */
#if defined(__i386__) || defined(__x86_64__)
/* We save ebx and restore it to be PIC compatible */
static inline void _x86_cpuid(int op, int *a, int *b, int *c, int *d)
{
	asm volatile(
#if defined(__x86_64__)
		"pushq %%rbx      \n\t" /* save %ebx */
#else
		"pushl %%ebx      \n\t" /* save %ebx */
#endif
		"cpuid            \n\t"
		"movl %%ebx, %1   \n\t" /* save what cpuid just put in %ebx */
#if defined(__x86_64__)
		"popq %%rbx       \n\t" /* restore the old %ebx */
#else
		"popl %%ebx       \n\t" /* restore the old %ebx */
#endif
		: "=a"(*a), "=r"(*b), "=c"(*c), "=d"(*d)
		: "a"(op)
		: "cc");
}

void _x86_simd(Eina_Cpu_Features *features)
{
	int a, b, c, d;

	_x86_cpuid(1, &a, &b, &c, &d);
	/*
	 * edx
	 * 18 = PN (Processor Number)
	 * 19 = CLFlush (Cache Line Flush)
	 * 23 = MMX
	 * 25 = SSE
	 * 26 = SSE2
	 * 28 = HTT (Hyper Threading)
	 * ecx
	 * 0 = SSE3
	 */
	if ((d >> 23) & 1)
		*features |= EINA_CPU_MMX;
	if ((d >> 25) & 1)
		*features |= EINA_CPU_SSE;
	if ((d >> 26) & 1)
		*features |= EINA_CPU_SSE2;
	if (c & 1)
		*features |= EINA_CPU_SSE3;
}
#endif
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/* FIXME the features checks should be called when this function is called?
 * or make it static by doing eina_cpu_init() and return a local var
 */
/**
 *
 * @return
 */
EAPI Eina_Cpu_Features eina_cpu_features_get(void)
{
	Eina_Cpu_Features ecf = 0;
#if defined(__i386__) || defined(__x86_64__)
	_x86_simd(&ecf);
#endif
	return ecf;
}
