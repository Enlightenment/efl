/* Differential test: C reference vs NEON for the gaussian blur steps.
 *
 * The blur steps are static inline templates instantiated per direction and
 * per format, so unlike the op tables there is no dispatch table to walk.
 * Instead this instantiates each template twice under different names - once
 * from the C file and once from the NEON one - and runs the pair over
 * identical buffers.
 *
 * Buffers are over allocated and compared in full, so a step that writes
 * outside the region it was given is caught as well as one that computes the
 * wrong pixel.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef BUILD_NEON
# include <arm_neon.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_filter.h"

int _evas_filter_log_dom = -1;

/* the templates log through CRI on a division by zero; keep them satisfied
 * without dragging in the whole filter machinery */
#undef CRI
#define CRI(...) do { printf("  kernel reported: " __VA_ARGS__); printf("\n"); } while (0)

#define FUNCTION_NAME blur_h_rgba_c
#define STEP 1
#include "blur/blur_gaussian_rgba_.c"

#define FUNCTION_NAME blur_v_rgba_c
#define STEP loops
#include "blur/blur_gaussian_rgba_.c"

#define FUNCTION_NAME blur_h_alpha_c
#define STEP 1
#include "blur/blur_gaussian_alpha_.c"

#define FUNCTION_NAME blur_v_alpha_c
#define STEP loops
#include "blur/blur_gaussian_alpha_.c"

#ifdef BUILD_NEON
# define FUNCTION_NAME blur_h_rgba_neon
# define STEP 1
# define BLUR_NEON 1
# include "blur/blur_gaussian_rgba_.c"

# define FUNCTION_NAME blur_v_rgba_neon
# define STEP loops
# define BLUR_NEON 1
# include "blur/blur_gaussian_rgba_.c"

# define FUNCTION_NAME blur_h_alpha_neon
# define STEP 1
# define BLUR_NEON 1
# include "blur/blur_gaussian_alpha_.c"

# define FUNCTION_NAME blur_v_alpha_neon
# define STEP loops
# define BLUR_NEON 1
# include "blur/blur_gaussian_alpha_.c"
#endif

/*--------------------------------------------------------------------------*/

#include "blur/blur_weights_.c"

#define GUARD 0xA5
#define PAD   64        /* words of slack either side of every buffer */

static unsigned int rng_state = 1;

static unsigned int
rnd(void)
{
   unsigned int x = rng_state;
   x ^= x << 13; x ^= x >> 17; x ^= x << 5;
   rng_state = x;
   return x;
}

/* Source buffers are placed hard against an unmapped page so that any read
 * past the end of the data faults instead of quietly returning whatever was
 * next in the heap. The ramps used to walk off the end of a line whenever
 * len < 2 * radius + 1, and that is invisible to a plain malloc. */
