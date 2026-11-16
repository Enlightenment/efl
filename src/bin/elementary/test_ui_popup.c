#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_POPUP_BETA
#define EFL_UI_SCROLL_ALERT_POPUP_BETA
#include <Efl_Ui.h>
#include <Elementary.h>

#define POPUP_POINT_MAX 8

typedef struct
{
   Eo *win;
   Eo *panel_win;
   Eo *efl_ui_popup;
   Eo *button;
} efl_ui_popup_data;

static void
_backwall_clicked(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Popup's background is clicked.\n");
}

static void
_timeout_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   p_data->efl_ui_popup = NULL;
   printf("Popup times out and it is deleted.\n");
}

static void
_delete_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (p_data->efl_ui_popup)
     {
        efl_del(p_data->efl_ui_popup);
        p_data->efl_ui_popup = NULL;
        return;
     }
   printf("Efl.Ui.Popup does not exist.\n");
}

static Eo*
_create_popup(efl_ui_popup_data *p_data)
{
   if (!p_data->win) return NULL;
   if (p_data->efl_ui_popup)
     {
        printf("Efl.Ui.Popup is already created.\n");
        efl_gfx_entity_visible_set(p_data->efl_ui_popup, EINA_TRUE);
        return p_data->efl_ui_popup;
     }

   Eo *efl_ui_popup = efl_add(EFL_UI_POPUP_CLASS, p_data->win);

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_EVENT_BACKWALL_CLICKED, _backwall_clicked, NULL);
   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_EVENT_TIMEOUT, _timeout_cb, p_data);
   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, efl_ui_popup);
   efl_text_set(btn, "Efl.Ui.Popup");

   efl_content_set(efl_ui_popup, btn);

   return efl_ui_popup;
}

static void
_create_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   p_data->efl_ui_popup = _create_popup(p_data);
}

static void
_backwall_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   static int k = 0;
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   if (k == 0)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
        efl_file_simple_load(efl_part(p_data->efl_ui_popup, "backwall"), buf, NULL);
        efl_text_set(ev->object, "Backwall UnSet");
     }
   else
     {
        efl_file_simple_load(efl_part(p_data->efl_ui_popup, "backwall"), NULL, NULL);
        efl_text_set(ev->object, "Backwall Set");
     }
   k = !k;
}

static void
_repeat_event_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   if (!efl_ui_popup_part_backwall_repeat_events_get(efl_part(p_data->efl_ui_popup, "backwall")))
     {
        efl_ui_popup_part_backwall_repeat_events_set(efl_part(p_data->efl_ui_popup, "backwall"), EINA_TRUE);
        efl_text_set(ev->object, "Repeat Event UnSet");
     }
   else
     {
        efl_ui_popup_part_backwall_repeat_events_set(efl_part(p_data->efl_ui_popup, "backwall"), EINA_FALSE);
        efl_text_set(ev->object, "Repeat Event Set");
     }
}

static void
_repeat_test_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Repeat Test Button is Clicked.\n");
}

static void
_dummy_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   if (!efl_gfx_entity_visible_get(p_data->button))
     {
        efl_gfx_entity_visible_set(p_data->button, EINA_TRUE);
        efl_text_set(ev->object, "Hide Dummy Button");
     }
   else
     {
        efl_gfx_entity_visible_set(p_data->button, EINA_FALSE);
        efl_text_set(ev->object, "Show Dummy Button");
     }
}

static void
_center_align_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_ui_popup_align_set(p_data->efl_ui_popup, EFL_UI_POPUP_ALIGN_CENTER);
}

static void
_left_align_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_ui_popup_align_set(p_data->efl_ui_popup, EFL_UI_POPUP_ALIGN_LEFT);
}

static void
_right_align_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_ui_popup_align_set(p_data->efl_ui_popup, EFL_UI_POPUP_ALIGN_RIGHT);
}

