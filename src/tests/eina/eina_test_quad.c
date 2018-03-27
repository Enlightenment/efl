#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <assert.h>

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_quad_simple)
{
   Eina_Rectangle r1;
   Eina_Quad r2;
   double x0, x1, x2, x3;
   double y0, y1, y2, y3;


   eina_quad_coords_set(&r2,
                        0.0, 5.0,
                        5.0, 0.0,
                       10.0, 5.0,
                        5.0, 10.0);

   eina_quad_coords_get(&r2,
                        &x0, &y0,
                        &x1, &y1,
                        &x2, &y2,
                        &x3, &y3);
   fail_if (!EINA_FLT_EQ(r2.x0, 0.0) ||
            !EINA_FLT_EQ(r2.y0, 5.0) ||
            !EINA_FLT_EQ(r2.x1, 5.0) ||
            !EINA_FLT_EQ(r2.y1, 0.0) ||
            !EINA_FLT_EQ(r2.x2, 10.0) ||
            !EINA_FLT_EQ(r2.y2, 5.0) ||
            !EINA_FLT_EQ(r2.x3, 5.0) ||
            !EINA_FLT_EQ(r2.y3, 10.0));

   eina_quad_rectangle_to(&r2, &r1);
   fail_if (!EINA_FLT_EQ(r1.x, 0) ||
            !EINA_FLT_EQ(r1.y, 0) ||
            !EINA_FLT_EQ(r1.w, 10) ||
            !EINA_FLT_EQ(r1.h, 10));

   eina_quad_coords_set(&r2,
                        0.0, 0.0,
                        0.0, 0.0,
                        0.0, 0.0,
                        0.0, 0.0);
   eina_quad_rectangle_to(&r2, &r1);
   fail_if (!EINA_FLT_EQ(r1.x, 0) ||
            !EINA_FLT_EQ(r1.y, 0) ||
            !EINA_FLT_EQ(r1.w, 0) ||
            !EINA_FLT_EQ(r1.h, 0));

   EINA_RECTANGLE_SET(&r1, 5, 10, 20, 30);
   eina_quad_rectangle_from(&r2, &r1);
   fail_if (!EINA_FLT_EQ(r2.x0, 5.0) ||
            !EINA_FLT_EQ(r2.y0, 10.0) ||
            !EINA_FLT_EQ(r2.x1, 25.0) ||
            !EINA_FLT_EQ(r2.y1, 10.0) ||
            !EINA_FLT_EQ(r2.x2, 25.0) ||
            !EINA_FLT_EQ(r2.y2, 40.0) ||
            !EINA_FLT_EQ(r2.x3, 5.0) ||
            !EINA_FLT_EQ(r2.y3, 40.0));

}
EFL_END_TEST

void
eina_test_quad(TCase *tc)
{
   tcase_add_test(tc, eina_quad_simple);
}
