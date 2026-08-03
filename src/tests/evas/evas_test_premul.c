/* Premultiplication is the invariant the whole software compositor rests on:
 * every blend kernel assumes no colour channel exceeds its own alpha, and
 * s + MUL_256(256 - a, d) overflows its 8 bit channel the moment that stops
 * being true - carrying into the neighbouring channel and turning a barely
 * visible pixel into an opaque one of some unrelated colour.
 *
 * A NEON path that rounded one step too high was enough to do exactly that,
 * so these tests check the invariant itself rather than any one code path.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Evas.h>

#include "evas_suite.h"

/* Declared in evas_common_private.h, which pulls in far more than a test
 * needs; it is exported from libevas. */
EVAS_API unsigned int evas_common_convert_argb_premul(unsigned int *data,
                                                      unsigned int len);

#define ARGB(a, r, g, b) \
   (((unsigned int)(a) << 24) | ((unsigned int)(r) << 16) | \
    ((unsigned int)(g) << 8) | (unsigned int)(b))

/* Premultiplying must never leave a channel above the alpha it was scaled by.
 * Sweeping every alpha against the channel values most likely to round the
 * wrong way covers the whole space that matters. */
EFL_START_TEST(evas_premul_invariant)
{
   static const unsigned int vals[] =
     { 0, 1, 2, 3, 4, 127, 128, 129, 200, 250, 253, 254, 255 };
   const unsigned int nvals = sizeof(vals) / sizeof(vals[0]);
   unsigned int a, i;

   for (a = 0; a <= 255; a++)
     for (i = 0; i < nvals; i++)
       {
          /* eight identical pixels, so the vector path is used where one exists */
          unsigned int px[8];
          unsigned int k;

          for (k = 0; k < 8; k++) px[k] = ARGB(a, vals[i], vals[i], vals[i]);
          evas_common_convert_argb_premul(px, 8);

          for (k = 0; k < 8; k++)
            {
               unsigned int ga = (px[k] >> 24) & 0xff;
               unsigned int gr = (px[k] >> 16) & 0xff;
               unsigned int gg = (px[k] >> 8) & 0xff;
               unsigned int gb = px[k] & 0xff;

               ck_assert_int_eq(ga, a);
               ck_assert_msg((gr <= ga) && (gg <= ga) && (gb <= ga),
                             "alpha %u colour %u premultiplied to %08x: a "
                             "channel exceeds its alpha, which overflows the "
                             "blend and corrupts the pixel", a, vals[i], px[k]);
            }
       }
}
EFL_END_TEST

/* The vector path only engages for runs of eight or more, so premultiplying
 * the same pixels one at a time exercises the scalar fallback instead. The two
 * must agree exactly - they are the same formula. */
EFL_START_TEST(evas_premul_vector_matches_scalar)
{
   unsigned int a, c;

   for (a = 0; a <= 255; a++)
     for (c = 0; c <= 255; c++)
       {
          unsigned int bulk[8], one[8];
          unsigned int k;

          for (k = 0; k < 8; k++)
            bulk[k] = one[k] = ARGB(a, c, (c + 85) & 0xff, (c + 170) & 0xff);

          evas_common_convert_argb_premul(bulk, 8);       /* vectorised */
          for (k = 0; k < 8; k++)
            evas_common_convert_argb_premul(&one[k], 1);  /* scalar */

          for (k = 0; k < 8; k++)
            ck_assert_msg(bulk[k] == one[k],
                          "alpha %u colour %u: bulk premultiply gave %08x but "
                          "one at a time gave %08x", a, c, bulk[k], one[k]);
       }
}
EFL_END_TEST

/* The count of trivially opaque or transparent pixels feeds the sparse alpha
 * flag, so it has to be right whichever path produced it. */
EFL_START_TEST(evas_premul_trivial_count)
{
   unsigned int px[8], i, n;

   for (i = 0; i < 8; i++) px[i] = ARGB(255, 10, 20, 30);
   n = evas_common_convert_argb_premul(px, 8);
   ck_assert_int_eq(n, 8);

   for (i = 0; i < 8; i++) px[i] = ARGB(0, 10, 20, 30);
   n = evas_common_convert_argb_premul(px, 8);
   ck_assert_int_eq(n, 8);

   for (i = 0; i < 8; i++) px[i] = ARGB(128, 10, 20, 30);
   n = evas_common_convert_argb_premul(px, 8);
   ck_assert_int_eq(n, 0);
}
EFL_END_TEST

void evas_test_premul(TCase *tc)
{
   tcase_add_test(tc, evas_premul_invariant);
   tcase_add_test(tc, evas_premul_vector_matches_scalar);
   tcase_add_test(tc, evas_premul_trivial_count);
}
