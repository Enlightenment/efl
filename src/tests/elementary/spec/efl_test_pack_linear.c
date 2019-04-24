#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"
#include <limits.h>

/* spec-meta-start
      {"test-interface":"Efl.Pack_Linear",
       "test-widgets": ["Efl.Ui.Box"]}
   spec-meta-end */

static void
_fill_array(Efl_Ui_Widget *wid[3])
{
   for (int i = 0; i < 3; ++i)
     {
        wid[i] = create_test_widget();
     }
}

static void
_ordering_equals(Efl_Ui_Widget **wid, unsigned int len)
{
   for (unsigned int i = 0; i < len; ++i)
     {
        ck_assert_ptr_eq(efl_pack_content_get(widget, i), wid[i]);
     }
   efl_test_container_content_equal(wid, len);
}

EFL_START_TEST(pack_begin1)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();

   _fill_array(wid);

   for (int i = 2; i >= 0; i--)
     {
        efl_pack_begin(widget, wid[i]);
        ck_assert_ptr_eq(efl_ui_widget_parent_get(wid[i]), widget);
     }
   _ordering_equals(wid, 3);
   efl_pack_begin(widget, inv);
   ck_assert_ptr_eq(efl_pack_content_get(widget, 0), inv);
   Efl_Ui_Widget *wid2[] = {inv, wid[0], wid[1], wid[2]};
   _ordering_equals(wid2, 4);
}
EFL_END_TEST

EFL_START_TEST(pack_begin2)
{
   Efl_Ui_Widget *wid[3];

   _fill_array(wid);

   for (int i = 2; i >= 0; i--)
     {
        efl_pack_begin(widget, wid[i]);
        ck_assert_ptr_eq(efl_ui_widget_parent_get(wid[i]), widget);
     }

   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_begin(widget, wid[0]), EINA_FALSE);
   EXPECT_ERROR_END;
   _ordering_equals(wid, 3);
}
EFL_END_TEST

EFL_START_TEST(pack_end1)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();

   _fill_array(wid);

   for (int i = 0; i < 3; i++)
     {
        efl_pack_end(widget, wid[i]);
        ck_assert_ptr_eq(efl_ui_widget_parent_get(wid[i]), widget);
     }

   _ordering_equals(wid, 3);
   efl_pack_end(widget, inv);
   ck_assert_ptr_eq(efl_pack_content_get(widget, 3), inv);
   Efl_Ui_Widget *wid2[] = {wid[0], wid[1], wid[2], inv};
   _ordering_equals(wid2, 4);
}
EFL_END_TEST

EFL_START_TEST(pack_end2)
{
   Efl_Ui_Widget *wid[3];

   _fill_array(wid);

   for (int i = 0; i < 3; i++)
     {
        efl_pack_end(widget, wid[i]);
        ck_assert_ptr_eq(efl_ui_widget_parent_get(wid[i]), widget);
     }

   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_end(widget, wid[0]), EINA_FALSE);
   EXPECT_ERROR_END;
   _ordering_equals(wid, 3);
}
EFL_END_TEST

EFL_START_TEST(pack_before1)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();

   _fill_array(wid);

   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);

   ck_assert_int_eq(efl_pack_before(widget, inv, wid[0]), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);
   Efl_Ui_Widget *wid2[] = {inv, wid[0], wid[1], wid[2]};
   _ordering_equals(wid2, 4);
   efl_pack_unpack(widget, inv);

   ck_assert_int_eq(efl_pack_before(widget, inv, wid[2]), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);
   Efl_Ui_Widget *wid3[] = {wid[0], wid[1], inv, wid[2]};
   _ordering_equals(wid3, 4);
}
EFL_END_TEST

EFL_START_TEST(pack_before2)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();

   _fill_array(wid);

   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);

   ck_assert_int_eq(efl_pack_before(widget, inv, wid[0]), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);
   Efl_Ui_Widget *wid2[] = {inv, wid[0], wid[1], wid[2]};
   _ordering_equals(wid2, 4);

   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_before(widget, inv, wid[2]), EINA_FALSE);
   EXPECT_ERROR_END;
}
EFL_END_TEST

