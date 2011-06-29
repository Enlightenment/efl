#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
my_fileselector_done(void            *data,
                     Evas_Object *obj __UNUSED__,
                     void            *event_info)
{
   /* event_info conatin the full path of the selected file
    * or NULL if none is selected or cancel is pressed */
    const char *selected = event_info;

    if (selected)
      printf("Selected file: %s\n", selected);
    else
      evas_object_del(data);  /* delete the test window */
}

static void
my_fileselector_selected(void *data   __UNUSED__,
                         Evas_Object *obj,
                         void        *event_info)
{
   /* event_info conatin the full path of the selected file */
   const char *selected = event_info;
   printf("Selected file: %s\n", selected);

   /* or you can query the selection */
   printf("or: %s\n", elm_fileselector_selected_get(obj));
}

static void
_is_save_clicked(void            *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   printf("Toggle Is save\n");
   if (elm_fileselector_is_save_get(fs))
     elm_fileselector_is_save_set(fs, EINA_FALSE);
   else
     elm_fileselector_is_save_set(fs, EINA_TRUE);
}

static void
_folder_only_clicked(void            *data,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   printf("Toggle Folder only\n");
   if (elm_fileselector_folder_only_get(fs))
     elm_fileselector_folder_only_set(fs, EINA_FALSE);
   else
     elm_fileselector_folder_only_set(fs, EINA_TRUE);
}

static void
_expandable_clicked(void            *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   printf("Toggle Expandable\n");
   if (elm_fileselector_expandable_get(fs))
     elm_fileselector_expandable_set(fs, EINA_FALSE);
   else
     elm_fileselector_expandable_set(fs, EINA_TRUE);
}

static void
_sel_get_clicked(void            *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   printf("Get Selected: %s\n", elm_fileselector_selected_get(fs));
}

static void
_path_get_clicked(void            *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   printf("Get Path: %s\n", elm_fileselector_path_get(fs));
}

static void
_mode_cycle_clicked(void            *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   Elm_Fileselector_Mode mode;

   mode = (elm_fileselector_mode_get(fs) + 1) % ELM_FILESELECTOR_LAST;
   elm_fileselector_mode_set(fs, mode);

   printf("Setting mode to: %d\n", mode);
}

void
test_fileselector(void *data       __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Evas_Object *win, *fs, *bg, *vbox, *hbox, *bt, *sep;

   /* Set the locale according to the system pref.
    * If you dont do so the file selector will order the files list in
    * a case sensitive manner
    */
   setlocale(LC_ALL, "");

   elm_need_ethumb();

   win = elm_win_add(NULL, "fileselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "File Selector");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   vbox = elm_box_add(win);
   elm_win_resize_object_add(win, vbox);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);

   fs = elm_fileselector_add(win);
   /* enable the fs file name entry */
   elm_fileselector_is_save_set(fs, EINA_TRUE);
   /* make the file list a tree with dir expandable in place */
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   /* start the fileselector in the home dir */
   elm_fileselector_path_set(fs, getenv("HOME"));
   /* allow fs to expand in x & y */
   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, fs);
   evas_object_show(fs); // TODO fix this is the widget

   /* the 'done' cb is called when the user press ok/cancel */
   evas_object_smart_callback_add(fs, "done", my_fileselector_done, win);
   /* the 'selected' cb is called when the user click on a file/dir */
   evas_object_smart_callback_add(fs, "selected", my_fileselector_selected,
                                  win);

   /* test buttons */
   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(vbox, sep);
   evas_object_show(sep);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   bt = elm_check_add(win);
   elm_check_label_set(bt, "is save");
   elm_check_state_set(bt, elm_fileselector_is_save_get(fs));
   evas_object_smart_callback_add(bt, "changed", _is_save_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_check_add(win);
   elm_check_label_set(bt, "folder only");
   elm_check_state_set(bt, elm_fileselector_folder_only_get(fs));
   evas_object_smart_callback_add(bt, "changed", _folder_only_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_check_add(win);
   elm_check_label_set(bt, "expandable");
   elm_check_state_set(bt, elm_fileselector_expandable_get(fs));
   evas_object_smart_callback_add(bt, "changed", _expandable_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "selected get");
   evas_object_smart_callback_add(bt, "clicked", _sel_get_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "path get");
   evas_object_smart_callback_add(bt, "clicked", _path_get_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "mode cycle");
   evas_object_smart_callback_add(bt, "clicked", _mode_cycle_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   evas_object_resize(win, 240, 350);
   evas_object_show(win);
}

#endif