static void
_top_align_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_ui_popup_align_set(p_data->efl_ui_popup, EFL_UI_POPUP_ALIGN_TOP);
}

static void
_bottom_align_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_ui_popup_align_set(p_data->efl_ui_popup, EFL_UI_POPUP_ALIGN_BOTTOM);
}

static void
_position_set_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_gfx_entity_position_set(p_data->efl_ui_popup, EINA_POSITION2D(0, 0));
}

static void
_timeout_set_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_ui_popup_closing_timeout_set(p_data->efl_ui_popup, 3);
   printf("timemout is set to 3 seconds\n");
}

static void
_win_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   Eo *win = p_data->panel_win;
   p_data->panel_win = NULL;
   /* only free pointer if the other window has already been freed */
   if (!p_data->win)
     free(p_data);
   efl_del(win);
   printf("window is deleted\n");
}

static void
_panel_win_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   Eo *win = p_data->win;
   p_data->win = NULL;
   /* only free pointer if the other window has already been freed */
   if (!p_data->win)
     free(p_data);
   efl_del(win);
   printf("window is deleted\n");
}

void
test_ui_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_ui_popup_data *p_data = (efl_ui_popup_data*)calloc(1, sizeof(efl_ui_popup_data));

   Eo *win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                     efl_text_set(efl_added, "Efl.Ui.Popup"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _win_del, p_data);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));

   Eo *panel_win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                           efl_text_set(efl_added, "Efl.Ui.Popup Panel"),
                           efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_event_callback_add(panel_win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _panel_win_del, p_data);

   efl_gfx_entity_size_set(panel_win, EINA_SIZE2D(300, 500));

   Eo *repeat_test_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_gfx_entity_position_set(repeat_test_btn, EINA_POSITION2D(0, 0));
   efl_gfx_entity_size_set(repeat_test_btn, EINA_SIZE2D(100, 100));
   efl_text_set(repeat_test_btn, "Repeat Event Test");
   efl_event_callback_add(repeat_test_btn, EFL_INPUT_EVENT_CLICKED, _repeat_test_cb, NULL);
   efl_gfx_entity_visible_set(repeat_test_btn, EINA_FALSE);

   p_data->win = win;
   p_data->panel_win = panel_win;
   p_data->button = repeat_test_btn;
   p_data->efl_ui_popup = _create_popup(p_data);

   Eo *create_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(create_btn, "Create Popup");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 0));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _create_cb, p_data);

   Eo *delete_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(delete_btn, "Delete Popup");
   efl_gfx_entity_position_set(delete_btn, EINA_POSITION2D(150, 0));
   efl_gfx_entity_size_set(delete_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(delete_btn, EFL_INPUT_EVENT_CLICKED, _delete_cb, p_data);

   Eo *repeat_event_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(repeat_event_btn, "Repeat Event Set");
   efl_gfx_entity_position_set(repeat_event_btn, EINA_POSITION2D(0, 50));
   efl_gfx_entity_size_set(repeat_event_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(repeat_event_btn, EFL_INPUT_EVENT_CLICKED, _repeat_event_cb, p_data);

   Eo *dummy_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(dummy_btn, "Show Dummy Button");
   efl_gfx_entity_position_set(dummy_btn, EINA_POSITION2D(150, 50));
   efl_gfx_entity_size_set(dummy_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(dummy_btn, EFL_INPUT_EVENT_CLICKED, _dummy_cb, p_data);

   Eo *backwall_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_gfx_entity_position_set(backwall_btn, EINA_POSITION2D(0, 100));
   efl_gfx_entity_size_set(backwall_btn, EINA_SIZE2D(150, 50));
   efl_text_set(backwall_btn, "Backwall Set");
   efl_event_callback_add(backwall_btn, EFL_INPUT_EVENT_CLICKED, _backwall_cb, p_data);

   Eo *timeout_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(timeout_btn, "Timeout Set");
   efl_gfx_entity_position_set(timeout_btn, EINA_POSITION2D(150, 100));
   efl_gfx_entity_size_set(timeout_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(timeout_btn, EFL_INPUT_EVENT_CLICKED, _timeout_set_cb, p_data);

   Eo *center_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(center_btn, "Center Align");
   efl_gfx_entity_position_set(center_btn, EINA_POSITION2D(0, 150));
   efl_gfx_entity_size_set(center_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(center_btn, EFL_INPUT_EVENT_CLICKED, _center_align_cb, p_data);

   Eo *top_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(top_btn, "Top Align");
   efl_gfx_entity_position_set(top_btn, EINA_POSITION2D(150, 150));
   efl_gfx_entity_size_set(top_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(top_btn, EFL_INPUT_EVENT_CLICKED, _top_align_cb, p_data);

   Eo *left_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(left_btn, "Left Align");
   efl_gfx_entity_position_set(left_btn, EINA_POSITION2D(0, 200));
   efl_gfx_entity_size_set(left_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(left_btn, EFL_INPUT_EVENT_CLICKED, _left_align_cb, p_data);

   Eo *right_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(right_btn, "Right Align");
   efl_gfx_entity_position_set(right_btn, EINA_POSITION2D(150, 200));
   efl_gfx_entity_size_set(right_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(right_btn, EFL_INPUT_EVENT_CLICKED, _right_align_cb, p_data);

   Eo *bottom_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(bottom_btn, "Bottom Align");
   efl_gfx_entity_position_set(bottom_btn, EINA_POSITION2D(0, 250));
   efl_gfx_entity_size_set(bottom_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(bottom_btn, EFL_INPUT_EVENT_CLICKED, _bottom_align_cb, p_data);

   Eo *position_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(position_btn, "Position Set (x:0, y:0)");
   efl_gfx_entity_position_set(position_btn, EINA_POSITION2D(150, 250));
   efl_gfx_entity_size_set(position_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(position_btn, EFL_INPUT_EVENT_CLICKED, _position_set_cb, p_data);
}

static void
efl_ui_alert_popup_clicked_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Alert_Popup_Button_Clicked_Event *event = ev->info;

   if (event->button_type == EFL_UI_ALERT_POPUP_BUTTON_POSITIVE)
     printf("Positive Button is clicked\n");
   else if(event->button_type == EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE)
     printf("Negative Button is clicked\n");
   else if(event->button_type == EFL_UI_ALERT_POPUP_BUTTON_USER)
     printf("User Button is clicked\n");

   efl_del(ev->object);
}

void
test_ui_alert_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];

   Eo *win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                     efl_text_set(efl_added, "Efl.Ui.Popup.Alert"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 320));

   Eo *efl_ui_popup = efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   efl_gfx_entity_size_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_simple_load(layout, buf, "efl_ui_popup_scroll_content");

   efl_content_set(efl_ui_popup, layout);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_alert_popup_clicked_cb, NULL);
}

static void
_alert_scroll_case1_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   char buf[PATH_MAX];
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_simple_load(layout, buf, "efl_ui_popup_scroll_content");

   efl_ui_widget_scrollable_content_set(efl_ui_popup, layout);

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_alert_popup_clicked_cb, NULL);
}

static void
_alert_scroll_case2_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   char buf[PATH_MAX];
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_simple_load(layout, buf, "efl_ui_popup_scroll_content");

   efl_ui_widget_scrollable_content_set(efl_ui_popup, layout);

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   //This is beta API for now. So EFL_UI_SCROLL_ALERT_POPUP_BETA should be defined.
   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(320, -1));
   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_alert_popup_clicked_cb, NULL);
}

