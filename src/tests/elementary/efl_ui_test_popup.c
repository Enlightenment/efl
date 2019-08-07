#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "efl_ui_suite.h"

#define WIN_SIZE 500
#define POPUP_SIZE 160
#define POPUP_SIZE_EXPAND POPUP_SIZE * 2

static Eo *
_popup_layout_create(Eo *popup)
{
   char buf[PATH_MAX];
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   ck_assert(efl_file_simple_load(layout, buf, "efl_ui_popup_scroll_content"));
   efl_canvas_group_calculate(layout);
   return layout;
}

static Eo *
_popup_alert_setup(const Efl_Class *klass)
{
   Eo *win, *popup;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   popup = efl_add(klass, win);
   efl_text_set(efl_part(popup, "title"), "title");
   efl_ui_alert_popup_button_set(popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);
   return popup;
}

static Eina_Size2D
_popup_scroll_alert_setup(Eo **popup_ret, Eo **layout_ret)
{
   Eina_Size2D layout_sz_min;
   Eo *popup = *popup_ret = _popup_alert_setup(EFL_UI_SCROLL_ALERT_POPUP_CLASS);
   Eo *layout = *layout_ret = _popup_layout_create(popup);

   /* should be 200x200 */
   layout_sz_min = efl_gfx_hint_size_combined_min_get(layout);
   efl_gfx_entity_size_set(layout, layout_sz_min);

   efl_content_set(popup, layout);
   return layout_sz_min;
}

