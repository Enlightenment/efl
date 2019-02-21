#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Clock value is changed\n");
}

static Evas_Object *
_create_indicator(void)
{
   const char *indi_name;

   Eo *win = efl_add_ref(EFL_UI_WIN_SOCKET_CLASS, NULL,
		     efl_text_set(efl_added, "indicator"),
		     efl_ui_win_autodel_set(efl_added, EINA_TRUE),
		     efl_ui_win_alpha_set(efl_added, EINA_TRUE));

   if (!win)
     {
        printf("fail to create a portrait indicator window\n");
        return NULL;
     }

   indi_name = "elm_indicator_portrait";

   if (!efl_ui_win_socket_listen(win, indi_name, 0, EINA_FALSE))
     {
        printf("failed to listen portrait window socket.\n");
        efl_del(win);
        return NULL;
     }

   Eo *bx = efl_add(EFL_UI_BOX_CLASS, win,
                    efl_content_set(win, efl_added));

   efl_add(EFL_UI_CLOCK_CLASS, bx,
           efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
           efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
           efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_HOUR, EINA_FALSE),
           efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_MINUTE, EINA_FALSE),
           efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_AMPM, EINA_FALSE),
           efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_SECOND, EINA_FALSE),
           efl_ui_clock_field_visible_set(efl_added, EFL_UI_CLOCK_TYPE_DAY, EINA_FALSE),
           efl_ui_clock_pause_set(efl_added, EINA_TRUE),
           efl_event_callback_add(efl_added, EFL_UI_CLOCK_EVENT_CHANGED, _changed_cb, NULL),
           efl_pack(bx, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 30));
   return win;
}

static void
_off_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
	efl_ui_win_indicator_mode_set(data, EFL_UI_WIN_INDICATOR_OFF);
}

static void
_opaque_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
	efl_ui_win_indicator_mode_set(data, EFL_UI_WIN_INDICATOR_BG_OPAQUE);
}

static void
_transparent_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
	efl_ui_win_indicator_mode_set(data, EFL_UI_WIN_INDICATOR_BG_TRANSPARENT);
}

static void
_hidden_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
	efl_ui_win_indicator_mode_set(data, EFL_UI_WIN_INDICATOR_HIDDEN);
}

static void
_win_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
	efl_del(data);
}

void
test_win_indicator(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *indicator;

   indicator = _create_indicator();

   // FIXME: Resizing window should no cause sizing issues!
   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Win.Indicator"),
                 efl_gfx_hint_size_max_set(efl_added, EINA_SIZE2D(300, -1)),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_event_callback_add(win, EFL_EVENT_DEL, _win_del, indicator);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 360));

   Eo *bx = efl_add(EFL_UI_BOX_CLASS, win,
                    efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(300, 0)),
                    efl_content_set(win, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Indicator Off"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _off_clicked, win),
           efl_pack(bx, efl_added));
   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Bg Opaque"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _opaque_clicked, win),
           efl_pack(bx, efl_added));
   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Bg Transparent"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _transparent_clicked, win),
           efl_pack(bx, efl_added));
   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Hidden"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _hidden_clicked, win),
           efl_pack(bx, efl_added));
}
