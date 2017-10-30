#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_POPUP_ANCHOR_BETA
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
        efl_gfx_visible_set(p_data->efl_ui_popup, EINA_TRUE);
        return p_data->efl_ui_popup;
     }

   Eo *efl_ui_popup = efl_add(EFL_UI_POPUP_CLASS, p_data->win);

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_EVENT_BACKWALL_CLICKED, _backwall_clicked, NULL);
   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_EVENT_TIMEOUT, _timeout_cb, p_data);
   efl_gfx_size_set(efl_ui_popup, EINA_SIZE2D(160, 160));

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
        efl_ui_popup_backwall_set(p_data->efl_ui_popup, buf, NULL);
        efl_text_set(ev->object, "Backwall UnSet");
     }
   else
     {
        efl_ui_popup_backwall_set(p_data->efl_ui_popup, NULL, NULL);
        efl_text_set(ev->object, "Backwall Set");
     }
   k = !k;
}

static void
_repeat_event_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   if (!efl_ui_popup_backwall_repeat_events_get(p_data->efl_ui_popup))
     {
        efl_ui_popup_backwall_repeat_events_set(p_data->efl_ui_popup, EINA_TRUE);
        efl_text_set(ev->object, "Repeat Event UnSet");
     }
   else
     {
        efl_ui_popup_backwall_repeat_events_set(p_data->efl_ui_popup, EINA_FALSE);
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
   if (!efl_gfx_visible_get(p_data->button))
     {
        efl_gfx_visible_set(p_data->button, EINA_TRUE);
        efl_text_set(ev->object, "Hide Dummy Button");
     }
   else
     {
        efl_gfx_visible_set(p_data->button, EINA_FALSE);
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
   efl_gfx_position_set(p_data->efl_ui_popup, EINA_POSITION2D(0, 0));
}

static void
_timeout_set_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   if (!p_data->efl_ui_popup) return;
   efl_ui_popup_timeout_set(p_data->efl_ui_popup, 3);
   printf("timemout is set to 3 seconds\n");
}

static void
_win_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   efl_del(p_data->panel_win);
   free(p_data);
   printf("window is deleted\n");
}

static void
_panel_win_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_data *p_data = data;
   efl_del(p_data->win);
   free(p_data);
   printf("window is deleted\n");
}

