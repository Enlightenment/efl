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
   ck_assert(EINA_DBL_EQ(v, rv)); \
   ck_assert(EINA_DBL_EQ(h, rh)); \
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
#define TUPLE_CHECK(H, V, rh, rv) \
  do { \
   unsigned int v, h; \
   efl_gfx_arrangement_content_padding_set(widget, H, V); \
   efl_gfx_arrangement_content_padding_get(widget, &h, &v); \
   ck_assert(v == rv); \
   ck_assert(h == rh); \
  } while(0);

  TUPLE_CHECK( 0, 0, 0, 0);
  TUPLE_CHECK( 1,  123, 1, 123);
#undef TUPLE_CHECK
}
EFL_END_TEST

void
efl_gfx_arrangement_behavior_test(TCase *tc)
{
   tcase_add_test(tc, pack_align);
   tcase_add_test(tc, pack_padding);
}