EFL_START_TEST(pack_after1)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();

   _fill_array(wid);

   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);

   ck_assert_int_eq(efl_pack_after(widget, inv, wid[0]), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);
   Efl_Ui_Widget *wid2[] = {wid[0], inv, wid[1], wid[2]};
   _ordering_equals(wid2, 4);
   efl_pack_unpack(widget, inv);

   ck_assert_int_eq(efl_pack_after(widget, inv, wid[2]), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);
   Efl_Ui_Widget *wid3[] = {wid[0], wid[1], wid[2], inv};
   _ordering_equals(wid3, 4);
}
EFL_END_TEST

EFL_START_TEST(pack_after2)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();

   _fill_array(wid);

   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);

   ck_assert_int_eq(efl_pack_after(widget, inv, wid[0]), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);
   Efl_Ui_Widget *wid2[] = {wid[0], inv, wid[1], wid[2]};
   _ordering_equals(wid2, 4);

   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_pack_after(widget, inv, wid[2]), EINA_FALSE);
   EXPECT_ERROR_END;
}
EFL_END_TEST

EFL_START_TEST(pack_at1)
{
   for (int x = -3; x < 3; ++x)
     {
        Efl_Ui_Widget *wid[3];
        Efl_Ui_Widget *inv = create_test_widget();
        unsigned int i;

        _fill_array(wid);
        for (int i = 0; i < 3; i++)
          efl_pack_end(widget, wid[i]);

        efl_pack_at(widget, inv, x);
        ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);

        for (i = 0; i < 4; ++i)
          {
             Efl_Ui_Widget *w = efl_pack_content_get(widget, i);
             unsigned int place_to_expect_inv = (3 + x) % 3;
             if (i == place_to_expect_inv)
               ck_assert_ptr_eq(w, inv);
             else if (i < place_to_expect_inv)
               ck_assert_ptr_eq(w, wid[i]);
             else if (i > place_to_expect_inv)
               ck_assert_ptr_eq(w, wid[i - 1]);
          }
        efl_pack_clear(widget);
     }
}
EFL_END_TEST

EFL_START_TEST(pack_at2)
{
   for (int x = -3; x < 3; ++x)
     {
        Efl_Ui_Widget *wid[3];
        Efl_Ui_Widget *inv = create_test_widget();
        unsigned int i;

        _fill_array(wid);
        for (int i = 0; i < 3; i++)
          efl_pack_end(widget, wid[i]);

        ck_assert_int_eq(efl_pack_at(widget, inv, x), EINA_TRUE);
        ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);

        EXPECT_ERROR_START;
        ck_assert_int_eq(efl_pack_at(widget, inv, x - 1), EINA_FALSE);
        EXPECT_ERROR_END;

        for (i = 0; i < 4; ++i)
          {
             Efl_Ui_Widget *w = efl_pack_content_get(widget, i);
             unsigned int place_to_expect_inv = (3 + x) % 3;
             if (i == place_to_expect_inv)
               ck_assert_ptr_eq(w, inv);
             else if (i < place_to_expect_inv)
               ck_assert_ptr_eq(w, wid[i]);
             else if (i > place_to_expect_inv)
               ck_assert_ptr_eq(w, wid[i - 1]);
          }
        efl_pack_clear(widget);
     }
}
EFL_END_TEST


EFL_START_TEST(pack_at3)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();
   Efl_Ui_Widget *inv2 = create_test_widget();

   _fill_array(wid);
   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);

   ck_assert_int_eq(efl_pack_at(widget,  inv,-100000), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv), widget);
   ck_assert_int_eq(efl_pack_at(widget, inv2, 100000), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(inv2), widget);
   Efl_Ui_Widget *wid2[] = {inv, wid[0], wid[1], wid[2], inv2};
   _ordering_equals(wid2, 5);
}
EFL_END_TEST

