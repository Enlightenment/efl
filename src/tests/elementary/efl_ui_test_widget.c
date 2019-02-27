//#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_widget.h"
#include <Efl_Ui.h>
#include "efl_ui_suite.h"

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
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
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

void efl_ui_test_widget(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_widget_parent_iterator);
   tcase_add_test(tc, efl_ui_test_widget_widget_iterator);
   tcase_add_test(tc, efl_ui_test_widget_widget_sub_iterator);
   tcase_add_test(tc, efl_ui_test_widget_iterator);
   tcase_add_test(tc, efl_ui_test_widget_sub_iterator);
}
