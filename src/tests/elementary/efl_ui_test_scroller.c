#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "efl_ui_suite.h"

static void
_startfinish_event_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   int *called = data;
   (*called)++;
   /* start/finish always come in pairs */
   if (*called == 2) ecore_main_loop_quit();
}

EFL_START_TEST(efl_ui_test_scroller_events)
{
   Eo *sc, *sc2, *sc3, *bx, *bx2, *gd, *gd2;
   int i, j, called = 0, called2 = 0, called3 = 0;
   Eo *win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));


   sc = efl_add(EFL_UI_SCROLLER_CLASS, win,
                efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_STARTED, _startfinish_event_cb, &called),
                efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_FINISHED, _startfinish_event_cb, &called),
                efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(500, 500))
                );

   bx = efl_add(EFL_UI_BOX_CLASS, sc,
                efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0),
                efl_gfx_hint_align_set(efl_added, 0.5, 0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                efl_content_set(sc, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, bx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 0)),
           efl_pack(bx, efl_added));

   for (i = 0; i < 3; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx,
                efl_text_set(efl_added, "Vertical"),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                //efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx, efl_added));
      }

   sc2 = efl_add(EFL_UI_SCROLLER_CLASS, bx,
                 efl_ui_scrollable_match_content_set(efl_added, EINA_FALSE, EINA_TRUE),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_STARTED, _startfinish_event_cb, &called2),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_FINISHED, _startfinish_event_cb, &called2),
                 efl_pack(bx, efl_added));

   bx2 = efl_add(EFL_UI_BOX_CLASS, sc2,
                 efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                 efl_content_set(sc2, efl_added));

   for (i = 0; i < 10; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx2,
                efl_text_set(efl_added, "... Horizontal scrolling ..."),
                //efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx2, efl_added));
      }

   for (i = 0; i < 3; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx,
                efl_text_set(efl_added, "Vertical"),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                //efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx, efl_added));
      }

   gd = efl_add(EFL_UI_TABLE_CLASS, bx,
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                efl_gfx_hint_align_set(efl_added, 0.5, 0),
                efl_pack(bx, efl_added));

   efl_add(EFL_CANVAS_RECTANGLE_CLASS, win,
           efl_gfx_color_set(efl_added, 0, 0, 0, 0),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(200, 120)),
           efl_pack_table(gd, efl_added, 0, 0, 1, 1));

   sc3 = efl_add(EFL_UI_SCROLLER_CLASS, win,
                 efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                 efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_STARTED, _startfinish_event_cb, &called3),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_SCROLL_FINISHED, _startfinish_event_cb, &called3),
                 efl_pack_table(gd, efl_added, 0, 0, 1, 1));

   gd2 = efl_add(EFL_UI_TABLE_CLASS, sc3,
                 efl_content_set(sc3, efl_added));

   for (j = 0; j < 4; j++)
     {
        for (i = 0; i < 4; i++)
          {
             efl_add(EFL_UI_BUTTON_CLASS, win,
                     efl_text_set(efl_added, "Both"),
                     //efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _bt_clicked, NULL),
                     efl_pack_table(gd2, efl_added, i, j, 1, 1));
          }
     }

   for (i = 0; i < 10; i++)
      {
        efl_add(EFL_UI_BUTTON_CLASS, bx,
                efl_text_set(efl_added, "Vertical"),
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                //efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _bt_clicked, NULL),
                efl_pack(bx, efl_added));
      }
    get_me_to_those_events(sc);
    /* this should only trigger the inner scroller */
    wheel_object(sc3, 0, 1);
    ecore_main_loop_begin();
    ck_assert_int_eq(called3, 2);
    ck_assert_int_eq(called2, 0);
    ck_assert_int_eq(called, 0);
    called3 = 0;

    /* this should only trigger the horizontal scroller */
    wheel_object(sc2, 1, 1);
    ecore_main_loop_begin();
    ck_assert_int_eq(called3, 0);
    ck_assert_int_eq(called2, 2);
    ck_assert_int_eq(called, 0);
    called2 = 0;

    /* this should only trigger the outer scroller */
    wheel_object_at(sc, 250, 400, 0, 1);
    ecore_main_loop_begin();
    ck_assert_int_eq(called3, 0);
    ck_assert_int_eq(called2, 0);
    ck_assert_int_eq(called, 2);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_scroller_scrollbar)
{
   Eo *sc;

   Eo *win = win_add();
   Eina_Bool hbar_visible = EINA_FALSE, vbar_visible = EINA_FALSE;
   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));

   sc = efl_add(EFL_UI_SCROLLER_CLASS, win,
                efl_ui_scrollbar_bar_mode_set(efl_added, EFL_UI_SCROLLBAR_MODE_AUTO, EFL_UI_SCROLLBAR_MODE_AUTO),
                efl_content_set(win, efl_added));

   /*Scrollbar auto mode test.*/
   efl_loop_iterate(efl_main_loop_get());

   efl_ui_scrollbar_bar_visibility_get(sc, &hbar_visible, &vbar_visible);
   ck_assert(hbar_visible == EINA_FALSE);
   ck_assert(vbar_visible == EINA_FALSE);

   /*Scrollbar auto mode test.*/
   efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(sc),
           efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(5000, 5000)),
           efl_content_set(sc, efl_added));

   efl_loop_iterate(efl_main_loop_get());

   efl_ui_scrollbar_bar_visibility_get(sc, &hbar_visible, &vbar_visible);
   ck_assert(hbar_visible == EINA_TRUE);
   ck_assert(vbar_visible == EINA_TRUE);

   /*Scrollbar off mode test.*/
   efl_ui_scrollbar_bar_mode_set(sc, EFL_UI_SCROLLBAR_MODE_OFF, EFL_UI_SCROLLBAR_MODE_OFF);

   efl_loop_iterate(efl_main_loop_get());

   efl_ui_scrollbar_bar_visibility_get(sc, &hbar_visible, &vbar_visible);
   ck_assert(hbar_visible == EINA_FALSE);
   ck_assert(vbar_visible == EINA_FALSE);

   /*Scrollbar on mode test.*/
   efl_ui_scrollbar_bar_mode_set(sc, EFL_UI_SCROLLBAR_MODE_ON, EFL_UI_SCROLLBAR_MODE_ON);

   efl_loop_iterate(efl_main_loop_get());

   efl_ui_scrollbar_bar_visibility_get(sc, &hbar_visible, &vbar_visible);
   ck_assert(hbar_visible == EINA_TRUE);
   ck_assert(vbar_visible == EINA_TRUE);
}
EFL_END_TEST

void efl_ui_test_scroller(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_scroller_events);
   tcase_add_test(tc, efl_ui_test_scroller_scrollbar);
}
