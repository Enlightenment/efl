//#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_widget.h"
#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "eo_internal.h"

EFL_CLASS_SIMPLE_CLASS(efl_ui_widget, "efl_ui_widget", EFL_UI_WIDGET_CLASS)

typedef struct {
   Efl_Ui_Widget *btn1, *btn2;
   Efl_Ui_Widget *box;
   Efl_Ui_Widget *win;
   Efl_Ui_Widget *ic;
} State;

static void
_small_ui(State *s)
{
   s->win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Hello World"));

   s->ic = efl_add(EFL_UI_IMAGE_CLASS, s->win,
                 efl_ui_win_icon_object_set(s->win, efl_added));

   s->box = efl_add(EFL_UI_BOX_CLASS, s->win,
                efl_content_set(s->win, efl_added));

   s->btn1 = efl_add(EFL_UI_BUTTON_CLASS, s->box,
                efl_text_set(efl_added, "Quit1"),
                efl_pack(s->box, efl_added));

   s->btn2 = efl_add(EFL_UI_BUTTON_CLASS, s->box,
                efl_text_set(efl_added, "Quit"),
                efl_pack(s->box, efl_added));

}

EFL_START_TEST(efl_ui_test_widget_parent_iterator)
{
   Eina_Iterator *it;
   Efl_Ui_Widget *o;
   Eina_Array *a;
   State s;

   a = eina_array_new(10);
   _small_ui(&s);
   eina_array_push(a, s.win);
   eina_array_push(a, s.box);
   eina_array_push(a, s.btn1);

   it = efl_ui_widget_parent_iterator(s.btn1);
   EINA_ITERATOR_FOREACH(it, o)
     {
        ck_assert_ptr_eq(eina_array_pop(a), o);
     }
   eina_iterator_free(it);
   ck_assert_int_eq(eina_array_count(a), 0);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_widget_widget_iterator)
{
   Eina_Iterator *it;
   Efl_Ui_Widget *o;
   Eina_Array *a;
   State s;

   a = eina_array_new(10);
   _small_ui(&s);
   eina_array_push(a, s.btn2);
   eina_array_push(a, s.btn1);
   eina_array_push(a, s.box);
   eina_array_push(a, s.ic); //Hack arround the icon of the window
   eina_array_push(a, s.win);

   it = efl_ui_widget_tree_widget_iterator(s.win);
   EINA_ITERATOR_FOREACH(it, o)
     {
        Eo *c = eina_array_pop(a);
        ck_assert_ptr_eq(c, o);
     }
   eina_iterator_free(it);
   ck_assert_int_eq(eina_array_count(a), 0);
}
EFL_END_TEST

static Evas_Object*
resize_object(Efl_Canvas_Object *o)
{
   Efl_Ui_Widget_Data *pd = efl_data_scope_safe_get(o, EFL_UI_WIDGET_CLASS);

   return eina_list_data_get(pd->subobjs);
}

