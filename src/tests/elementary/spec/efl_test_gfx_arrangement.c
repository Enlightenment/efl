#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Gfx.Arrangement",
       "test-widgets": ["Efl.Ui.Box", "Efl.Ui.Table"]}
   spec-meta-end */

EFL_START_TEST(pack_align)
{
#define TUPLE_CHECK(H,V,rh,rv) \
  do { \
   double v, h; \
   efl_gfx_arrangement_content_align_set(widget, H, V); \
   efl_gfx_arrangement_content_align_get(widget, &h, &v); \
   ck_assert(v == rv); \
   ck_assert(h == rh); \
  } while(0);

  TUPLE_CHECK(  1.0,   1.0,  1.0,  1.0);
  TUPLE_CHECK(  0.0,   0.0,  0.0,  0.0);
  TUPLE_CHECK(- 1.0, - 1.0, -1.0, -1.0);
  TUPLE_CHECK(-42.0, -42.0, -1.0, -1.0);
  TUPLE_CHECK( 42.0,  42.0,  1.0,  1.0);
  TUPLE_CHECK(-42.0,  42.0, -1.0,  1.0);
  TUPLE_CHECK( 42.0, -42.0,  1.0, -1.0);
#undef TUPLE_CHECK
}
EFL_END_TEST

EFL_START_TEST(pack_padding)
{
#define TUPLE_CHECK(H, V, rh, rv, S, rs) \
  do { \
   double v, h; \
   Eina_Bool r; \
   efl_gfx_arrangement_content_padding_set(widget, H, V, S); \
   efl_gfx_arrangement_content_padding_get(widget, &h, &v, &r); \
   ck_assert(v == rv); \
   ck_assert(h == rh); \
   ck_assert_int_eq(r, S); \
  } while(0);

  TUPLE_CHECK( 0.0, 0.0, 0.0, 0.0, EINA_TRUE, EINA_TRUE);
  TUPLE_CHECK( -1.0, -123.0, 0.0, 0.0, EINA_FALSE, EINA_FALSE);
  TUPLE_CHECK( -1.0,  123.0, 0.0, 123.0, EINA_FALSE, EINA_FALSE);
#undef TUPLE_CHECK
}
EFL_END_TEST

void
efl_gfx_arrangement_behavior_test(TCase *tc)
{
   tcase_add_test(tc, pack_align);
   tcase_add_test(tc, pack_padding);
}
