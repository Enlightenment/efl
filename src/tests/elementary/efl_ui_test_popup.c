#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "efl_ui_suite.h"

#define WIN_SIZE 500
#define POPUP_SIZE 160
#define POPUP_SIZE_EXPAND POPUP_SIZE * 2

#define POPUP_ANCHOR_BUTTON_SIZE_VERT 35
/* top row */
#define POPUP_ANCHOR_BUTTON_SIZE_HORIZ1 70
/* middle and bottom row */
#define POPUP_ANCHOR_BUTTON_SIZE_HORIZ2 100

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
   Eo *popup = *popup_ret = _popup_alert_setup(EFL_UI_ALERT_POPUP_CLASS);
   Eo *layout = *layout_ret = _popup_layout_create(popup);

   /* should be 200x200 */
   layout_sz_min = efl_gfx_hint_size_combined_min_get(layout);
   efl_gfx_entity_size_set(layout, layout_sz_min);

   efl_ui_widget_scrollable_content_set(popup, layout);
   return layout_sz_min;
}

EFL_START_TEST(efl_ui_test_popup_basic_align)
{
   Eo *win, *popup;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   popup = efl_add(EFL_UI_POPUP_CLASS, win);
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

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
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

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
     (void*)event_callback_single_call_int_data, &called);
   efl_event_callback_add(popup, EFL_UI_POPUP_EVENT_TIMEOUT, event_callback_that_quits_the_main_loop_when_called, NULL);
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

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
     (void*)event_callback_single_call_int_data, &called);
   efl_ui_popup_part_backwall_repeat_events_set(efl_part(popup, "backwall"), EINA_TRUE);
   click_object(repeat_test_btn);

   efl_ui_popup_closing_timeout_set(popup, 0.1);
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

   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
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
   efl_gfx_hint_size_max_set(popup, EINA_SIZE2D(POPUP_SIZE_EXPAND, -1));
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
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
   efl_gfx_hint_size_max_set(popup, EINA_SIZE2D(-1, POPUP_SIZE_EXPAND));
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
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
   efl_gfx_hint_size_max_set(popup, EINA_SIZE2D(POPUP_SIZE_EXPAND, POPUP_SIZE_EXPAND));
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
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
   efl_gfx_hint_size_max_set(popup, EINA_SIZE2D(POPUP_SIZE / 2, POPUP_SIZE / 2));
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));
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
   Eo *popup = _popup_alert_setup(EFL_UI_ALERT_POPUP_CLASS);
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
   efl_gfx_hint_size_min_set(popup, EINA_SIZE2D(POPUP_SIZE, POPUP_SIZE));

   for (i = 0; i < num_tests; i++)
     {
        unsigned int j;
        Eina_Size2D popup_sz_min, popup_sz, label_sz_min, label_sz, scroller_sz;
        Eina_Strbuf *buf = eina_strbuf_new();
        Eo *label, *scroller;

        for (j = 0; j < string_counts[i]; j++)
          eina_strbuf_append(buf, test_string);

        efl_ui_widget_scrollable_text_set(popup, eina_strbuf_string_get(buf));
        efl_gfx_hint_size_max_set(popup, test_expands[i]);
        efl_canvas_group_calculate(popup);

        /* get internal label object: VERY illegal */
        scroller = efl_content_get(efl_part(efl_super(popup, EFL_UI_ALERT_POPUP_CLASS), "efl.content"));
        label = efl_content_get(scroller);

        /* label should never be larger than scroller horizontally
         * ...but we give it an extra pixel because that's how it's always been
         */
        label_sz = efl_gfx_entity_size_get(label);
        scroller_sz = efl_gfx_entity_size_get(scroller);
        ck_assert_int_le(label_sz.w, scroller_sz.w + 1);

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

static void
_align_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Popup_Align align = (uintptr_t)data;
   Eo *popup  = efl_ui_widget_parent_get(efl_ui_widget_parent_get(ev->object));
   efl_ui_popup_align_set(popup, align);
}