EFL_START_TEST(efl_ui_test_popup_basic_align)
{
   Eo *win, *popup;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   popup = efl_add(EFL_UI_POPUP_CLASS, win);
   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Efl.Ui.Popup");

   efl_content_set(popup, btn);


   get_me_to_those_events(popup);

   {
      Eina_Position2D pos = efl_gfx_entity_position_get(popup);
      Eina_Size2D sz = efl_gfx_entity_size_get(popup);

      /* verify centered */
      ck_assert_int_eq(pos.x, WIN_SIZE / 2 - sz.w / 2);
      ck_assert_int_eq(pos.y, WIN_SIZE / 2 - sz.h / 2);
   }

   efl_ui_popup_align_set(popup, EFL_UI_POPUP_ALIGN_LEFT);
   efl_canvas_group_calculate(popup);
   {
      Eina_Position2D pos = efl_gfx_entity_position_get(popup);
      Eina_Size2D sz = efl_gfx_entity_size_get(popup);

      /* verify left */
      ck_assert_int_eq(pos.x, 0);
      ck_assert_int_eq(pos.y, WIN_SIZE / 2 - sz.h / 2);
   }


   efl_ui_popup_align_set(popup, EFL_UI_POPUP_ALIGN_RIGHT);
   efl_canvas_group_calculate(popup);
   {
      Eina_Position2D pos = efl_gfx_entity_position_get(popup);
      Eina_Size2D sz = efl_gfx_entity_size_get(popup);

      /* verify right */
      ck_assert_int_eq(pos.x, WIN_SIZE - sz.w);
      ck_assert_int_eq(pos.y, WIN_SIZE / 2 - sz.h / 2);
   }

   efl_ui_popup_align_set(popup, EFL_UI_POPUP_ALIGN_TOP);
   efl_canvas_group_calculate(popup);
   {
      Eina_Position2D pos = efl_gfx_entity_position_get(popup);
      Eina_Size2D sz = efl_gfx_entity_size_get(popup);

      /* verify top */
      ck_assert_int_eq(pos.x, WIN_SIZE / 2 - sz.w / 2);
      ck_assert_int_eq(pos.y, 0);
   }

   efl_ui_popup_align_set(popup, EFL_UI_POPUP_ALIGN_BOTTOM);
   efl_canvas_group_calculate(popup);
   {
      Eina_Position2D pos = efl_gfx_entity_position_get(popup);
      Eina_Size2D sz = efl_gfx_entity_size_get(popup);

      /* verify bottom */
      ck_assert_int_eq(pos.x, WIN_SIZE / 2 - sz.w / 2);
      ck_assert_int_eq(pos.y, WIN_SIZE - sz.h);
   }

   efl_gfx_entity_position_set(popup, EINA_POSITION2D(0, 0));
   efl_canvas_group_calculate(popup);
   {
      Eina_Position2D pos = efl_gfx_entity_position_get(popup);

      /* verify bottom */
      ck_assert_int_eq(pos.x, 0);
      ck_assert_int_eq(pos.y, 0);
   }
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_popup_basic_sizing)
{
   Eo *win, *popup;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   popup = efl_add(EFL_UI_POPUP_CLASS, win);
   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Efl.Ui.Popup");

   efl_content_set(popup, btn);


   get_me_to_those_events(popup);

   Eina_Size2D sz = efl_gfx_entity_size_get(popup);
   ck_assert_int_eq(sz.w, POPUP_SIZE);
   ck_assert_int_eq(sz.h, POPUP_SIZE);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_popup_events)
{
   Eo *win, *popup;
   int called = 0;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   popup = efl_add(EFL_UI_POPUP_CLASS, win);

   efl_event_callback_add(popup, EFL_UI_POPUP_EVENT_BACKWALL_CLICKED,
     (void*)event_callback_that_is_called_exactly_one_time_and_sets_a_single_int_data_pointer_when_called, &called);
   efl_event_callback_add(popup, EFL_UI_POPUP_EVENT_TIMEOUT, event_callback_that_quits_the_main_loop_when_called, NULL);
   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Efl.Ui.Popup");

   efl_content_set(popup, btn);


   get_me_to_those_events(popup);
   click_object_at(win, 1, 1);
   /* trigger signal process -> events */
   ecore_main_loop_iterate();
   ck_assert_int_eq(called, 1);
   called = 0;

   Eo *repeat_test_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_gfx_entity_position_set(repeat_test_btn, EINA_POSITION2D(0, 0));
   efl_gfx_entity_size_set(repeat_test_btn, EINA_SIZE2D(100, 100));
   efl_text_set(repeat_test_btn, "Repeat Event Test");
   efl_event_callback_add(repeat_test_btn, EFL_INPUT_EVENT_CLICKED,
     (void*)event_callback_that_is_called_exactly_one_time_and_sets_a_single_int_data_pointer_when_called, &called);
   efl_ui_popup_part_backwall_repeat_events_set(efl_part(popup, "backwall"), EINA_TRUE);
   click_object(repeat_test_btn);

   efl_ui_popup_timeout_set(popup, 0.1);
   ecore_main_loop_begin();
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_popup_backwall_img)
{
   Eo *win, *popup;
   char buf[PATH_MAX];

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   popup = efl_add(EFL_UI_POPUP_CLASS, win);

   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", ELM_IMAGE_DATA_DIR);
   ck_assert(efl_file_simple_load(efl_part(popup, "backwall"), buf, NULL));
   ck_assert_str_eq(efl_file_get(efl_part(popup, "backwall")), buf);
}
EFL_END_TEST

static void
_popup_button_click(void *data, const Efl_Event *ev)
{
   Efl_Ui_Alert_Popup_Button_Clicked_Event *event = ev->info;
   int *called = data;

   *called = event->button_type;
}