static void
_alert_scroll_case3_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   char buf[PATH_MAX];
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_simple_load(layout, buf, "efl_ui_popup_scroll_content");

   efl_ui_widget_scrollable_content_set(efl_ui_popup, layout);

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   //This is beta API for now. So EFL_UI_SCROLL_ALERT_POPUP_BETA should be defined.
   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(-1, 320));
   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_alert_popup_clicked_cb, NULL);
}

static void
_alert_scroll_case4_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   char buf[PATH_MAX];
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_simple_load(layout, buf, "efl_ui_popup_scroll_content");

   efl_ui_widget_scrollable_content_set(efl_ui_popup, layout);

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   //This is beta API for now. So EFL_UI_SCROLL_ALERT_POPUP_BETA should be defined.
   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(320, 320));
   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_alert_popup_clicked_cb, NULL);
}

static void
_alert_scroll_case5_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   char buf[PATH_MAX];
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_simple_load(layout, buf, "efl_ui_popup_scroll_content");

   efl_ui_widget_scrollable_content_set(efl_ui_popup, layout);

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   //This is beta API for now. So EFL_UI_SCROLL_ALERT_POPUP_BETA should be defined.
   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(80, 80));
   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_alert_popup_clicked_cb, NULL);
}

void
test_ui_scroll_alert_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                     efl_text_set(efl_added, "Efl.Ui.Popup.Alert.Scroll"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));

   Eo *create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "1. size(160,160), content(200, 200), expand(-1, -1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 0));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_scroll_case1_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "2. size(160,160), content(200, 200), expand(320, -1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 50));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_scroll_case2_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "3. size(160,160), content(200, 200), expand(-1, 320)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 100));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_scroll_case3_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "4. size(160,160), content(200, 200), expand(320, 320)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 150));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_scroll_case4_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "5. size(160,160), content(200, 200), expand(80, 80)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 200));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_scroll_case5_cb, win);
}