typedef enum
{
   ALIGN_CENTER = 0,
   ALIGN_TOP,
   ALIGN_BOTTOM,
   ALIGN_LEFT,
   ALIGN_RIGHT,
   ALIGN_POSITION,
   ALIGN_RESIZE,
} Align;

static inline Align
popup_prio_to_align(Efl_Ui_Popup_Align pa)
{
   switch (pa)
     {
      case EFL_UI_POPUP_ALIGN_CENTER:
        return ALIGN_CENTER;
      case EFL_UI_POPUP_ALIGN_LEFT:
        return ALIGN_LEFT;
      case EFL_UI_POPUP_ALIGN_RIGHT:
        return ALIGN_RIGHT;
      case EFL_UI_POPUP_ALIGN_TOP:
        return ALIGN_TOP;
      case EFL_UI_POPUP_ALIGN_BOTTOM:
        return ALIGN_BOTTOM;
      default: break;
     }
   printf("FAIL\n");
   abort();
}

static Eina_Bool
can_move_x(Align align)
{
   if (align == ALIGN_LEFT) return EINA_FALSE;
   if (align == ALIGN_RIGHT) return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
can_move_y(Align align)
{
   if (align == ALIGN_TOP) return EINA_FALSE;
   if (align == ALIGN_BOTTOM) return EINA_FALSE;
   return EINA_TRUE;
}

#undef COORDS_INSIDE
#define COORDS_INSIDE(x, y, xx, yy, ww, hh) \
  (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && ((x) >= (xx)) && ((y) >= (yy)))
#define CONTAINS(x, y, w, h, xx, yy, ww, hh) \
  (((xx) >= (x)) && (((x) + (w)) >= ((xx) + (ww))) && ((yy) >= (y)) && (((y) + (h)) >= ((yy) + (hh))))
#define INTERSECTS(x, y, w, h, xx, yy, ww, hh) \
  (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && (((x) + (w)) > (xx)) && (((y) + (h)) > (yy)))

#define OFF_LEFT(x) \
  ((x) < 0)
#define OFF_TOP(y) \
  ((y) < 0)
#define OFF_RIGHT(x, w, ww) \
  ((x) + (w) > (ww))
#define OFF_BOTTOM(y, h, hh) \
  ((y) + (h) > (hh))

static void
verify_anchor(Eo *popup, Eo **align_buttons, Align align, Eina_Size2D *popup_sz)
{
   Eo *anchor = efl_ui_popup_anchor_get(popup);
   Eo *win = efl_provider_find(popup, EFL_UI_WIN_CLASS);
   Eina_Rect anchor_geom;
   Efl_Ui_Popup_Align cur_prio;

   Eina_Rect win_geom = efl_gfx_entity_geometry_get(win);

   if (anchor)
     anchor_geom = efl_gfx_entity_geometry_get(anchor);
   else
     anchor_geom = win_geom;

   Evas_Coord x = 0, y = 0;

   /* click the button to trigger changing the align on the popup
    * this could be done manually, but we're trying to copy the elm_test case
    */
   click_object(align_buttons[align]);
   /* spin main loop to ensure edje signal and event propagation */
   ecore_main_loop_iterate();
   /* manually calc popup to verify */
   efl_canvas_group_calculate(popup);

   cur_prio = efl_ui_popup_align_get(popup);

   Efl_Ui_Popup_Align prio[] =
   {
      cur_prio,
      EFL_UI_POPUP_ALIGN_TOP,
      EFL_UI_POPUP_ALIGN_BOTTOM,
      EFL_UI_POPUP_ALIGN_LEFT,
      EFL_UI_POPUP_ALIGN_RIGHT,
      EFL_UI_POPUP_ALIGN_CENTER
   };
   unsigned int i;

   /* popup anchor (currently) attempts to apply an align using the existing anchor
    * constraint checking fails in two cases:
    * - popup is outside the window
    * - popup is not exactly aligned to anchor
    *
    * if either of these cases occur, the next priority in the priority list will be used
    * UNLESS the align is CENTER, in which case the popup may be adjusted
    */
   for (i = 0; i < EINA_C_ARRAY_LENGTH(prio); i++)
     {
        if (i && (prio[i] == cur_prio)) continue;
        Align cur_align = popup_prio_to_align(prio[i]);
        switch (cur_align)
          {
           /* allow x/y adjust, permit covering anchor */
           case ALIGN_CENTER:
             x = anchor_geom.x + ((anchor_geom.w - popup_sz->w) / 2);
             y = anchor_geom.y + ((anchor_geom.h - popup_sz->h) / 2);
             break;

           /* allow x adjust, do not permit covering anchor */
           case ALIGN_TOP:
             x = anchor_geom.x + ((anchor_geom.w - popup_sz->w) / 2);
             y = anchor_geom.y - popup_sz->h;
             break;
           /* allow x adjust, do not permit covering anchor */
           case ALIGN_BOTTOM:
             x = anchor_geom.x + ((anchor_geom.w - popup_sz->w) / 2);
             y = anchor_geom.y + anchor_geom.h;
             break;
           /* allow y adjust, do not permit covering anchor */
           case ALIGN_LEFT:
             x = anchor_geom.x - popup_sz->w;
             y = anchor_geom.y + ((anchor_geom.h - popup_sz->h) / 2);
             break;
           /* allow y adjust, do not permit covering anchor */
           case ALIGN_RIGHT:
             x = anchor_geom.x + anchor_geom.w;
             y = anchor_geom.y + ((anchor_geom.h - popup_sz->h) / 2);
             break;
           default: break;
          }
        if ((!anchor) || can_move_x(cur_align))
          {
             /* verify that popups not anchored to the win object are inside the window;
              * clamp to window if necessary
              */
             if (!CONTAINS(0, 0, win_geom.w, win_geom.h, x, y, popup_sz->w, popup_sz->h))
               { if (OFF_LEFT(x)) x = 0; }
             if (!CONTAINS(0, 0, win_geom.w, win_geom.h, x, y, popup_sz->w, popup_sz->h))
               { if (OFF_RIGHT(x, popup_sz->w, win_geom.w)) x = win_geom.w - popup_sz->w; }
          }
        if ((!anchor) || can_move_y(cur_align))
          {
             /* verify that popups not anchored to the win object are inside the window;
              * clamp to window if necessary
              */
             if (!CONTAINS(0, 0, win_geom.w, win_geom.h, x, y, popup_sz->w, popup_sz->h))
               { if (OFF_TOP(y)) y = 0; }
             if (!CONTAINS(0, 0, win_geom.w, win_geom.h, x, y, popup_sz->w, popup_sz->h))
               { if (OFF_BOTTOM(y, popup_sz->h, win_geom.h)) y = win_geom.h - popup_sz->h; }
          }
        /* if the popup is not allowed to be clamped on a given axis, try a different align */
        if (!CONTAINS(0, 0, win_geom.w, win_geom.h, x, y, popup_sz->w, popup_sz->h)) continue;
        /* no further clamping for center aligns since this permits popups to be placed over the anchor */
        if (align == ALIGN_CENTER) break;
        /* no further clamping if the anchor is the win object since the popup will always be placed in the window */
        if (!anchor) break;
        /* verify clamping on X-axis */
        if (can_move_x(cur_align))
          {
             if ((x > anchor_geom.x + anchor_geom.w) || (x + popup_sz->w < anchor_geom.x)) continue;
          }
        /* verify clamping on Y-axis */
        if (can_move_y(cur_align))
          {
             if ((y > anchor_geom.y + anchor_geom.h) || (y + popup_sz->h < anchor_geom.y)) continue;
          }
        break;
     }

   Eina_Position2D popup_pos = efl_gfx_entity_position_get(popup);
   ck_assert_int_eq(x, popup_pos.x);
   ck_assert_int_eq(y, popup_pos.y);
}

EFL_START_TEST(efl_ui_test_popup_text_anchor)
{
   Eo *win, *popup, *layout;
   char buf[PATH_MAX];
   int i, num_anchors = 6;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(WIN_SIZE, WIN_SIZE));

   layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   ck_assert(efl_file_simple_load(layout, buf, "efl_ui_popup_anchor_layout"));
   efl_content_set(win, layout);

   popup = efl_add(EFL_UI_POPUP_CLASS, win);
   efl_ui_popup_part_backwall_repeat_events_set(efl_part(popup, "backwall"), EINA_TRUE);
   //Default align priority order is top, left, right, bottom, center.
   efl_ui_popup_align_priority_set(popup, EFL_UI_POPUP_ALIGN_TOP,
                                          EFL_UI_POPUP_ALIGN_BOTTOM,
                                          EFL_UI_POPUP_ALIGN_LEFT,
                                          EFL_UI_POPUP_ALIGN_RIGHT,
                                          EFL_UI_POPUP_ALIGN_CENTER);

   Eo *btn, *bganchors[num_anchors], *aligns[7];
   for (i = 0; i < num_anchors; i++)
     {
        btn = bganchors[i] = efl_add(EFL_UI_BUTTON_CLASS, win);
        efl_text_set(btn, "anchor");
        //efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _anchor_set_cb, popup);

        snprintf(buf, sizeof(buf), "anchor%d", i+1);
        efl_content_set(efl_part(layout, buf), btn);
     }

   btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(btn, "anchor none");
   //efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _anchor_unset_cb, popup);
   efl_content_set(efl_part(layout, "anchor_none"), btn);

   Eo *table = efl_add(EFL_UI_TABLE_CLASS, popup);
   efl_gfx_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   i = 0;

   aligns[i++] = btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Center Align");
   efl_gfx_hint_size_min_set(btn, EINA_SIZE2D(POPUP_ANCHOR_BUTTON_SIZE_HORIZ1, POPUP_ANCHOR_BUTTON_SIZE_VERT));
   efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _align_cb, (intptr_t*)EFL_UI_POPUP_ALIGN_CENTER);
   efl_pack_table(table, btn, 0, 0, 2, 1);

   aligns[i++] = btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Top Align");
   efl_gfx_hint_size_min_set(btn, EINA_SIZE2D(POPUP_ANCHOR_BUTTON_SIZE_HORIZ1, POPUP_ANCHOR_BUTTON_SIZE_VERT));
   efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _align_cb, (intptr_t*)EFL_UI_POPUP_ALIGN_TOP);
   efl_pack_table(table, btn, 2, 0, 2, 1);

   aligns[i++] = btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Bottom Align");
   efl_gfx_hint_size_min_set(btn, EINA_SIZE2D(POPUP_ANCHOR_BUTTON_SIZE_HORIZ1, POPUP_ANCHOR_BUTTON_SIZE_VERT));
   efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _align_cb, (intptr_t*)EFL_UI_POPUP_ALIGN_BOTTOM);
   efl_pack_table(table, btn, 4, 0, 2, 1);

   aligns[i++] = btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Left Align");
   efl_gfx_hint_size_min_set(btn, EINA_SIZE2D(POPUP_ANCHOR_BUTTON_SIZE_HORIZ2, POPUP_ANCHOR_BUTTON_SIZE_VERT));
   efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _align_cb, (intptr_t*)EFL_UI_POPUP_ALIGN_LEFT);
   efl_pack_table(table, btn, 0, 1, 3, 1);

   aligns[i++] = btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Right Align");
   efl_gfx_hint_size_min_set(btn, EINA_SIZE2D(POPUP_ANCHOR_BUTTON_SIZE_HORIZ2, POPUP_ANCHOR_BUTTON_SIZE_VERT));
   efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _align_cb, (intptr_t*)EFL_UI_POPUP_ALIGN_RIGHT);
   efl_pack_table(table, btn, 3, 1, 3, 1);

   aligns[i++] = btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Position Set");
   efl_gfx_hint_size_min_set(btn, EINA_SIZE2D(POPUP_ANCHOR_BUTTON_SIZE_HORIZ2, POPUP_ANCHOR_BUTTON_SIZE_VERT));
   /* this is done manually */
   //efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _position_set_cb, popup);
   efl_pack_table(table, btn, 0, 2, 3, 1);

   aligns[i++] = btn = efl_add(EFL_UI_BUTTON_CLASS, popup);
   efl_text_set(btn, "Resize");
   efl_gfx_hint_size_min_set(btn, EINA_SIZE2D(POPUP_ANCHOR_BUTTON_SIZE_HORIZ2, POPUP_ANCHOR_BUTTON_SIZE_VERT));
   /* this is done manually */
   //efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, _popup_resize_cb, popup);
   efl_pack_table(table, btn, 3, 2, 3, 1);

   efl_content_set(popup, table);

   get_me_to_those_events(popup);

   Eina_Size2D popup_sz = efl_gfx_entity_size_get(popup);
   /* popup should be at least the size of the table, which is the size of the buttons */
   ck_assert_int_ge(popup_sz.w, 2 * POPUP_ANCHOR_BUTTON_SIZE_HORIZ2);
   ck_assert_int_ge(popup_sz.h, 3 * POPUP_ANCHOR_BUTTON_SIZE_VERT);

   for (unsigned int sizes = 0; sizes < 2; sizes++)
     {
        if (sizes > 0)
          {
             /* increase size of buttons to make popup larger and test different constrain codepaths */
             for (unsigned int j = 0; j < ALIGN_RESIZE; j++)
               {
                  Eina_Size2D min_sz = efl_gfx_hint_size_min_get(aligns[j]);
                  min_sz.w += min_sz.w / 2;
                  min_sz.h += min_sz.h / 2;
                  efl_gfx_hint_size_min_set(aligns[j], min_sz);
               }
             efl_canvas_group_calculate(table);
             efl_canvas_group_calculate(popup);

             popup_sz = efl_gfx_entity_size_get(popup);
             /* popup should be at least the size of the table, which is the size of the buttons */
             ck_assert_int_ge(popup_sz.w, 2 * (POPUP_ANCHOR_BUTTON_SIZE_HORIZ2 + sizes * (POPUP_ANCHOR_BUTTON_SIZE_HORIZ2 / 2)));
             ck_assert_int_ge(popup_sz.h, 3 * (POPUP_ANCHOR_BUTTON_SIZE_VERT + sizes * (POPUP_ANCHOR_BUTTON_SIZE_VERT / 2)));
          }
        for (i = -1; i < num_anchors; i++)
          {
             if (i >= 0) efl_ui_popup_anchor_set(popup, bganchors[i]);
             /* -1 is anchored to win object */
             else efl_ui_popup_anchor_set(popup, NULL);
             for (unsigned int j = 0; j < ALIGN_RESIZE; j++)
               {
                  verify_anchor(popup, aligns, j, &popup_sz);
               }
          }
     }
   /* this should unset the anchor completely */
   efl_gfx_entity_position_set(popup, EINA_POSITION2D(0, 0));
   efl_canvas_group_calculate(popup);
   /* verify that the popup doesn't reuse its previous anchor */
   Eina_Position2D popup_pos = efl_gfx_entity_position_get(popup);
   ck_assert_int_eq(popup_pos.x, 0);
   ck_assert_int_eq(popup_pos.y, 0);
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
   tcase_add_test(tc, efl_ui_test_popup_text_anchor);
}
