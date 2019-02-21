#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_ui_panel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX] = {0};
   Eo *win, *table, *panel;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                     efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                     efl_text_set(efl_added, "Efl.Ui.Panel"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   table = efl_add(EFL_UI_TABLE_CLASS, win,
                   efl_content_set(win, efl_added));

   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   efl_add(EFL_UI_IMAGE_CLASS, table,
           efl_file_set(efl_added, buf, NULL),
           efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_pack_table(table, efl_added, 0, 0, 4, 5));

   // Top Panel
   panel = efl_add(EFL_UI_PANEL_CLASS, table,
           efl_ui_panel_orient_set(efl_added, EFL_UI_PANEL_ORIENT_TOP),
           efl_pack_table(table, efl_added, 0, 0, 4, 1));

   efl_add(EFL_UI_BUTTON_CLASS, panel,
           efl_text_set(efl_added, "Top Panel"),
           efl_content_set(panel, efl_added));

   // Left Panel
   panel = efl_add(EFL_UI_PANEL_CLASS, table,
           efl_ui_panel_orient_set(efl_added, EFL_UI_PANEL_ORIENT_LEFT),
           efl_pack_table(table, efl_added, 0, 2, 2, 1));

   efl_add(EFL_UI_BUTTON_CLASS, panel,
           efl_text_set(efl_added, "Left Panel"),
           efl_content_set(panel, efl_added));

   // Right Panel
   panel = efl_add(EFL_UI_PANEL_CLASS, table,
           efl_ui_panel_orient_set(efl_added, EFL_UI_PANEL_ORIENT_RIGHT),
           efl_pack_table(table, efl_added, 2, 2, 2, 1));

   efl_add(EFL_UI_BUTTON_CLASS, panel,
           efl_text_set(efl_added, "Right Panel"),
           efl_content_set(panel, efl_added));

   // Bottom Panel
   panel = efl_add(EFL_UI_PANEL_CLASS, table,
           efl_ui_panel_orient_set(efl_added, EFL_UI_PANEL_ORIENT_BOTTOM),
           efl_pack_table(table, efl_added, 0, 4, 4, 1));

   efl_add(EFL_UI_BUTTON_CLASS, panel,
           efl_text_set(efl_added, "Bottom Panel"),
           efl_content_set(panel, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(280, 320));
}

static void
_check_changed(void *data EINA_UNUSED, const Efl_Event *ev)
{
   elm_config_scroll_thumbscroll_enabled_set(efl_ui_nstate_value_get(ev->object));
}

static void
_panel_toggled(void *data, const Efl_Event *ev)
{
   Evas_Object *list;
   int i;

   if (!efl_ui_nstate_value_get(data)) return;

   list = efl_content_get(ev->object);
   evas_object_del(list);

   list = elm_list_add(ev->object);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   for (i = 0; i < 7; i++)
     elm_list_item_append(list, "panel list item", NULL, NULL, NULL, NULL);
   efl_content_set(ev->object, list);

   printf("Panel toggled:%s\n", efl_ui_panel_hidden_get(ev->object) ? "hidden" : "visible");
}

static void
_btn_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *panel = data;
   efl_ui_panel_toggle(panel);
}
void
test_ui_panel2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int i;
   Eo *win, *box, *check, *btn, *table, *list, *panel;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                     efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                     efl_text_set(efl_added, "Efl.Ui.Panel"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_content_set(win, efl_added));

   efl_add(EFL_UI_CHECK_CLASS, box,
           efl_ui_check_selected_set(efl_added, elm_config_scroll_thumbscroll_enabled_get()),
           efl_text_set(efl_added, "Enable thumb scroll (temporarily"),
           efl_event_callback_add(efl_added, EFL_UI_NSTATE_EVENT_CHANGED, _check_changed, NULL),
           efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0),
           efl_pack(box, efl_added));

   check = efl_add(EFL_UI_CHECK_CLASS, box,
                   efl_text_set(efl_added, "Reset content on toggle"),
                   efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0),
                   efl_pack(box, efl_added));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "toggle"),
                 efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0),
                 efl_pack(box, efl_added));

   table = efl_add(EFL_UI_TABLE_CLASS, box,
                   efl_pack(box, efl_added));

   list = elm_list_add(table);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);
   for (i = 0; i < 20; i++)
     elm_list_item_append(list, "center list item", NULL, NULL, NULL, NULL);
   efl_pack_table(table, list, 0, 0, 1, 1);

   panel = efl_add(EFL_UI_PANEL_CLASS, table,
                   efl_ui_panel_orient_set(efl_added, EFL_UI_PANEL_ORIENT_LEFT),
                   efl_ui_panel_scrollable_set(efl_added, EINA_TRUE),
                   efl_ui_panel_hidden_set(efl_added, EINA_TRUE),
                   efl_ui_panel_scrollable_content_size_set(efl_added, 0.75),
                   efl_pack_table(table, efl_added, 0, 0, 1, 1));

   list = elm_list_add(panel);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   for (i = 0; i < 7; i++)
     elm_list_item_append(list, "left panel item", NULL, NULL, NULL, NULL);
   efl_content_set(panel, list);

   efl_event_callback_add(panel, EFL_UI_PANEL_EVENT_TOGGLED, _panel_toggled, check);
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _btn_clicked, panel);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 400));
}