static void
efl_ui_text_alert_popup_clicked_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Alert_Popup_Button_Clicked_Event *event = ev->info;

   if (event->button_type == EFL_UI_ALERT_POPUP_BUTTON_POSITIVE)
     efl_ui_widget_scrollable_text_set(ev->object, "Text is changed");
   else
     efl_del(ev->object);
}

static void
create_message(Eina_Strbuf *message_buf, const char *message, int iterate_cnt)
{
   int i;
   for (i = 0; i < iterate_cnt; i++)
      eina_strbuf_append(message_buf, message);
}

static void
_alert_text_case1_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, "This is Text Popup");

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);
}

static void
_alert_text_case2_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 50);

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case3_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));
   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(300, 300));

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, "This is Text Popup");

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);
}

static void
_alert_text_case4_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, "This is Text Popup");

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(300, -1));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);
}

static void
_alert_text_case5_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, "This is Text Popup");

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(-1, 300));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));
   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);
}

static void
_alert_text_case6_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 50);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(300, 300));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case7_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 50);

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(300, -1));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case8_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 50);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(-1, 300));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case9_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 50);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(10, 10));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case10_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 10);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(150, 150));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case11_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 10);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(150, -1));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case12_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 10);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(300, -1));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case13_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 20);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(-1, 150));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

static void
_alert_text_case14_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *efl_ui_popup= efl_add(EFL_UI_ALERT_POPUP_CLASS, win);

   Eina_Strbuf *message_buf = eina_strbuf_new();
   create_message(message_buf, "This is Text Popup ", 50);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");
   efl_ui_widget_scrollable_text_set(efl_ui_popup, eina_strbuf_string_get(message_buf));

   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_POSITIVE, "Yes", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_NEGATIVE, "No", NULL);
   efl_ui_alert_popup_button_set(efl_ui_popup, EFL_UI_ALERT_POPUP_BUTTON_USER, "Cancel", NULL);

   efl_gfx_hint_size_max_set(efl_ui_popup, EINA_SIZE2D(-1, 300));

   efl_gfx_hint_size_min_set(efl_ui_popup, EINA_SIZE2D(200, 200));

   efl_event_callback_add(efl_ui_popup, EFL_UI_ALERT_POPUP_EVENT_BUTTON_CLICKED, efl_ui_text_alert_popup_clicked_cb, NULL);

   eina_strbuf_free(message_buf);
}

