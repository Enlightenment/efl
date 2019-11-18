#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

EFL_START_TEST(efl_canvas_animation_negative_double_checking)
{
   Efl_Canvas_Animation *animation = efl_new(EFL_CANVAS_ANIMATION_CLASS);

   efl_animation_duration_set(animation, 1.0);
   ck_assert_int_eq(efl_animation_duration_get(animation), 1.0);
   efl_animation_duration_set(animation, 3.0);
   ck_assert_int_eq(efl_animation_duration_get(animation), 3.0);
   EXPECT_ERROR_START;
   efl_animation_duration_set(animation, -1.0);
   ck_assert_int_eq(efl_animation_duration_get(animation), 3.0);
   EXPECT_ERROR_END;

   efl_animation_start_delay_set(animation, 1.0);
   ck_assert(EINA_DBL_EQ(efl_animation_start_delay_get(animation), 1.0));
   efl_animation_start_delay_set(animation, 0.0);
   ck_assert(EINA_DBL_EQ(efl_animation_start_delay_get(animation), 0.0));
   EXPECT_ERROR_START;
   efl_animation_start_delay_set(animation, -1.0);
   ck_assert_int_eq(efl_animation_start_delay_get(animation), 0.0);
   EXPECT_ERROR_END;

}
EFL_END_TEST

void efl_test_canvas_animation(TCase *tc)
{
   tcase_add_test(tc, efl_canvas_animation_negative_double_checking);
}