EFL_START_TEST(pack_content_get1)
{
   Efl_Ui_Widget *wid[3];

   _fill_array(wid);

   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);

   for (int i = -100000; i <= 100000; i++)
     {
        if (i < -3)
          ck_assert_ptr_eq(efl_pack_content_get(widget, i), efl_pack_content_get(widget, 0));
        else if (i >= -3 && i < 3)
          ck_assert_ptr_eq(efl_pack_content_get(widget, i), wid[(i + 3) % 3]);
        else
          ck_assert_ptr_eq(efl_pack_content_get(widget, i), efl_pack_content_get(widget, 2));
     }
}
EFL_END_TEST

EFL_START_TEST(pack_index_get1)
{
   Efl_Ui_Widget *wid[3];
   Efl_Ui_Widget *inv = create_test_widget();

   _fill_array(wid);
   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);
   for (int i = 0; i < 3; i++)
     ck_assert_int_eq(efl_pack_index_get(widget, wid[i]), i);

   ck_assert_int_eq(efl_pack_index_get(widget, (void*)0xAFFE), -1);
   ck_assert_int_eq(efl_pack_index_get(widget, inv), -1);
}
EFL_END_TEST

EFL_START_TEST(pack_unpack_at1)
{
   Efl_Ui_Widget *wid[3];

   _fill_array(wid);
   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);
   for (int i = 0; i < 3; i++)
     ck_assert_int_eq(efl_pack_index_get(widget, wid[i]), i);

   ck_assert_ptr_eq(efl_pack_unpack_at(widget, 1), wid[1]);
   Efl_Ui_Widget *wid1[] = {wid[0], wid[2]};
   _ordering_equals(wid1, 2);
   ck_assert_ptr_eq(efl_pack_unpack_at(widget, 1), wid[2]);
   Efl_Ui_Widget *wid2[] = {wid[0]};
   _ordering_equals(wid2, 1);
   ck_assert_ptr_eq(efl_pack_unpack_at(widget, 0), wid[0]);
   Efl_Ui_Widget *wid3[] = {};
   _ordering_equals(wid3, 0);
}
EFL_END_TEST


EFL_START_TEST(pack_unpack_at2)
{
   Efl_Ui_Widget *wid[3];

   _fill_array(wid);
   for (int i = 0; i < 3; i++)
     efl_pack_end(widget, wid[i]);
   for (int i = 0; i < 3; i++)
     ck_assert_int_eq(efl_pack_index_get(widget, wid[i]), i);

   ck_assert_ptr_eq(efl_pack_unpack_at(widget, 1), wid[1]);
   efl_del(widget);
   ck_assert_int_eq(efl_ref_count(wid[1]), 1);
   ck_assert_int_eq(efl_ref_count(widget), 0);
}
EFL_END_TEST

EFL_START_TEST(pack_unpack_at3)
{
   for (int x = -3; x < 3; ++x)
     {
        Efl_Ui_Widget *wid[3];

        _fill_array(wid);
        for (int i = 0; i < 3; i++)
          efl_pack_end(widget, wid[i]);
        ck_assert_ptr_eq(efl_pack_unpack_at(widget, x), wid[(3+x)%3]);
        ck_assert_int_eq(efl_content_count(widget), 2);
        ck_assert_ptr_ne(efl_ui_widget_parent_get(wid[(3+x)%3]), widget);
        efl_pack_unpack_all(widget);
     }
}
EFL_END_TEST

void
efl_pack_linear_behavior_test(TCase *tc)
{
   tcase_add_test(tc, pack_begin1);
   tcase_add_test(tc, pack_begin2);
   tcase_add_test(tc, pack_end1);
   tcase_add_test(tc, pack_end2);
   tcase_add_test(tc, pack_before1);
   tcase_add_test(tc, pack_before2);
   tcase_add_test(tc, pack_after1);
   tcase_add_test(tc, pack_after2);
   tcase_add_test(tc, pack_at1);
   tcase_add_test(tc, pack_at2);
   tcase_add_test(tc, pack_at3);
   tcase_add_test(tc, pack_content_get1);
   tcase_add_test(tc, pack_index_get1);
   tcase_add_test(tc, pack_unpack_at1);
   tcase_add_test(tc, pack_unpack_at2);
   tcase_add_test(tc, pack_unpack_at3);
   efl_pack_behavior_test(tc);
}