void
test_ui_text_alert_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                     efl_text_set(efl_added, "Efl.Ui.Popup.Alert.Text"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 700));

   Eo *create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "1. size(200,200), text(short), expand(-1,-1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 0));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case1_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "2. size(200,200), text(long), expand(-1,-1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 50));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case2_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "3. size(200,200), text(short), expand(300,300)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 100));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case3_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "4. size(200,200), text(short), expand(300,-1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 150));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case4_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "5. size(200,200), text(short), expand(-1,300)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 200));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case5_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "6. size(200,200), text(long), expand(300,300)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 250));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case6_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "7. size(200,200), text(long), expand(300,-1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 300));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case7_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "8. size(200,200), text(long), expand(-1,300)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 350));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case8_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "9. size(200,200), text(long), expand(10, 10)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 400));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case9_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "10. size(200,200), text(long), expand(150,150)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 450));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case10_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "11. size(200,200), text(long), expand(150,-1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 500));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case11_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "12. size(200,200), text(long), expand(300,-1)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 550));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case12_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "13. size(200,200), text(long), expand(-1,150)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 600));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case13_cb, win);

   create_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(create_btn, "14. size(200,200), text(long), expand(-1,300)");
   efl_gfx_entity_position_set(create_btn, EINA_POSITION2D(0, 650));
   efl_gfx_entity_size_set(create_btn, EINA_SIZE2D(500, 50));
   efl_event_callback_add(create_btn, EFL_INPUT_EVENT_CLICKED, _alert_text_case14_cb, win);
}

/* Efl.Ui.Popup.Anchor
 *
 * A popup with an anchor places itself against another widget rather than
 * against the window. Which side of the anchor it lands on is the align.
 *
 * One anchor in the middle, one button per align around it, and the popup is
 * only up while an align is being shown. */

typedef struct
{
   Eo *popup;
   Eo *anchor;
   Eo *label;
} Anchor_Page;

static void
_anchor_page_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
_anchor_align_cb(void *data, const Efl_Event *ev)
{
   Anchor_Page *page = data;
   Efl_Ui_Popup_Align align =
     (Efl_Ui_Popup_Align)(intptr_t) efl_key_data_get(ev->object, "align");

   /* The requested align first and CENTER last: a popup falls back through
      the priority list when the side it wants does not fit, and without a
      last resort it would have nowhere to go. */
   efl_ui_popup_align_priority_set(page->popup, align,
                                   EFL_UI_POPUP_ALIGN_TOP,
                                   EFL_UI_POPUP_ALIGN_BOTTOM,
                                   EFL_UI_POPUP_ALIGN_LEFT,
                                   EFL_UI_POPUP_ALIGN_CENTER);
   efl_ui_popup_align_set(page->popup, align);
   efl_ui_popup_anchor_set(page->popup, page->anchor);

   efl_text_set(page->label, efl_text_get(ev->object));
   efl_gfx_entity_visible_set(page->popup, EINA_TRUE);
}

/* Without an anchor the align is measured against the window instead, which
 * is the other half of what the property means. */
static void
_anchor_none_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Anchor_Page *page = data;

   efl_ui_popup_anchor_set(page->popup, NULL);
   efl_text_set(page->label, "no anchor: aligned to the window");
   efl_gfx_entity_visible_set(page->popup, EINA_TRUE);
}

static void
_anchor_close_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Anchor_Page *page = data;

   efl_gfx_entity_visible_set(page->popup, EINA_FALSE);
}

static void
_anchor_backwall_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Anchor_Page *page = data;

   efl_gfx_entity_visible_set(page->popup, EINA_FALSE);
}

