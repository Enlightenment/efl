#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Pack",
       "test-widgets": ["Efl.Ui.Table"]}

   spec-meta-end */

/*
  In general:
    - If a subobject is deleted the box simply forgets about it. Never return this element again container.
*/

EFL_START_TEST(base2)
{
   ck_assert(win);
   ck_assert(widget);
}
EFL_END_TEST

static void
_setup_std_pack(Efl_Ui_Widget *wid[3])
{
   unsigned int i;

   for (i = 0; i < 3; ++i)
     {
        wid[i] = efl_add(WIDGET_CLASS, widget);
        ck_assert_int_eq(efl_pack(widget, wid[i]), EINA_TRUE);
        efl_gfx_entity_visible_set(widget, EINA_TRUE);
     }
}

/*
  pack_clear function:
  - each element must be deleted after the call
  - the container must have 0 contents after the call
  - the call must return true even if the container is empty
*/

EFL_START_TEST(pack_clear1)
{
   Efl_Ui_Widget *wid[3];
   unsigned int i;

   _setup_std_pack(wid);

   for (i = 0; i < sizeof(wid)/sizeof(Efl_Ui_Widget*); ++i)
     {
        efl_wref_add(wid[i], &wid[i]);
     }

   ck_assert_int_eq(efl_pack_clear(widget), EINA_TRUE);

   for (i = 0; i < sizeof(wid)/sizeof(Efl_Ui_Widget*); ++i)
     {
        ck_assert_ptr_eq(wid[i], NULL);
     }

   efl_test_container_content_equal(NULL, 0);
}
EFL_END_TEST

EFL_START_TEST(pack_clear2)
{
   Efl_Ui_Widget *wid[3];

   _setup_std_pack(wid);

   ck_assert_int_eq(efl_pack_clear(widget), EINA_TRUE);
   ck_assert_int_eq(efl_pack_clear(widget), EINA_TRUE);
   ck_assert_int_eq(efl_pack_clear(widget), EINA_TRUE);
   ck_assert_int_eq(efl_pack_clear(widget), EINA_TRUE);
   ck_assert_int_eq(efl_pack_clear(widget), EINA_TRUE);
}
EFL_END_TEST

/*
  unpack_all function:
  - each element must have exactly one reference after the call
  - the container must have 0 contents after the call
  - the call must return true even if the container is empty
*/

EFL_START_TEST(unpack_all1)
{
   Efl_Ui_Widget *wid[3];
   unsigned int i;

   _setup_std_pack(wid);

   ck_assert_int_eq(efl_pack_unpack_all(widget), EINA_TRUE);

   for (i = 0; i < sizeof(wid)/sizeof(Efl_Ui_Widget*); ++i)
     {
        ck_assert_int_eq(efl_ref_count(wid[i]), 1);
     }
   efl_test_container_content_equal(NULL, 0);
}
EFL_END_TEST

EFL_START_TEST(unpack_all2)
{
   Efl_Ui_Widget *wid[3];
   unsigned int i;

   _setup_std_pack(wid);

   ck_assert_int_eq(efl_pack_unpack_all(widget), EINA_TRUE);
   ck_assert_int_eq(efl_pack_clear(widget), EINA_TRUE);

   for (i = 0; i < sizeof(wid)/sizeof(Efl_Ui_Widget*); ++i)
     {
        ck_assert_int_eq(efl_ref_count(wid[i]), 1);
     }
   efl_test_container_content_equal(NULL, 0);
}
EFL_END_TEST

/*
  unpack function:
  - the element must have exactly one reference after the call
  - the container must have one content less after the call
  - the widget parent must be different from the parent
  - unpacking something invalid can return EINA_FALSE but must not print an error
*/

EFL_START_TEST(unpack1)
{
   Efl_Ui_Widget *wid[3];
   _setup_std_pack(wid);

   ck_assert_int_eq(efl_pack_unpack(widget, wid[2]), EINA_TRUE);
   ck_assert_ptr_ne(efl_ui_widget_parent_get(wid[2]), widget);
   ck_assert_int_eq(efl_ref_count(wid[2]), 1);
   efl_test_container_content_equal(wid, 2);
}
EFL_END_TEST

EFL_START_TEST(unpack2)
{
   Efl_Ui_Widget *wid[3];
   _setup_std_pack(wid);

   ck_assert_int_eq(efl_pack_unpack(widget, wid[0]), EINA_TRUE);
   ck_assert_int_eq(efl_pack(widget, wid[0]), EINA_TRUE);
   ck_assert_int_eq(efl_pack_unpack(widget, wid[0]), EINA_TRUE);
   ck_assert_int_eq(efl_pack(widget, wid[0]), EINA_TRUE);

   efl_test_container_content_equal(wid, 3);
}
EFL_END_TEST

