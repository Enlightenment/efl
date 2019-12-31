#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Gfx.View",
       "test-widgets": ["Efl.Ui.Vg_Animation"]}
   spec-meta-end */

EFL_START_TEST(view_size)
{
#define TUPLE_CHECK(W, H) \
  do { \
   Eina_Size2D sz; \
   efl_gfx_view_size_set(widget, EINA_SIZE2D(W, H)); \
   sz = efl_gfx_view_size_get(widget); \
   ck_assert(W == sz.w); \
   ck_assert(H == sz.h); \
  } while(0);

  TUPLE_CHECK(100, 100);
  TUPLE_CHECK(100, 200);
  TUPLE_CHECK(200, 100);
  TUPLE_CHECK(800, 700);
  TUPLE_CHECK(10, 50);

#undef TUPLE_CHECK
}
EFL_END_TEST

void
efl_gfx_view_behavior_test(TCase *tc)
{
   tcase_add_test(tc, view_size);
}