EFL_START_TEST(efl_ui_test_popup_alert)
{
   Eo *win, *popup;
   Eina_Size2D layout_sz_min;
   int called;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   popup = efl_add(EFL_UI_ALERT_POPUP_CLASS, win);
   efl_text_set(efl_part(popup, "title"), "title");

   efl_gfx_entity_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

   Eo *layout = _popup_layout_create(popup);

   layout_sz_min = efl_gfx_hint_size_combined_min_get(layout);

   efl_content_set(popup, layout);
   efl_ui_alert_popup_button_set(popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_event_callback_add(popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, _popup_button_click, &called);

   get_me_to_those_events(popup);
   {
      /* the layout should currently be the size of its calculated (edje) min size */
      Eina_Size2D layout_sz = efl_gfx_entity_size_get(layout);
      ck_assert_int_eq(layout_sz.w, layout_sz_min.w);
      ck_assert_int_eq(layout_sz.h, layout_sz_min.h);
   }

   Eo *btn_layout = efl_content_get(efl_part(popup, "efl.buttons"));
   /* verify button events work as expected using the layout set above:

      Cancel | Yes | No
    */
   called = -1;
   click_part(btn_layout, "efl.button1");
   ecore_main_loop_iterate();
   ck_assert_int_eq(called, EFL_UI_ALERT_POPUP_BUTTON_USER);

   called = -1;
   click_part(btn_layout, "efl.button2");
   ecore_main_loop_iterate();
   ck_assert_int_eq(called, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE);

   called = -1;
   click_part(btn_layout, "efl.button3");
   ecore_main_loop_iterate();
   ck_assert_int_eq(called, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_popup_scroll_alert)
{
   Eo *popup, *layout;
   Eina_Size2D popup_sz_min;
   Eina_Size2D layout_sz_min = _popup_scroll_alert_setup(&popup, &layout);

   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
   get_me_to_those_events(popup);

   /* base popup size without content */
   popup_sz_min = efl_gfx_hint_size_combined_min_get(popup);
   {
      /* the layout should currently be the size of its calculated (edje) min size */
      Eina_Size2D layout_sz = efl_gfx_entity_size_get(layout);
      ck_assert_int_eq(layout_sz.w, layout_sz_min.w);
      ck_assert_int_eq(layout_sz.h, layout_sz_min.h);
   }

   {
      /* the popup should currently be the specified size...
       * except that it's not because popup_size_set is just a fucking wrapper for
       * efl_gfx_entity_size_set
       */
      Eina_Size2D popup_sz = efl_gfx_entity_size_get(popup);
      //ck_assert_int_eq(popup_sz.w, POPUP_SIZE);
      //ck_assert_int_eq(popup_sz.h, POPUP_SIZE);
      ck_assert_int_eq(popup_sz.w, MAX(POPUP_SIZE, popup_sz_min.w));
      ck_assert_int_eq(popup_sz.h, MAX(POPUP_SIZE, popup_sz_min.h));
   }
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_popup_scroll_alert_expand)
{
   Eo *popup, *layout;
   Eina_Size2D popup_sz_min, popup_sz_min1, popup_sz1;
   Eina_Size2D layout_sz_min = _popup_scroll_alert_setup(&popup, &layout);

   get_me_to_those_events(popup);

   /* base popup size without content */
   popup_sz_min = popup_sz_min1 = efl_gfx_hint_size_combined_min_get(popup);

   /* first test horizontal expand */
   efl_ui_scroll_alert_popup_expandable_set(popup, EINA_SIZE2D(POPUP_SIZE_EXPAND, -1));
   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
   efl_canvas_group_calculate(popup);


   {
      /* the layout should currently be AT LEAST the size of its calculated (edje) min size
       * this popup should expand.
       */
      Eina_Size2D layout_sz = efl_gfx_entity_size_get(layout);
      ck_assert_int_ge(layout_sz.w, layout_sz_min.w);
      ck_assert_int_ge(layout_sz.h, layout_sz_min.h);
   }

   {
      Eina_Size2D popup_sz = popup_sz1 = efl_gfx_entity_size_get(popup);
      /* this popup expands horizontally, so its width should be greater than the
       * layout's min size even though the popup's size was explicitly set to be smaller
       */
      ck_assert_int_gt(popup_sz.w, layout_sz_min.w);
      ck_assert_int_le(popup_sz.w, POPUP_SIZE_EXPAND);
      /* it does not expand vertically, so this should still be small. */
      ck_assert_int_eq(popup_sz.h, MAX(POPUP_SIZE, popup_sz_min.h));
   }

   /* now expand vertically */
   efl_ui_scroll_alert_popup_expandable_set(popup, EINA_SIZE2D(-1, POPUP_SIZE_EXPAND));
   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
   efl_canvas_group_calculate(popup);

   /* base popup size without content */
   popup_sz_min = efl_gfx_hint_size_combined_min_get(popup);
   {
      /* the layout should currently be AT LEAST the size of its calculated (edje) min size
       * this popup should expand.
       */
      Eina_Size2D layout_sz = efl_gfx_entity_size_get(layout);
      ck_assert_int_ge(layout_sz.w, layout_sz_min.w);
      ck_assert_int_ge(layout_sz.h, layout_sz_min.h);
   }

   {
      Eina_Size2D popup_sz = efl_gfx_entity_size_get(popup);
      /* this popup expands vertically, so its height should be greater than the
       * layout's min size even though the popup's size was explicitly set to be smaller
       */
      ck_assert_int_gt(popup_sz.h, layout_sz_min.h);
      ck_assert_int_le(popup_sz.h, POPUP_SIZE_EXPAND);
      /* it does not expand horizontally, so this should still be small. */
      ck_assert_int_eq(popup_sz.w, MAX(POPUP_SIZE, popup_sz_min.w));
   }

   /* now both */
   efl_ui_scroll_alert_popup_expandable_set(popup, EINA_SIZE2D(POPUP_SIZE_EXPAND, POPUP_SIZE_EXPAND));
   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
   efl_canvas_group_calculate(popup);

   /* base popup size without content */
   popup_sz_min = efl_gfx_hint_size_combined_min_get(popup);
   {
      /* the layout should currently be AT LEAST the size of its calculated (edje) min size
       * this popup should expand.
       */
      Eina_Size2D layout_sz = efl_gfx_entity_size_get(layout);
      ck_assert_int_ge(layout_sz.w, layout_sz_min.w);
      ck_assert_int_ge(layout_sz.h, layout_sz_min.h);
   }

   {
      Eina_Size2D popup_sz = efl_gfx_entity_size_get(popup);

      /* this popup expands horizontally, so its width should be greater than the
       * layout's min size even though the popup's size was explicitly set to be smaller
       */
      ck_assert_int_gt(popup_sz.w, layout_sz_min.w);
      ck_assert_int_le(popup_sz.w, POPUP_SIZE_EXPAND);

      /* this popup expands vertically, so its height should be greater than the
       * layout's min size even though the popup's size was explicitly set to be smaller
       */
      ck_assert_int_gt(popup_sz.h, layout_sz_min.h);
      ck_assert_int_le(popup_sz.h, POPUP_SIZE_EXPAND);
   }

   /* now use a confining expand which ignores content min size */
   efl_ui_scroll_alert_popup_expandable_set(popup, EINA_SIZE2D(POPUP_SIZE / 2, POPUP_SIZE / 2));
   efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
   efl_canvas_group_calculate(popup);

   {
      /* the layout should currently be the size of its calculated (edje) min size */
      Eina_Size2D layout_sz = efl_gfx_entity_size_get(layout);
      ck_assert_int_eq(layout_sz.w, layout_sz_min.w);
      ck_assert_int_eq(layout_sz.h, layout_sz_min.h);
   }


   {
      Eina_Size2D popup_sz = efl_gfx_entity_size_get(popup);

      /* this popup is confined, so its size should be the popup's min size without content */
      ck_assert_int_eq(popup_sz.w, popup_sz_min1.w);
      ck_assert_int_eq(popup_sz.h, popup_sz_min1.h);
      /* this popup should NOT be the same height as the popup in the first test */
      ck_assert_int_ne(popup_sz.h, popup_sz1.h);
   }
}
EFL_END_TEST



EFL_START_TEST(efl_ui_test_popup_text_alert)
{
   Eo *popup = _popup_alert_setup(EFL_UI_TEXT_ALERT_POPUP_CLASS);
   char test_string[] = "This is Text Popup";
   unsigned int string_counts[] =
   {
    1, 50, 1, 1, 1, 50, 50, 50, 50, 50, 50, 50, 50, 50
   };
   Eina_Size2D test_expands[] =
   {
      EINA_SIZE2D(-1, -1),
      EINA_SIZE2D(-1, -1),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, POPUP_SIZE_EXPAND),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, -1),
      EINA_SIZE2D(-1, POPUP_SIZE_EXPAND),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, POPUP_SIZE_EXPAND),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, -1),
      EINA_SIZE2D(-1, POPUP_SIZE_EXPAND),
      EINA_SIZE2D(10, 10),
      EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE),
      EINA_SIZE2D(POPUP_SIZE, -1),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, -1),
      EINA_SIZE2D(-1, POPUP_SIZE),
      EINA_SIZE2D(-1, POPUP_SIZE_EXPAND),
   };
   Eina_Size2D expected_sizes[] =
   {
      /* -1 is MAX(POPUP_SIZE, min_size)
       * 0 is min_size
       * POPUP_SIZE_EXPAND on height is MIN(POPUP_SIZE_EXPAND, label_min)
       */
      EINA_SIZE2D(-1, -1),
      EINA_SIZE2D(-1, -1),
      EINA_SIZE2D(-1, 0),
      EINA_SIZE2D(-1, -1),
      EINA_SIZE2D(-1, 0),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, POPUP_SIZE_EXPAND),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, POPUP_SIZE),
      EINA_SIZE2D(-1, 0),
      EINA_SIZE2D(-1, 0),
      EINA_SIZE2D(-1, POPUP_SIZE),
      EINA_SIZE2D(-1, POPUP_SIZE),
      EINA_SIZE2D(POPUP_SIZE_EXPAND, POPUP_SIZE),
      EINA_SIZE2D(-1, POPUP_SIZE),
      EINA_SIZE2D(-1, POPUP_SIZE_EXPAND),
   };
   unsigned int num_tests = EINA_C_ARRAY_LENGTH(string_counts);
   unsigned int i;
   Eina_Size2D popup_sz_min1;

   /* ensure future modifications don't break the test */
   ck_assert_int_eq(EINA_C_ARRAY_LENGTH(string_counts), EINA_C_ARRAY_LENGTH(test_expands));

   get_me_to_those_events(popup);
   popup_sz_min1 = efl_gfx_hint_size_combined_min_get(popup);

   for (i = 0; i < num_tests; i++)
     {
        unsigned int j;
        Eina_Size2D popup_sz_min, popup_sz, label_sz_min;
        Eina_Strbuf *buf = eina_strbuf_new();
        Eo *label;

        for (j = 0; j < string_counts[i]; j++)
          eina_strbuf_append(buf, test_string);

        efl_text_set(popup, eina_strbuf_string_get(buf));
        efl_ui_text_alert_popup_expandable_set(popup, test_expands[i]);
        efl_ui_popup_size_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
        efl_canvas_group_calculate(popup);

        /* get internal label object: VERY illegal */
        label = efl_content_get(efl_content_get(efl_part(efl_super(popup, efl_ui_text_alert_popup_class_get()), "efl.content")));

        label_sz_min = efl_gfx_hint_size_combined_min_get(label);
        popup_sz_min = efl_gfx_hint_size_combined_min_get(popup);
        popup_sz = efl_gfx_entity_size_get(popup);
        if (expected_sizes[i].w == -1)
          ck_assert_int_eq(popup_sz.w, MAX(POPUP_SIZE, popup_sz_min.w));
        else if (expected_sizes[i].w == 0)
          ck_assert_int_eq(popup_sz.w, popup_sz_min.w);
        else
          ck_assert_int_eq(popup_sz.w, expected_sizes[i].w);
        if (expected_sizes[i].h == -1)
          ck_assert_int_eq(popup_sz.h, MAX(POPUP_SIZE, popup_sz_min.h));
        else if (expected_sizes[i].h == 0)
          {
             if ((test_expands[i].h >= label_sz_min.h) && (label_sz_min.h > popup_sz_min1.h))
               ck_assert_int_eq(popup_sz.h, popup_sz_min.h);
             else
               ck_assert_int_eq(popup_sz.h, popup_sz_min1.h);
          }
        else if (expected_sizes[i].h == POPUP_SIZE_EXPAND)
          ck_assert_int_eq(popup_sz.h, MIN(POPUP_SIZE_EXPAND, label_sz_min.h));
        else
          ck_assert_int_eq(popup_sz.h, expected_sizes[i].h);

        eina_strbuf_free(buf);
     }

}
EFL_END_TEST

void efl_ui_test_popup(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_popup_events);
   tcase_add_test(tc, efl_ui_test_popup_basic_sizing);
   tcase_add_test(tc, efl_ui_test_popup_basic_align);
   tcase_add_test(tc, efl_ui_test_popup_backwall_img);
   tcase_add_test(tc, efl_ui_test_popup_alert);
   tcase_add_test(tc, efl_ui_test_popup_scroll_alert);
   tcase_add_test(tc, efl_ui_test_popup_scroll_alert_expand);
   tcase_add_test(tc, efl_ui_test_popup_text_alert);
}