EFL_START_TEST(unpack3)
{
   Efl_Ui_Widget *wid[3], *invalid;
   _setup_std_pack(wid);

   invalid = efl_add(WIDGET_CLASS, win);
   ck_assert_int_eq(efl_pack_unpack(widget, wid[2]), EINA_TRUE);
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_unpack(widget, wid[2]), EINA_FALSE);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_unpack(widget, wid[2]), EINA_FALSE);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_unpack(widget, (Eo*)0x111), EINA_FALSE);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_unpack(widget, invalid), EINA_FALSE);
   EXPECT_ERROR_END;
   efl_test_container_content_equal(wid, 2);
}
EFL_END_TEST

/*
  pack function:
  - element must be added to the content of the container
  - elements widget_parent must be the container
  - double adding a widdget must error
  - adding something invalid must error
*/

EFL_START_TEST(pack1)
{
   Efl_Ui_Widget *wid[3];
   unsigned int i;

   _setup_std_pack(wid);

   for (i = 0; i < sizeof(wid)/sizeof(Efl_Ui_Widget*); ++i)
     {
        ck_assert_int_eq(efl_ref_count(wid[i]), 1);
        ck_assert_ptr_eq(efl_ui_widget_parent_get(wid[i]), widget);
     }

   efl_test_container_content_equal(wid, 3);
}
EFL_END_TEST

EFL_START_TEST(pack2)
{
   Efl_Ui_Widget *wid[3];
   _setup_std_pack(wid);

   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack(widget, wid[0]), EINA_FALSE);
   EXPECT_ERROR_END;
   efl_test_container_content_equal(wid, 3);
}
EFL_END_TEST

EFL_START_TEST(pack3)
{
   Efl_Ui_Widget *wid[3];
   _setup_std_pack(wid);

   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack(widget, (Eo*)0x111F), EINA_FALSE);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack(widget, widget), EINA_FALSE);
   EXPECT_ERROR_END;
   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack(widget, NULL), EINA_FALSE);
   EXPECT_ERROR_END;
   efl_test_container_content_equal(wid, 3);
}
EFL_END_TEST

EFL_START_TEST(pack_align)
{
#define TUPLE_CHECK(H,V,rh,rv) \
  do { \
   double v, h; \
   efl_pack_align_set(widget, H, V); \
   efl_pack_align_get(widget, &h, &v); \
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
   efl_pack_padding_set(widget, H, V, S); \
   efl_pack_padding_get(widget, &h, &v, &r); \
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

EFL_START_TEST(evt_content_added)
{
   Eina_Bool called = EINA_TRUE;
   Efl_Ui_Widget *wid = efl_add(WIDGET_CLASS, win);
   efl_test_container_expect_evt_content_added(widget, EFL_CONTAINER_EVENT_CONTENT_ADDED, &called, wid);
   efl_pack(widget, wid);
   ck_assert_int_eq(called, EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(evt_content_removed)
{
   Eina_Bool called = EINA_TRUE;
   Efl_Ui_Widget *wid[3];

   _setup_std_pack(wid);

   efl_test_container_expect_evt_content_added(widget, EFL_CONTAINER_EVENT_CONTENT_REMOVED, &called, wid[1]);
   efl_pack_unpack(widget, wid[1]);
   ck_assert_int_eq(called, EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(child_killed)
{
   Efl_Ui_Widget *wid[3];
   _setup_std_pack(wid);

   efl_wref_add(wid[0], &wid[0]);
   efl_del(wid[0]);
   efl_test_container_content_equal(&wid[1], 2);
   ck_assert_ptr_eq(wid[0], NULL);
}
EFL_END_TEST

EFL_START_TEST(container_killed)
{
   Efl_Ui_Widget *wid[3];
   unsigned int i = 0;

   _setup_std_pack(wid);

   for (i = 0; i < sizeof(wid)/sizeof(Efl_Ui_Widget*); ++i)
     {
        efl_wref_add(wid[i], &wid[i]);
     }
   efl_del(widget);
   for (i = 0; i < sizeof(wid)/sizeof(Efl_Ui_Widget*); ++i)
     {
        ck_assert_ptr_eq(wid[i], NULL);
     }
}
EFL_END_TEST

void
efl_pack_behavior_test(TCase *tc)
{
   tcase_add_test(tc, base2);
   tcase_add_test(tc, pack_clear1);
   tcase_add_test(tc, pack_clear2);
   tcase_add_test(tc, unpack_all1);
   tcase_add_test(tc, unpack_all2);
   tcase_add_test(tc, unpack1);
   tcase_add_test(tc, unpack2);
   tcase_add_test(tc, unpack3);
   tcase_add_test(tc, pack1);
   tcase_add_test(tc, pack2);
   tcase_add_test(tc, pack3);
   tcase_add_test(tc, pack_align);
   tcase_add_test(tc, pack_padding);
   tcase_add_test(tc, evt_content_added);
   tcase_add_test(tc, evt_content_removed);
   tcase_add_test(tc, child_killed);
   tcase_add_test(tc, container_killed);
}
