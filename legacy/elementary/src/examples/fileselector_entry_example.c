/**
 * Simple Elementary's <b>file selector entry widget</b> example,
 * illustrating its usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` file selector_entry_example.c -o file selector_entry_example
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

/* hook on the file,chosen smart callback */
static void
_file_chosen(void        *data __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void        *event_info)
{
   const char *file = event_info;

   if (file)
     printf("File chosen: %s\n", file);
   else
     printf("File selection canceled.\n");
}

/* toggle inwin mode */
static void
_inwin_mode_toggle(void        *data,
                   Evas_Object *obj __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_entry_inwin_mode_get(fs_entry);

   elm_fileselector_entry_inwin_mode_set(fs_entry, !old_val);
   printf("Inwin mode set to: %s\n", old_val ? "false" : "true");
}

static void
_current_sel_toggle(void        *data,
                    Evas_Object *obj __UNUSED__,
                    void        *event_info __UNUSED__)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_entry_is_save_get(fs_entry);
   elm_fileselector_entry_is_save_set(fs_entry, !old_val);

   printf("%s text entry with selected item's name\n",
          old_val ? "Disabling" : "Enabling");
}

static void
_folder_only_toggle(void        *data,
                    Evas_Object *obj __UNUSED__,
                    void        *event_info __UNUSED__)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_entry_folder_only_get(fs_entry);
   elm_fileselector_entry_folder_only_set(fs_entry, !old_val);

   printf("Folder only mode set to: %s\n", old_val ? "false" : "true");
}

static void
_expandable_toggle(void        *data,
                   Evas_Object *obj __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_entry_expandable_get(fs_entry);
   elm_fileselector_entry_expandable_set(fs_entry, !old_val);

   printf("Expandable folders mode set to: %s\n", old_val ? "false" : "true");
}

EAPI int
elm_main(int    argc __UNUSED__,
         char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *vbox, *hbox, *ic, *ck, *fs_entry, *sep;

   win = elm_win_add(NULL, "fileselector-entry", ELM_WIN_BASIC);
   elm_win_title_set(win, "File Selector Entry Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   vbox = elm_box_add(win);
   elm_win_resize_object_add(win, vbox);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "file");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   /* file selector entry */
   fs_entry = elm_fileselector_entry_add(win);
   evas_object_size_hint_align_set(fs_entry, EVAS_HINT_FILL, 0);
   elm_fileselector_entry_path_set(fs_entry, "/tmp");
   elm_object_text_set(fs_entry, "Select a file");
   elm_fileselector_entry_button_icon_set(fs_entry, ic);

   elm_box_pack_end(vbox, fs_entry);
   evas_object_show(fs_entry);
   evas_object_show(ic);

   /* attribute setting knobs */
   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(vbox, sep);
   evas_object_show(sep);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "editable selection");
   elm_check_state_set(ck, elm_fileselector_entry_is_save_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _current_sel_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "\"inwin\" mode");
   elm_check_state_set(ck, elm_fileselector_entry_inwin_mode_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _inwin_mode_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "folders only");
   elm_check_state_set(ck, elm_fileselector_entry_folder_only_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _folder_only_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "expandable");
   elm_check_state_set(ck, elm_fileselector_entry_expandable_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _expandable_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   evas_object_smart_callback_add(fs_entry, "file,chosen", _file_chosen, NULL);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();
   return 0;
}

ELM_MAIN()
