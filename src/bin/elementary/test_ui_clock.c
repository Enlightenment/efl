#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

/* A simple test, just displaying clock in its default format */

Evas_Object *dt1, *dt2, *dt3, *dt4;

static void
_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Clock value is changed\n");
}

static void
_bt_clicked(void *data EINA_UNUSED, const Efl_Event *ev)
{
   time_t t;
   struct tm new_time;

   t = time(NULL);
   localtime_r(&t, &new_time);

   new_time.tm_year = 85;
   new_time.tm_mon = 9;
   new_time.tm_mday = 26;
   new_time.tm_hour = 9;
   new_time.tm_min = 0;
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_HOUR, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_MINUTE, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_AMPM, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_SECOND, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_DAY, EINA_TRUE);
   efl_ui_clock_time_set(dt1, new_time);

   elm_object_disabled_set(dt1, EINA_TRUE);
   elm_object_disabled_set(ev->object, EINA_TRUE);

   efl_del(dt2);
   efl_del(dt3);
   dt2 = dt3 = NULL;
}

void
test_ui_clock(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Clock"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(360, 240)));

   dt1 = efl_add(EFL_UI_CLOCK_CLASS, bx,
                 efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                 efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_HOUR, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_MINUTE, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_AMPM, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_SECOND, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_DAY, EINA_FALSE),
                 efl_ui_clock_pause_set(efl_added, EINA_TRUE),
                 efl_event_callback_add(efl_added, EFL_UI_CLOCK_EVENT_CHANGED, _changed_cb, NULL),
                 efl_pack(bx, efl_added));

   dt2 = efl_add(EFL_UI_CLOCK_CLASS, bx,
                 efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                 efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_YEAR, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_MONTH, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_DATE, EINA_FALSE),
                 efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_SECOND, EINA_FALSE),
                 efl_ui_clock_pause_set(efl_added, EINA_TRUE),
                 efl_pack(bx, efl_added));
   elm_object_disabled_set(dt2, EINA_TRUE);

   dt3 = efl_add(EFL_UI_CLOCK_CLASS, bx,
                 efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                 efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                 efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Editable Clock:"),
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_TRUE),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(100, 25)),
           efl_pack(bx, efl_added));

   dt4 = efl_add(EFL_UI_CLOCK_CLASS, bx,
                 efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                 efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                 efl_ui_clock_edit_mode_set(efl_added, EINA_TRUE),
                 efl_ui_clock_pause_set(efl_added, EINA_TRUE),
                 efl_pack(bx, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Back to the future..."),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _bt_clicked, NULL),
           efl_pack(bx, efl_added));
}