static Eo *
_align_button_add(Eo *table, Anchor_Page *page, const char *text,
                  Efl_Ui_Popup_Align align, int col, int row)
{
   Eo *bt = efl_add(EFL_UI_BUTTON_CLASS, table,
                    efl_text_set(efl_added, text),
                    efl_gfx_hint_weight_set(efl_added, 1, 1));

   efl_key_data_set(bt, "align", (void *)(intptr_t) align);
   efl_event_callback_add(bt, EFL_INPUT_EVENT_CLICKED, _anchor_align_cb, page);
   efl_pack_table(table, bt, col, row, 1, 1);
   return bt;
}

void
test_ui_anchor_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Anchor_Page *page;
   Eo *win, *box, *table, *content, *bt;

   page = calloc(1, sizeof(Anchor_Page));
   if (!page) return;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Efl.Ui.Popup.Anchor"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_event_callback_add(win, EFL_EVENT_DEL, _anchor_page_del, page);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_ui_layout_orientation_set(efl_added,
                                               EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
                 efl_content_set(win, efl_added));

   efl_add(EFL_UI_TEXTBOX_CLASS, box,
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_text_set(efl_added,
                        "Each button puts the popup on that side of the anchor."),
           efl_gfx_hint_weight_set(efl_added, 1, 0),
           efl_pack_end(box, efl_added));

   /* The anchor sits in the middle of a three by three table and the aligns
      sit around it, so each button is on the side it names. */
   table = efl_add(EFL_UI_TABLE_CLASS, box,
                   efl_gfx_hint_weight_set(efl_added, 1, 1),
                   efl_pack_end(box, efl_added));

   page->anchor = efl_add(EFL_UI_BUTTON_CLASS, table,
                          efl_text_set(efl_added, "anchor"),
                          efl_gfx_hint_weight_set(efl_added, 1, 1));
   efl_pack_table(table, page->anchor, 1, 1, 1, 1);

   /* Built before the buttons that show it: they hand it their align. */
   page->popup = efl_add(EFL_UI_POPUP_CLASS, win);
   efl_gfx_entity_size_set(page->popup, EINA_SIZE2D(180, 100));
   efl_gfx_entity_visible_set(page->popup, EINA_FALSE);
   efl_event_callback_add(page->popup, EFL_UI_POPUP_EVENT_BACKWALL_CLICKED,
                          _anchor_backwall_cb, page);

   content = efl_add(EFL_UI_BOX_CLASS, page->popup,
                     efl_ui_layout_orientation_set(efl_added,
                                                   EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   page->label = efl_add(EFL_UI_TEXTBOX_CLASS, content,
                         efl_text_interactive_editable_set(efl_added, EINA_FALSE),
                         efl_text_set(efl_added, "Top"),
                         efl_gfx_hint_weight_set(efl_added, 1, 1),
                         efl_pack_end(content, efl_added));
   efl_add(EFL_UI_BUTTON_CLASS, content,
           efl_text_set(efl_added, "Close"),
           efl_gfx_hint_weight_set(efl_added, 1, 0),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _anchor_close_cb, page),
           efl_pack_end(content, efl_added));
   efl_content_set(page->popup, content);

   _align_button_add(table, page, "Top", EFL_UI_POPUP_ALIGN_TOP, 1, 0);
   _align_button_add(table, page, "Bottom", EFL_UI_POPUP_ALIGN_BOTTOM, 1, 2);
   _align_button_add(table, page, "Left", EFL_UI_POPUP_ALIGN_LEFT, 0, 1);
   _align_button_add(table, page, "Right", EFL_UI_POPUP_ALIGN_RIGHT, 2, 1);
   _align_button_add(table, page, "Center", EFL_UI_POPUP_ALIGN_CENTER, 0, 0);

   bt = efl_add(EFL_UI_BUTTON_CLASS, table,
                efl_text_set(efl_added, "No anchor"),
                efl_gfx_hint_weight_set(efl_added, 1, 1));
   efl_event_callback_add(bt, EFL_INPUT_EVENT_CLICKED, _anchor_none_cb, page);
   efl_pack_table(table, bt, 2, 2, 1, 1);
}
