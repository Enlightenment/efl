/**
 * Simple Elementary's <b>file selector widget</b> example,
 * illustrating its usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` file selector_example.c -o file selector_example
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_on_done(void        *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void        *event_info __UNUSED__)
{
   elm_exit();
}

/* 'done' cb */
static void
_fs_done(void        *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void        *event_info)
{
   const char *selected = event_info;
   /* event_info contains the full path of the selected file or NULL
    * if none is selected (or cancel is pressed) */

   printf("We're done! Selected file is: %s\n",
          selected ? selected : "*none!*");

   _on_done(NULL, NULL, NULL);
}

/* 'selected' cb */
static void
_fs_selected(void        *data __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void        *event_info)
{
   const char *selected = event_info;
   /* event_info contains the full path of the selected file */

   printf("There's been a selection: %s\n", selected);
}

static void
_is_save_clicked(void        *data,
                 Evas_Object *obj __UNUSED__,
                 void        *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   Eina_Bool old_val = elm_fileselector_is_save_get(fs);

   printf("%s text entry with selected item's name\n",
          old_val ? "Disabling" : "Enabling");

   elm_fileselector_is_save_set(fs, !old_val);
}

static void
_folder_only_clicked(void        *data,
                     Evas_Object *obj __UNUSED__,
                     void        *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   Eina_Bool old_val = elm_fileselector_folder_only_get(fs);

   printf("%s folder-only mode\n",
          old_val ? "Disabling" : "Enabling");

   elm_fileselector_folder_only_set(fs, !old_val);
}

static void
_expandable_clicked(void        *data,
                    Evas_Object *obj __UNUSED__,
                    void        *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   Eina_Bool old_val = elm_fileselector_expandable_get(fs);

   printf("%s tree-view mode\n",
          old_val ? "Disabling" : "Enabling");

   elm_fileselector_expandable_set(fs, !old_val);
}

static void
_sel_get_clicked(void        *data,
                 Evas_Object *obj __UNUSED__,
                 void        *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   printf("Current selection is: %s\n", elm_fileselector_selected_get(fs));
}

static void
_path_get_clicked(void        *data,
                  Evas_Object *obj __UNUSED__,
                  void        *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   printf("Current selection's directory path is: %s\n",
          elm_fileselector_path_get(fs));
}

EAPI int
elm_main(int    argc __UNUSED__,
         char **argv __UNUSED__)
{
    Evas_Object *win, *fs, *bg, *vbox, *buttons_bx, *bt, *sep, *bx;

   /* Set the locale according to the system pref. If you dont do so
    * the file selector will order the files list in a case sensitive
    * manner
    */
   setlocale(LC_ALL, "");

   elm_need_ethumb(); /* let's have thumbnails of images on grid view */

   win = elm_win_add(NULL, "fileselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "File Selector Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL); */
   evas_object_show(bx);

   vbox = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(vbox);
   elm_box_pack_end(bx, vbox);

   /* first file selector, in list mode */
   fs = elm_fileselector_add(win);
   /* enable the fs file name entry */
   elm_fileselector_is_save_set(fs, EINA_TRUE);
   /* custom list view */
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   /* start the fileselector in the /tmp/ dir */
   elm_fileselector_path_set(fs, "/tmp");

   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, fs);
   evas_object_show(fs);

   /* the 'done' cb is called when the user presses ok/cancel */
   evas_object_smart_callback_add(fs, "done", _fs_done, win);
   /* the 'selected' cb is called when the user clicks on a file/dir */
   evas_object_smart_callback_add(fs, "selected", _fs_selected, win);

   /* test buttons */
   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(vbox, sep);
   evas_object_show(sep);

   buttons_bx = elm_box_add(win);
   elm_box_horizontal_set(buttons_bx, EINA_TRUE);
   elm_box_pack_end(vbox, buttons_bx);
   evas_object_show(buttons_bx);

   bt = elm_check_add(win);
   elm_object_text_set(bt, "editable selection");
   elm_check_state_set(bt, elm_fileselector_is_save_get(fs));
   evas_object_smart_callback_add(bt, "changed", _is_save_clicked, fs);
   elm_box_pack_end(buttons_bx, bt);
   evas_object_show(bt);

   bt = elm_check_add(win);
   elm_object_text_set(bt, "folders only");
   elm_check_state_set(bt, elm_fileselector_folder_only_get(fs));
   evas_object_smart_callback_add(bt, "changed", _folder_only_clicked, fs);
   elm_box_pack_end(buttons_bx, bt);
   evas_object_show(bt);

   bt = elm_check_add(win);
   elm_object_text_set(bt, "expandable");
   elm_check_state_set(bt, elm_fileselector_expandable_get(fs));
   evas_object_smart_callback_add(bt, "changed", _expandable_clicked, fs);
   elm_box_pack_end(buttons_bx, bt);
   evas_object_show(bt);

   buttons_bx = elm_box_add(win);
   elm_box_horizontal_set(buttons_bx, EINA_TRUE);
   elm_box_pack_end(vbox, buttons_bx);
   evas_object_show(buttons_bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Print selection");
   evas_object_smart_callback_add(bt, "clicked", _sel_get_clicked, fs);
   elm_box_pack_end(buttons_bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Print path");
   evas_object_smart_callback_add(bt, "clicked", _path_get_clicked, fs);
   elm_box_pack_end(buttons_bx, bt);
   evas_object_show(bt);

   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_FALSE);
   elm_box_pack_end(bx, sep);
   evas_object_show(sep);

   /* second file selector, now with grid view */
   fs = elm_fileselector_add(win);
   elm_fileselector_is_save_set(fs, EINA_TRUE);
   elm_fileselector_mode_set(fs, ELM_FILESELECTOR_GRID);
   elm_fileselector_buttons_ok_cancel_set(fs, EINA_FALSE);
   elm_fileselector_path_set(fs, "/tmp");

   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fs);
   evas_object_show(fs);

   evas_object_resize(win, 800, 600);
   evas_object_show(win);

   elm_run();
   return 0;
}

ELM_MAIN()
