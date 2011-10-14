#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifndef ELM_LIB_QUICKLAUNCH
static void
_file_chosen(void            *data,
             Evas_Object *obj __UNUSED__,
             void            *event_info)
{
   Evas_Object *entry = data;
   const char *file = event_info;
   if (file)
     {
        elm_entry_entry_set(entry, file);
        printf("File chosen: %s\n", file);
     }
   else
     printf("File selection canceled.\n");
}

static void
_inwin_mode_toggle(void            *data,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   Evas_Object *fs_bt = data;
   Eina_Bool value = elm_fileselector_button_inwin_mode_get(fs_bt);
   elm_fileselector_button_inwin_mode_set(fs_bt, !value);
   printf("Inwin mode set to: %s\n", value ? "false" : "true");
}

static void
_current_sel_toggle(void            *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Evas_Object *fs_bt = data;
   Eina_Bool value = elm_fileselector_button_is_save_get(fs_bt);
   elm_fileselector_button_is_save_set(fs_bt, !value);
   printf("Current selection editable entry set to: %s\n",
          value ? "false" : "true");
}

static void
_folder_only_toggle(void            *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Evas_Object *fs_bt = data;
   Eina_Bool value = elm_fileselector_button_folder_only_get(fs_bt);
   elm_fileselector_button_folder_only_set(fs_bt, !value);
   printf("Folder only flag set to: %s\n", value ? "false" : "true");
}

static void
_expandable_toggle(void            *data,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   Evas_Object *fs_bt = data;
   Eina_Bool value = elm_fileselector_button_expandable_get(fs_bt);
   elm_fileselector_button_expandable_set(fs_bt, !value);
   printf("Expandable flag set to: %s\n", value ? "false" : "true");
}

void
test_fileselector_button(void *data       __UNUSED__,
                         Evas_Object *obj __UNUSED__,
                         void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *vbox, *hbox, *ic, *bt, *fs_bt, *en, *lb;

   win = elm_win_add(NULL, "fileselector-button", ELM_WIN_BASIC);
   elm_win_title_set(win, "File Selector Button");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   vbox = elm_box_add(win);
   elm_win_resize_object_add(win, vbox);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);

   /* file selector button */
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "file");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   fs_bt = elm_fileselector_button_add(win);
   elm_object_text_set(fs_bt, "Select a file");
   elm_fileselector_button_icon_set(fs_bt, ic);

   elm_box_pack_end(vbox, fs_bt);
   evas_object_show(fs_bt);
   evas_object_show(ic);

   /* attribute setting buttons */
   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   bt = elm_toggle_add(win);
   elm_object_text_set(bt, "current selection text entry");
   evas_object_smart_callback_add(bt, "changed", _current_sel_toggle, fs_bt);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_toggle_add(win);
   elm_object_text_set(bt, "Inwin mode");
   elm_toggle_state_set (bt, EINA_TRUE);
   evas_object_smart_callback_add(bt, "changed", _inwin_mode_toggle, fs_bt);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_toggle_add(win);
   elm_object_text_set(bt, "Folder only mode");
   evas_object_smart_callback_add(bt, "changed", _folder_only_toggle, fs_bt);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_toggle_add(win);
   elm_object_text_set(bt, "Expandable mode");
   evas_object_smart_callback_add(bt, "changed", _expandable_toggle, fs_bt);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Last selection:");
   elm_box_pack_end(vbox, lb);
   evas_object_show(lb);

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, ELM_WRAP_NONE);
   elm_entry_editable_set(en, EINA_FALSE);
   evas_object_smart_callback_add(fs_bt, "file,chosen", _file_chosen, en);
   elm_box_pack_end(vbox, en);
   evas_object_show(en);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

#endif