EFL_START_TEST(efl_ui_test_widget_widget_sub_iterator)
{
   Eina_Iterator *it;
   Efl_Ui_Widget *o;
   Eina_Array *a;
   State s;

   a = eina_array_new(10);
   _small_ui(&s);
   eina_array_push(a, s.btn2);
   eina_array_push(a, s.btn1);
   eina_array_push(a, s.box);

   it = efl_ui_widget_tree_widget_iterator(s.box);
   EINA_ITERATOR_FOREACH(it, o)
     {
        ck_assert_ptr_eq(eina_array_pop(a), o);
     }
   eina_iterator_free(it);
   ck_assert_int_eq(eina_array_count(a), 0);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_widget_iterator)
{
   Eina_Iterator *it;
   Efl_Ui_Widget *o;
   Eina_Array *a;
   State s;

   a = eina_array_new(10);
   _small_ui(&s);
   eina_array_push(a, resize_object(s.btn2));
   eina_array_push(a, s.btn2);
   eina_array_push(a, resize_object(s.btn1));
   eina_array_push(a, s.btn1);
   eina_array_push(a, resize_object(s.box));
   eina_array_push(a, s.box);
   eina_array_push(a, resize_object(s.ic));
   eina_array_push(a, s.ic);
   eina_array_push(a, s.win);

   it = efl_ui_widget_tree_iterator(s.win);
   EINA_ITERATOR_FOREACH(it, o)
     {
        ck_assert_ptr_eq(eina_array_pop(a), o);
     }
   eina_iterator_free(it);
   ck_assert_int_eq(eina_array_count(a), 0);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_widget_sub_iterator)
{
   Eina_Iterator *it;
   Efl_Ui_Widget *o;
   Eina_Array *a;
   State s;

   a = eina_array_new(10);
   _small_ui(&s);
   eina_array_push(a, resize_object(s.btn2));
   eina_array_push(a, s.btn2);
   eina_array_push(a, resize_object(s.btn1));
   eina_array_push(a, s.btn1);
   eina_array_push(a, resize_object(s.box));
   eina_array_push(a, s.box);

   it = efl_ui_widget_tree_iterator(s.box);
   EINA_ITERATOR_FOREACH(it, o)
     {
        ck_assert_ptr_eq(eina_array_pop(a), o);
     }
   eina_iterator_free(it);
   ck_assert_int_eq(eina_array_count(a), 0);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_widget_sub_object_add_del)
{
   State s;

   _small_ui(&s);
   DISABLE_ABORT_ON_CRITICAL_START;
   ck_assert(!efl_ui_widget_sub_object_add(s.btn1, s.btn1));
   DISABLE_ABORT_ON_CRITICAL_END;
   ck_assert(efl_ui_widget_sub_object_add(s.box, s.btn1));
   DISABLE_ABORT_ON_CRITICAL_START;
   ck_assert(!efl_ui_widget_sub_object_add(s.box, NULL));
   ck_assert(!efl_ui_widget_sub_object_del(s.btn1, s.btn1));
   ck_assert(!efl_ui_widget_sub_object_del(s.box, NULL));
   ck_assert(!efl_ui_widget_sub_object_del(s.btn1, s.box));
   DISABLE_ABORT_ON_CRITICAL_END;
   ck_assert(efl_ui_widget_sub_object_del(s.box, s.btn1));
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_widget_sub_object_theme_sync)
{
   State s;
   Efl_Canvas_Layout *edje;

   _small_ui(&s);
   edje = elm_widget_resize_object_get(s.btn1);

   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn1), 1.0));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(edje), 1.0));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn2), 1.0));

   efl_gfx_entity_scale_set(s.win, 0.123);
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn1), 0.123));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(edje), 0.123));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn2), 0.123));

   efl_ui_widget_sub_object_del(s.box, s.btn1);
   efl_gfx_entity_scale_set(s.win, 0.456);
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn1), 1.0));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(edje), 0.123));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn2), 0.456));

   efl_gfx_entity_scale_set(s.win, 0.789);
   efl_ui_widget_sub_object_add(s.box, s.btn1);
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn1), 0.789));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(edje), 0.789));
   ck_assert(EINA_DBL_EQ(efl_gfx_entity_scale_get(s.btn2), 0.789));
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_widget_parent_relation)
{
   Efl_Ui_Win *win, *w1, *w2, *w3;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Hello World"));
   //first check the initial state
   w1 = efl_add(efl_ui_widget_realized_class_get(), win);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(w1), win);
   w2 = efl_add(efl_ui_widget_realized_class_get(), w1);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(w2), w1);
   w3 = efl_add(efl_ui_widget_realized_class_get(), w1);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(w3), w1);

   //check if widget_parent is working correctly
   efl_ui_widget_sub_object_add(w2, w3);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(w3), w2);

   efl_del(win);
}
EFL_END_TEST


EFL_START_TEST(efl_ui_test_widget_disabled_parent)
{
   Efl_Ui_Win *win, *w1, *w2, *t;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Hello World"));
   //first check the initial state
   w1 = efl_add(efl_ui_widget_realized_class_get(), win);
   efl_ui_widget_disabled_set(w1, EINA_TRUE);
   w2 = efl_add(efl_ui_widget_realized_class_get(), win);
   efl_ui_widget_disabled_set(w2, EINA_FALSE);
   ck_assert_int_eq(efl_ui_widget_disabled_get(w1), EINA_TRUE);
   ck_assert_int_eq(efl_ui_widget_disabled_get(w2), EINA_FALSE);

   //check if disalbed gets correctly setted on reparent
   t = efl_add(efl_ui_widget_realized_class_get(), win);
   efl_ui_widget_sub_object_add(w2, t);
   ck_assert_int_eq(efl_ui_widget_disabled_get(t), EINA_FALSE);
   efl_ui_widget_sub_object_add(w1, t);
   ck_assert_int_eq(efl_ui_widget_disabled_get(t), EINA_TRUE);
   efl_del(t);

   t = efl_add(efl_ui_widget_realized_class_get(), w2);
   ck_assert_int_eq(efl_ui_widget_disabled_get(t), EINA_FALSE);
   efl_del(t);

   t = efl_add(efl_ui_widget_realized_class_get(), w1);
   ck_assert_int_eq(efl_ui_widget_disabled_get(t), EINA_TRUE);
   efl_del(t);


   efl_del(win);
}
EFL_END_TEST

static int tree_abort;
static int tree_abort_level;

static void
_shutdown(void)
{
   eina_log_abort_on_critical_set(tree_abort);
   eina_log_abort_on_critical_level_set(tree_abort_level);
}

static void
_setup(void)
{
   tree_abort = eina_log_abort_on_critical_get();
   tree_abort_level = eina_log_abort_on_critical_level_get();
   eina_log_abort_on_critical_level_set(2);
   eina_log_abort_on_critical_set(1);
}

void efl_ui_test_widget(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _shutdown);
   tcase_add_test(tc, efl_ui_test_widget_parent_iterator);
   tcase_add_test(tc, efl_ui_test_widget_widget_iterator);
   tcase_add_test(tc, efl_ui_test_widget_widget_sub_iterator);
   tcase_add_test(tc, efl_ui_test_widget_iterator);
   tcase_add_test(tc, efl_ui_test_widget_sub_iterator);
   tcase_add_test(tc, efl_ui_test_widget_sub_object_add_del);
   tcase_add_test(tc, efl_ui_test_widget_sub_object_theme_sync);
   tcase_add_test(tc, efl_ui_test_widget_parent_relation);
   tcase_add_test(tc, efl_ui_test_widget_disabled_parent);
}
