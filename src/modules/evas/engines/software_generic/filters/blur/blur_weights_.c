/* @file blur_weights_.c
 * The weight curve shared by the gaussian blur and by its differential test.
 *
 * It lives in its own file so the test cannot drift from the curve the filter
 * actually uses: testing a NEON kernel against a C kernel is only meaningful
 * if both are fed the weights production will feed them.
 *
 * Base curve: f(x) = sin(x + pi/2) / 2 + 1/2, normalised so that
 * sum(weights) == 1 << *pow2_divider, which is what lets the middle of the
 * blur divide with a shift instead of a division.
 */

#ifndef EVAS_BLUR_WEIGHTS_H
#define EVAS_BLUR_WEIGHTS_H

#include <math.h>

static inline void
evas_blur_weights_get(int *weights, int *pow2_divider, int radius)
{
   const int diameter = 2 * radius + 1;
   double x, divider, sum = 0.0;
   double dweights[diameter];
   int k, nextpow2, isum = 0;
   const int FAKE_PI = 3.0;

   /* Base curve:
    * f(x) = sin(x+pi/2)/2+1/2
    */

   for (k = 0; k < diameter; k++)
     {
        x = ((double) k / (double) (diameter - 1)) * FAKE_PI * 2.0 - FAKE_PI;
        dweights[k] = ((sin(x + M_PI_2) + 1.0) / 2.0) * 1024.0;
        sum += dweights[k];
     }

   // Now we need to normalize to have a 2^N divider.
   nextpow2 = log2(2 * sum);
   divider = (double) (1 << nextpow2);

   for (k = 0; k < diameter; k++)
     {
        weights[k] = round(dweights[k] * divider / sum);
        isum += weights[k];
     }

   // Final correction. The difference SHOULD be small...
   weights[radius] += (int) divider - isum;

   if (pow2_divider)
     *pow2_divider = nextpow2;
}

#endif