static void *
guarded_alloc(size_t bytes, size_t *out_off)
{
   const size_t page = (size_t) sysconf(_SC_PAGESIZE);
   const size_t body = ((bytes + page - 1) / page) * page;
   char *p = mmap(NULL, body + page, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

   if (p == MAP_FAILED) { perror("mmap"); exit(2); }
   mprotect(p + body, page, PROT_NONE);
   *out_off = body - bytes;      /* butt the data up against the guard */
   return p;
}

static void
guarded_free(void *p, size_t bytes)
{
   const size_t page = (size_t) sysconf(_SC_PAGESIZE);

   munmap(p, ((bytes + page - 1) / page) * page + page);
}

typedef void (*Blur_Rgba)(const DATA32 *, DATA32 *, int, int, int, int,
                          const int *, int);
typedef void (*Blur_Alpha)(const DATA8 *, DATA8 *, int, int, int, int,
                           const int *, int);

static int verbose = 0;
static int reported = 0;
static int max_report = 8;

/* Both directions address the buffer as len * loops elements: horizontal walks
 * a row of len and steps loopstep = len between rows, vertical walks a column
 * of len with STEP = loops and steps one column at a time. */
static int
compare(const char *name, int radius, int len, int loops,
        const unsigned char *ref, const unsigned char *got, size_t bytes)
{
   size_t i;
   int bad = 0, worst = 0;

   for (i = 0; i < bytes; i++)
     {
        int d;

        if (ref[i] == got[i]) continue;
        bad++;
        d = ref[i] > got[i] ? ref[i] - got[i] : got[i] - ref[i];
        if (d > worst) worst = d;
        if ((verbose || bad == 1) && reported < max_report)
          {
             reported++;
             printf("  %s r=%d len=%d loops=%d: byte %zd ref=%02x neon=%02x\n",
                    name, radius, len, loops, (ssize_t)i - PAD, ref[i], got[i]);
          }
     }
   if (bad)
     printf("  %-12s radius=%-3d len=%-4d loops=%-4d differing bytes=%d worst=%d\n",
            name, radius, len, loops, bad, worst);
   return bad;
}

static int
run_rgba(const char *name, Blur_Rgba fc, Blur_Rgba fn,
         int radius, int len, int loops, int loopstep)
{
   const size_t words = (size_t)len * loops + 2 * PAD;
   const size_t bytes = words * sizeof(DATA32);
   size_t soff;
   char *sbase = guarded_alloc(bytes, &soff);
   DATA32 *src = (DATA32 *)(sbase + soff);
   DATA32 *ref = malloc(bytes), *got = malloc(bytes);
   int *weights = malloc((2 * radius + 1) * sizeof(int));
   int pow2 = 0, bad;
   size_t i;

   memset(ref, GUARD, bytes);
   for (i = 0; i < words; i++) src[i] = rnd();
   memcpy(got, ref, bytes);
   evas_blur_weights_get(weights, &pow2, radius);

   fc(src + PAD, ref + PAD, radius, len, loops, loopstep, weights, pow2);
   fn(src + PAD, got + PAD, radius, len, loops, loopstep, weights, pow2);

   bad = compare(name, radius, len, loops,
                 (unsigned char *)ref, (unsigned char *)got, bytes);
   guarded_free(sbase, bytes); free(ref); free(got); free(weights);
   return bad;
}

static int
run_alpha(const char *name, Blur_Alpha fc, Blur_Alpha fn,
          int radius, int len, int loops, int loopstep)
{
   const size_t bytes = (size_t)len * loops + 2 * PAD;
   size_t soff;
   char *sbase = guarded_alloc(bytes, &soff);
   DATA8 *src = (DATA8 *)(sbase + soff);
   DATA8 *ref = malloc(bytes), *got = malloc(bytes);
   int *weights = malloc((2 * radius + 1) * sizeof(int));
   int pow2 = 0, bad;
   size_t i;

   memset(ref, GUARD, bytes);
   for (i = 0; i < bytes; i++) src[i] = (DATA8)rnd();
   memcpy(got, ref, bytes);
   evas_blur_weights_get(weights, &pow2, radius);

   fc(src + PAD, ref + PAD, radius, len, loops, loopstep, weights, pow2);
   fn(src + PAD, got + PAD, radius, len, loops, loopstep, weights, pow2);

   bad = compare(name, radius, len, loops, ref, got, bytes);
   guarded_free(sbase, bytes); free(ref); free(got); free(weights);
   return bad;
}

/*--------------------------------------------------------------------------*/

static double
now_sec(void)
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* C and NEON are timed alternately inside one trial loop and the best of N is
 * reported, for the same reasons as the op table benchmark: on a loaded or
 * big.LITTLE machine separate batches are not comparable. Pin with taskset. */
static void
bench_rgba(const char *name, Blur_Rgba fc, Blur_Rgba fn,
           int radius, int len, int loops, int loopstep, int trials)
{
   const size_t words = (size_t)len * loops + 2 * PAD;
   DATA32 *src = malloc(words * sizeof(DATA32));
   DATA32 *dst = malloc(words * sizeof(DATA32));
   int *weights = malloc((2 * radius + 1) * sizeof(int));
   double bc = 1e30, bn = 1e30;
   int pow2 = 0, t;
   size_t i;

   for (i = 0; i < words; i++) src[i] = rnd();
   evas_blur_weights_get(weights, &pow2, radius);

   fc(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);
   fn(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);

   for (t = 0; t < trials; t++)
     {
        double t0 = now_sec(), d;

        fc(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);
        d = now_sec() - t0;
        if (d < bc) bc = d;

        t0 = now_sec();
        fn(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);
        d = now_sec() - t0;
        if (d < bn) bn = d;
     }

   printf("  %-12s radius=%-3d %4dx%-4d  C %7.1f Mpx/s  NEON %7.1f Mpx/s  %5.2fx\n",
          name, radius, len, loops,
          (double)len * loops / bc / 1e6, (double)len * loops / bn / 1e6, bc / bn);
   free(src); free(dst); free(weights);
}

static void
bench_alpha(const char *name, Blur_Alpha fc, Blur_Alpha fn,
            int radius, int len, int loops, int loopstep, int trials)
{
   const size_t bytes = (size_t)len * loops + 2 * PAD;
   DATA8 *src = malloc(bytes), *dst = malloc(bytes);
   int *weights = malloc((2 * radius + 1) * sizeof(int));
   double bc = 1e30, bn = 1e30;
   int pow2 = 0, t;
   size_t i;

   for (i = 0; i < bytes; i++) src[i] = (DATA8)rnd();
   evas_blur_weights_get(weights, &pow2, radius);

   fc(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);
   fn(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);

   for (t = 0; t < trials; t++)
     {
        double t0 = now_sec(), d;

        fc(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);
        d = now_sec() - t0;
        if (d < bc) bc = d;

        t0 = now_sec();
        fn(src + PAD, dst + PAD, radius, len, loops, loopstep, weights, pow2);
        d = now_sec() - t0;
        if (d < bn) bn = d;
     }

   printf("  %-12s radius=%-3d %4dx%-4d  C %7.1f Mpx/s  NEON %7.1f Mpx/s  %5.2fx\n",
          name, radius, len, loops,
          (double)len * loops / bc / 1e6, (double)len * loops / bn / 1e6, bc / bn);
   free(src); free(dst); free(weights);
}

int
main(int argc, char **argv)
{
#ifndef BUILD_NEON
   (void)argc; (void)argv;
   printf("built without BUILD_NEON - nothing to compare\n");
   return 77;
#else
   /* radii around the edges of the middle loop, lengths around the vector
    * width and around 2*radius where the middle section vanishes */
   static const int radii[] = { 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 40 };
   static const int lens[]  = { 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32,
                                33, 63, 64, 65, 100, 127, 128, 129, 256, 720 };
   int failures = 0, cases = 0, i, j;
   unsigned int seed = 1;

   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-v")) verbose = 1;
        else if (!strncmp(argv[i], "--seed=", 7)) seed = (unsigned)atoi(argv[i] + 7);
     }
   rng_state = seed ? seed : 1;

   for (i = 1; i < argc; i++)
     if (!strcmp(argv[i], "--bench"))
       {
          static const int br[] = { 2, 5, 10, 20, 40 };
          int b;

          printf("gaussian blur throughput, 720x480, best of 9\n\n");
          for (b = 0; b < (int)(sizeof(br) / sizeof(br[0])); b++)
            {
               bench_rgba("horiz rgba", blur_h_rgba_c, blur_h_rgba_neon,
                          br[b], 720, 480, 720, 9);
               bench_rgba("vert rgba", blur_v_rgba_c, blur_v_rgba_neon,
                          br[b], 480, 720, 1, 9);
               bench_alpha("horiz alpha", blur_h_alpha_c, blur_h_alpha_neon,
                           br[b], 720, 480, 720, 9);
               bench_alpha("vert alpha", blur_v_alpha_c, blur_v_alpha_neon,
                           br[b], 480, 720, 1, 9);
            }
          return 0;
       }

   printf("gaussian blur C vs NEON differential test (seed=%u)\n\n", seed);

   /* loops must straddle the column block width too: the vertical kernel
    * vectorises across columns, so a fixed small value would leave that path
    * unexercised entirely */
   for (i = 0; i < (int)(sizeof(radii) / sizeof(radii[0])); i++)
     for (j = 0; j < (int)(sizeof(lens) / sizeof(lens[0])); j++)
       {
          const int r = radii[i], len = lens[j];
          static const int loopvals[] = { 1, 3, 4, 5, 8, 17 };
          int li;

          if (len < 1) continue;

          for (li = 0; li < (int)(sizeof(loopvals) / sizeof(loopvals[0])); li++)
       {
          const int loops = loopvals[li];

          failures += run_rgba("horiz rgba", blur_h_rgba_c, blur_h_rgba_neon,
                               r, len, loops, len);
          failures += run_rgba("vert rgba", blur_v_rgba_c, blur_v_rgba_neon,
                               r, len, loops, 1);
          failures += run_alpha("horiz alpha", blur_h_alpha_c, blur_h_alpha_neon,
                                r, len, loops, len);
          failures += run_alpha("vert alpha", blur_v_alpha_c, blur_v_alpha_neon,
                                r, len, loops, 1);
          cases += 4;
       }
       }

   printf("\n--- summary ---\n");
   printf("configurations : %d\n", cases);
   printf("differing bytes: %d\n", failures);
   if (failures)
     {
        printf("RESULT: FAIL\n");
        return 1;
     }
   printf("RESULT: PASS (bit exact)\n");
   return 0;
#endif
}