void
test_ui_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_ui_popup_data *p_data = (efl_ui_popup_data*)calloc(1, sizeof(efl_ui_popup_data));

   Eo *win = efl_add(EFL_UI_WIN_CLASS, NULL,
                     efl_text_set(efl_added, "Efl.Ui.Popup"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _win_del, p_data);

   efl_gfx_size_set(win, EINA_SIZE2D(500, 500));

   Eo *panel_win = efl_add(EFL_UI_WIN_CLASS, NULL,
                           efl_text_set(efl_added, "Efl.Ui.Popup Panel"),
                           efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_event_callback_add(panel_win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _panel_win_del, p_data);

   efl_gfx_size_set(panel_win, EINA_SIZE2D(300, 500));

   Eo *repeat_test_btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_gfx_position_set(repeat_test_btn, EINA_POSITION2D(0, 0));
   efl_gfx_size_set(repeat_test_btn, EINA_SIZE2D(100, 100));
   efl_text_set(repeat_test_btn, "Repeat Event Test");
   efl_event_callback_add(repeat_test_btn, EFL_UI_EVENT_CLICKED, _repeat_test_cb, NULL);
   efl_gfx_visible_set(repeat_test_btn, EINA_FALSE);

   p_data->win = win;
   p_data->panel_win = panel_win;
   p_data->button = repeat_test_btn;
   p_data->efl_ui_popup = _create_popup(p_data);

   Eo *create_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(create_btn, "Create Popup");
   efl_gfx_position_set(create_btn, EINA_POSITION2D(0, 0));
   efl_gfx_size_set(create_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(create_btn, EFL_UI_EVENT_CLICKED, _create_cb, p_data);

   Eo *delete_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(delete_btn, "Delete Popup");
   efl_gfx_position_set(delete_btn, EINA_POSITION2D(150, 0));
   efl_gfx_size_set(delete_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(delete_btn, EFL_UI_EVENT_CLICKED, _delete_cb, p_data);

   Eo *repeat_event_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(repeat_event_btn, "Repeat Event Set");
   efl_gfx_position_set(repeat_event_btn, EINA_POSITION2D(0, 50));
   efl_gfx_size_set(repeat_event_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(repeat_event_btn, EFL_UI_EVENT_CLICKED, _repeat_event_cb, p_data);

   Eo *dummy_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(dummy_btn, "Show Dummy Button");
   efl_gfx_position_set(dummy_btn, EINA_POSITION2D(150, 50));
   efl_gfx_size_set(dummy_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(dummy_btn, EFL_UI_EVENT_CLICKED, _dummy_cb, p_data);

   Eo *backwall_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_gfx_position_set(backwall_btn, EINA_POSITION2D(0, 100));
   efl_gfx_size_set(backwall_btn, EINA_SIZE2D(150, 50));
   efl_text_set(backwall_btn, "Backwall Set");
   efl_event_callback_add(backwall_btn, EFL_UI_EVENT_CLICKED, _backwall_cb, p_data);

   Eo *timeout_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(timeout_btn, "Timeout Set");
   efl_gfx_position_set(timeout_btn, EINA_POSITION2D(150, 100));
   efl_gfx_size_set(timeout_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(timeout_btn, EFL_UI_EVENT_CLICKED, _timeout_set_cb, p_data);

   Eo *center_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(center_btn, "Center Align");
   efl_gfx_position_set(center_btn, EINA_POSITION2D(0, 150));
   efl_gfx_size_set(center_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(center_btn, EFL_UI_EVENT_CLICKED, _center_align_cb, p_data);

   Eo *top_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(top_btn, "Top Align");
   efl_gfx_position_set(top_btn, EINA_POSITION2D(150, 150));
   efl_gfx_size_set(top_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(top_btn, EFL_UI_EVENT_CLICKED, _top_align_cb, p_data);

   Eo *left_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(left_btn, "Left Align");
   efl_gfx_position_set(left_btn, EINA_POSITION2D(0, 200));
   efl_gfx_size_set(left_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(left_btn, EFL_UI_EVENT_CLICKED, _left_align_cb, p_data);

   Eo *right_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(right_btn, "Right Align");
   efl_gfx_position_set(right_btn, EINA_POSITION2D(150, 200));
   efl_gfx_size_set(right_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(right_btn, EFL_UI_EVENT_CLICKED, _right_align_cb, p_data);

   Eo *bottom_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(bottom_btn, "Bottom Align");
   efl_gfx_position_set(bottom_btn, EINA_POSITION2D(0, 250));
   efl_gfx_size_set(bottom_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(bottom_btn, EFL_UI_EVENT_CLICKED, _bottom_align_cb, p_data);

   Eo *position_btn = efl_add(EFL_UI_BUTTON_CLASS, panel_win);
   efl_text_set(position_btn, "Position Set (x:0, y:0)");
   efl_gfx_position_set(position_btn, EINA_POSITION2D(150, 250));
   efl_gfx_size_set(position_btn, EINA_SIZE2D(150, 50));
   efl_event_callback_add(position_btn, EFL_UI_EVENT_CLICKED, _position_set_cb, p_data);
}

static void
efl_ui_popup_alert_clicked_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Popup_Alert_Clicked_Event *event = ev->info;

   if (event->button_type == EFL_UI_POPUP_ALERT_BUTTON_POSITIVE)
     printf("Positive Button is clicked\n");
   else if(event->button_type == EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE)
     printf("Negative Button is clicked\n");
   else if(event->button_type == EFL_UI_POPUP_ALERT_BUTTON_USER)
     printf("User Button is clicked\n");
}

void
test_ui_popup_alert(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];

   Eo *win = efl_add(EFL_UI_WIN_CLASS, NULL,
                     efl_text_set(efl_added, "Efl.Ui.Popup.Alert"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_size_set(win, EINA_SIZE2D(320, 320));

   Eo *efl_ui_popup = efl_add(EFL_UI_POPUP_ALERT_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   efl_gfx_size_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_set(layout, buf, "efl_ui_popup_scroll_content");

   efl_content_set(efl_ui_popup, layout);
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_POSITIVE, "Yes");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE, "No");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_USER, "Cancel");

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_ALERT_EVENT_CLICKED, efl_ui_popup_alert_clicked_cb, NULL);
}

void
test_ui_popup_alert_scroll(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];

   Eo *win = efl_add(EFL_UI_WIN_CLASS, NULL,
                     efl_text_set(efl_added, "Efl.Ui.Popup.Alert.Scroll"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_size_set(win, EINA_SIZE2D(320, 320));

   Eo *efl_ui_popup= efl_add(EFL_UI_POPUP_ALERT_SCROLL_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   efl_gfx_size_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, efl_ui_popup);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_set(layout, buf, "efl_ui_popup_scroll_content");

   efl_content_set(efl_ui_popup, layout);

   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_POSITIVE, "Yes");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE, "No");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_USER, "Cancel");

   efl_ui_popup_alert_scroll_expandable_set(efl_ui_popup, EINA_TRUE, EINA_TRUE);
   efl_gfx_size_hint_max_set(efl_ui_popup, EINA_SIZE2D(300, 180));

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_ALERT_EVENT_CLICKED, efl_ui_popup_alert_clicked_cb, NULL);
}

void
test_ui_popup_alert_text(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = efl_add(EFL_UI_WIN_CLASS, NULL,
                     efl_text_set(efl_added, "Efl.Ui.Popup.Alert.Text"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_size_set(win, EINA_SIZE2D(320, 320));

   Eo *efl_ui_popup= efl_add(EFL_UI_POPUP_ALERT_TEXT_CLASS, win);

   efl_text_set(efl_part(efl_ui_popup, "title"), "title");

   efl_gfx_size_set(efl_ui_popup, EINA_SIZE2D(160, 160));

   efl_text_set(efl_ui_popup,
                "This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup"
                "This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup"
                "This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup This is Text Popup");

   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_POSITIVE, "Yes");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_NEGATIVE, "No");
   efl_ui_popup_alert_button_set(efl_ui_popup, EFL_UI_POPUP_ALERT_BUTTON_USER, "Cancel");

   efl_ui_popup_alert_text_expandable_set(efl_ui_popup, EINA_TRUE);
   efl_gfx_size_hint_max_set(efl_ui_popup, EINA_SIZE2D(100, 180));

   efl_event_callback_add(efl_ui_popup, EFL_UI_POPUP_ALERT_EVENT_CLICKED, efl_ui_popup_alert_clicked_cb, NULL);
}

static void
_anchor_set_cb(void *data, const Efl_Event *ev)
{
   efl_ui_popup_anchor_set(data, ev->object);
}

static void
_anchor_unset_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_popup_anchor_set(data, NULL);
}

void
test_ui_popup_anchor(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_ui_popup_data *p_data = (efl_ui_popup_data*)calloc(1, sizeof(efl_ui_popup_data));

   char buf[PATH_MAX];

   Eo *win = efl_add(EFL_UI_WIN_CLASS, NULL,
                     efl_text_set(efl_added, "Efl.Ui.Popup.Anchor"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _win_del, p_data);

   efl_gfx_size_set(win, EINA_SIZE2D(500, 500));

   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_set(layout, buf, "efl_ui_popup_anchor_layout");
   efl_gfx_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_content_set(win, layout);

   Eo *efl_ui_popup = efl_add(EFL_UI_POPUP_ANCHOR_CLASS, win);

   efl_ui_popup_backwall_repeat_events_set(efl_ui_popup, EINA_TRUE);

   //Default align priority order is top, left, right, bottom, center.
   efl_ui_popup_anchor_align_priority_set(efl_ui_popup, EFL_UI_POPUP_ALIGN_TOP,
                                          EFL_UI_POPUP_ALIGN_BOTTOM,
                                          EFL_UI_POPUP_ALIGN_LEFT,
                                          EFL_UI_POPUP_ALIGN_RIGHT,
                                          EFL_UI_POPUP_ALIGN_CENTER);

   efl_gfx_position_set(efl_ui_popup, EINA_POSITION2D(80, 80));
   efl_gfx_size_set(efl_ui_popup, EINA_SIZE2D(160, 120));

   p_data->win = win;
   p_data->efl_ui_popup = efl_ui_popup;

   Eo *btn = NULL;
   for (int i = 0; i < 6; i++)
     {
        btn = efl_add(EFL_UI_BUTTON_CLASS, win);
        efl_text_set(btn, "anchor");
        efl_gfx_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _anchor_set_cb, efl_ui_popup);

        snprintf(buf, sizeof(buf), "anchor%d", i+1);
        efl_content_set(efl_part(layout, buf), btn);
     }

   btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(btn, "anchor none");
   efl_gfx_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _anchor_unset_cb, efl_ui_popup);
   efl_content_set(efl_part(layout, "anchor_none"), btn);

   Eo *grid = efl_add(EFL_UI_GRID_CLASS, efl_ui_popup);
   efl_gfx_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   btn = efl_add(EFL_UI_BUTTON_CLASS, efl_ui_popup);
   efl_text_set(btn, "Center Align");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(70, 35));
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _center_align_cb, p_data);
   efl_pack_grid(grid, btn, 0, 0, 1, 1);

   btn = efl_add(EFL_UI_BUTTON_CLASS, efl_ui_popup);
   efl_text_set(btn, "Left Align");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(70, 35));
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _left_align_cb, p_data);
   efl_pack_grid(grid, btn, 1, 0, 1, 1);

   btn = efl_add(EFL_UI_BUTTON_CLASS, efl_ui_popup);
   efl_text_set(btn, "Right Align");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(70, 35));
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _right_align_cb, p_data);
   efl_pack_grid(grid, btn, 2, 0, 1, 1);

   btn = efl_add(EFL_UI_BUTTON_CLASS, efl_ui_popup);
   efl_text_set(btn, "Top Align");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(70, 35));
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _top_align_cb, p_data);
   efl_pack_grid(grid, btn, 0, 1, 1, 1);

   btn = efl_add(EFL_UI_BUTTON_CLASS, efl_ui_popup);
   efl_text_set(btn, "Bottom Align");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(70, 35));
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _bottom_align_cb, p_data);
   efl_pack_grid(grid, btn, 1, 1, 1, 1);

   btn = efl_add(EFL_UI_BUTTON_CLASS, efl_ui_popup);
   efl_text_set(btn, "Position Set");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(70, 35));
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _position_set_cb, p_data);
   efl_pack_grid(grid, btn, 2, 1, 1, 1);

   efl_content_set(efl_ui_popup, grid);
}
